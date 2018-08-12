#include <string.h>
#include<cmath>
#include<cstdio>
#include<cstdlib>
#include <fcntl.h>
#include <unistd.h>

#include "transt_analysis.h"
#include "prmconfig.h"
#include "../comm/datatype.h"
#include "../IPC/shmemfunc.h"
#include "../ComTraDE/comtrade_func.h"
#include "../device/device.h"

using namespace std;

TranstAnalysis::TranstAnalysis(void)
{
    qvvr_soe_ = new LoopBuffer<PqmQvvr>(8);

    memset(&qvvr_val_, 0, sizeof(qvvr_val_));
    //qvvr_val_.varend = 1;
    //memcpy(&qvvr_val_.str_time, &ptrst_hd->stime, sizeof(timeval));
    //memcpy(&qvvr_val_.end_time, &ptrst_hd->stime, sizeof(timeval));
    qvvr_soe_->push(&qvvr_val_);
    
    psquare_ = NULL;
    pthread_mutex_init (&mutex_,NULL);
}

TranstAnalysis::~TranstAnalysis(void)
{
    delete qvvr_soe_;
    pthread_mutex_destroy(&mutex_); //清除互斥锁mutex
    
}

//Event started
void TranstAnalysis::VarStr(TRST_F_HEAD * ptrst_hd)
{
    float fi;
    if (psquare_) {
        printf("Error!! psquare_ is not NULL. %s %s\n", __FILE__, __LINE__);
        return;
    }
    switch (ptrst_hd->smpl_freq) {
        case 0:
            smpl_freq_ = 6400;
            break;
        case 1:
            smpl_freq_ = 10240;
        default:
            break;
    }
    fi = smpl_freq_;
    smpnum_cyc_ = fi / 50 + 0.5;
    //printf("%s(): 2111******** psquare_=%x\n", __FUNCTION__, psquare_);

    psquare_ = new LoopBuffer<float>(smpnum_cyc_ * 3);
    reset_rms_ = true;
    //printf("%s(): 3111********\n", __FUNCTION__);

    memset(rms_qdrtc_sum_, 0, sizeof(rms_qdrtc_sum_));
    memset(duration_, 0, sizeof(duration_));
    memset(limit_cnt_, 0, sizeof(limit_cnt_));
    memset(max_val_, 0, sizeof(max_val_));
    for (int i = 0; i < 3; i++) {
        min_val_[i] = 99999999999.0;
    }
    event_end_ = false;

    //Get threshold
    fi = prmcfg->GetTrsThrld(0, 0); // voltage dip
    thr_dip_[0] = fi * fi * smpnum_cyc_;
    thr_dip_[1] = thr_dip_[0] * 1.04;     //2%(1.02^2=1.04)回滞hysteresis, 即[0]~[1]为deadband
    fi = prmcfg->GetTrsThrld(0, 1); // voltage swell
    thr_swl_[0] = fi * fi * smpnum_cyc_;
    thr_swl_[1] = thr_swl_[0] * 0.96;    //2%(0.98^2=0.96)回滞hysteresis
    fi = prmcfg->GetTrsThrld(0, 2); // voltage interrupt
    thr_intr_ = fi * fi * smpnum_cyc_;

    if (ptrst_hd->uscale) {     //电压单位 is 1/50V
        for (int i = 0; i < 2; i++) {
            thr_dip_[i] /= 4;
            thr_swl_[i] /= 4;
        }
    }

    memset(&qvvr_val_, 0, sizeof(qvvr_val_));
    qvvr_val_.happen = 1;
    qvvr_val_.varstr = 1;
    memcpy(&qvvr_val_.str_time, &ptrst_hd->stime, sizeof(timeval));
    memcpy(&qvvr_val_.end_time, &ptrst_hd->stime, sizeof(timeval));
    pthread_mutex_lock(&mutex_);
    qvvr_soe_->push(&qvvr_val_);
    pthread_mutex_unlock(&mutex_);
    offset_num_ = -1;
    FilterCPx(0);
}

/*!
Event finished

    Called by:  VoltVariation::save_transt_data
*/
void TranstAnalysis::VarEnd(TRST_F_HEAD * ptrst_hd)
{
    if (!psquare_) {
        printf("Error!! psquare_ is NULL. %s %s\n", __FILE__, __FUNCTION__);
        return;
    } else {
        delete psquare_;
        psquare_ = NULL;
    }
    float fi;
    int i, j;
    event_end_ = true;
    
    unsigned short u_extr;
    read_daram(&u_extr, 2, 0x1F91 * 2);
    printf("u_extr=%d\n", u_extr);
    
    //Write to share memory
    float max = 0;
    for (j = 0; j < 3; j++) {
        if (max < max_val_[j]) max = max_val_[j];
    }
    float min = 99999999999.0;
    for (j = 0; j < 3; j++) {
        if (min > min_val_[j]) min = min_val_[j];
    }
    int k;
    if (ptrst_hd->uscale) {     //电压单位 is 1/50V
        k = 20;
    } else {
        k = 10;
    }
    int evt;
    if ( min < thr_intr_ || thr_dip_[0]-min > max-thr_swl_[0]) {
        i = sqrt(min/smpnum_cyc_)*k;
        if (qvvr_val_.intrstr) evt = 1;
        else if (qvvr_val_.dipstr) evt = 2;
    } else {
        i = sqrt(max/smpnum_cyc_)*k;
        evt = 3;
    }
    evnt_char_val_.type = evt;

    //shmem_func().SetQvvr(kQvvrVva, &i);
    qvvr_val_.vva = i*10/prmcfg->get_pt(2);
#ifdef EXTREMA_PERCENT
    fi = qvvr_val_.vva;
    evnt_char_val_.extrema = fi/100;
#else
    fi = i;
    evnt_char_val_.extrema = fi/1000;
#endif
    
    i = 0;
    for (j = 0; j < 3; j++) {
        if(limit_cnt_[j]<3) continue;
        fi = duration_[j];
        fi *= 1000;
        fi /= smpl_freq_;
        duration_[j] = fi; //unit:ms
        if (i<duration_[j]) i = duration_[j];
    }
    if (!i) {   //没有触发任何事件
        fi = duration_[0];
        fi *= 1000;
        fi /= smpl_freq_;
        i = fi; //unit:ms
    }
    qvvr_val_.vvatm = i;
    fi = i;
    evnt_char_val_.dur = fi/1000;
    
    fi = u_extr;
    printf("vva=%6.2fV %6.2fV VVaTm=%6.3f type=%d\n", evnt_char_val_.extrema, fi/100, evnt_char_val_.dur, evnt_char_val_.type);     //for debug
    if (prmcfg->uextrm()&&ptrst_hd->cause==5) evnt_char_val_.extrema = fi/100;
        
    if (offset_num_<0) offset_time_ = 0;
    else {
        fi = offset_num_;
        fi *= 1000000;
        fi /= smpl_freq_;
        offset_time_ = fi;  //unit:us
    }
    
    qvvr_val_.varstr = 0;
    qvvr_val_.varend = 1;
    memcpy(&qvvr_val_.str_time, &ptrst_hd->etime, sizeof(timeval));
    memcpy(&qvvr_val_.end_time, &ptrst_hd->etime, sizeof(timeval));
    pthread_mutex_lock(&mutex_);
    qvvr_soe_->push(&qvvr_val_);
    pthread_mutex_unlock(&mutex_);
}

/*!
Description:copy transient data to transt_data_savebuf_u/i
Input:      vc -- 0=voltage, 1=current
            pdata -- Transient sampling value,A/B/C 1st value, A/B/C 2nd value.
                     unit:1/50V, 1/100V, mA
            smp_len -- data length, number of value, A/B/C is one value
*/
void TranstAnalysis::AddSmplData(int vc, short *pdata, unsigned int smp_len)
{
    float fi;
    int i = 0, j;
    if (vc) return;     //discard current transient data
    if (!psquare_) {
        printf("Error!! psquare_ is NULL. %s\n", __FUNCTION__);
        return;
    }

    if (reset_rms_) {   //First package of a event
        //reset_rms_ = false;
        for (; i < smpnum_cyc_; i++) {
            for (j = 0; j < 3; j++) {
                fi = *pdata;
                fi *= fi;
                psquare_->push(&fi);
                rms_qdrtc_sum_[j] += fi;
                pdata++;
                duration_[j]++;
            }
        }
    }
    bool end[3];
    memset(end, 0, sizeof(end));
    for (; i < smp_len; i++) {
        if (event_end_) break;
        for (j = 0; j < 3; j++) {
            if (psquare_->pop(&fi)<0) {
                printf("psquare_ is empty!\n");
                break;
            }
            rms_qdrtc_sum_[j] -= fi;
            fi = *pdata;
            fi *= fi;
            psquare_->push(&fi);
            rms_qdrtc_sum_[j] += fi;
            if (max_val_[j] < rms_qdrtc_sum_[j]) max_val_[j] = rms_qdrtc_sum_[j];
            if (min_val_[j] > rms_qdrtc_sum_[j]) min_val_[j] = rms_qdrtc_sum_[j];
            FilterCPx(2, j);
            if (rms_qdrtc_sum_[j] < thr_dip_[0] || rms_qdrtc_sum_[j] > thr_swl_[0]) {
                limit_cnt_[j]++;
            } else {
                if (limit_cnt_[j]<3) limit_cnt_[j] = 0;
            }
            if (limit_cnt_[j] == 3) {
                if (offset_num_<0) offset_num_ = duration_[j];
                duration_[j] = 0;
                FilterCPx(1, j);
            }
            if (limit_cnt_[j] > 3) {
                if (rms_qdrtc_sum_[j] > thr_dip_[1] && rms_qdrtc_sum_[j] < thr_swl_[1]) {
                    end[j] = true;
                    FilterCPx(3, j);
                }
            }
            pdata++;
            if (!end[j]) duration_[j]++;
        }
        if (end[0] && end[1] && end[2]) {
            event_end_ = true;
        }
    }

    char chi;
    //Determine the event type
    if (!qvvr_val_.intrstr) {  //Interrupt
        if (min_val_[0] < thr_intr_ || min_val_[1] < thr_intr_ || min_val_[2] < thr_intr_) {
            qvvr_val_.intrstr = 1;
            //chi = 1;
            //shmem_func().SetQvvr(kQvvrIntrstr, &chi);    //pPqmshm->qvvr.intrstr = 1;
        }
    }
    if (!qvvr_val_.dipstr) {   //Dip
        if (min_val_[0] < thr_dip_[0] || min_val_[1] < thr_dip_[0] || min_val_[2] < thr_dip_[0]) {
            qvvr_val_.dipstr = 1;
            //chi = 1;
            //shmem_func().SetQvvr(kQvvrDipstr, &chi);    //pPqmshm->qvvr.dipstr = 1;
        }
    }
    if (!qvvr_val_.swlstr) {   //Swell
        if (max_val_[0] > thr_swl_[0] || max_val_[1] > thr_swl_[0] || max_val_[2] > thr_swl_[0]) {
            qvvr_val_.swlstr = 1;
            //chi = 1;
            //shmem_func().SetQvvr(kQvvrSwlstr, &chi);    //pPqmshm->qvvr.swlstr = 1;
        }
    }
    reset_rms_ = false;
}

/*!
Description:Write QVVR data to share memory
*/
void TranstAnalysis::WriteQvvrShm()
{
    if (!qvvr_soe_->data_num()) return;
    if (shmem_func().qvvr_happen()) return;
    
    PqmQvvr qvvr;
    pthread_mutex_lock(&mutex_);
    qvvr_soe_->pop(&qvvr);
    pthread_mutex_unlock(&mutex_);

    shmem_func().ShmemCpy(kQvvrSOE, &qvvr);
    shmem_func().IncDataUp(1);
    //if(qvvr.varend) shmem_func().set_qvvr_ok();
    shmem_func().set_qvvr_ok();
}

/*!
CPx Filter,filter voltage fluacuation

    type -- 0=initialize; 1=start; 2=process; 3=end
    phs -- phase. 0-2=A-C
*/
void TranstAnalysis::FilterCPx(int type, int phs)
{
    int i, j;
    int cpx = prmcfg->fltrcpx();
    if (cpx==0) return;
    if (type==0) {  //initialize
        memset(fltr_par_, 0, sizeof(fltr_par_));
    } else if (type==1) {   //start
        fltr_par_[phs].start = true;
        for (i = 0; i < 24; i++) {
            fltr_par_[phs].min[i] = 99999999999.0;
        }
    } else if (type==2) {   //process
        if (fltr_par_[phs].start==false) return;
        if(++fltr_par_[phs].interval>=80) {
            fltr_par_[phs].interval = 0;
            for (i=0; i<24; i++) {
                if (fltr_par_[phs].min[i] > rms_qdrtc_sum_[phs]) {
                    for (j=23;j>i;j--) {
                        fltr_par_[phs].min[j] = fltr_par_[phs].min[j-1];
                    }
                    fltr_par_[phs].min[i] = rms_qdrtc_sum_[phs];
                    break;
                }
            }
            fltr_par_[phs].tol++;
        }    
    } else if (type==3) {   //end
        fltr_par_[phs].start = false;
        i = fltr_par_[phs].tol*cpx / 100;
        if (i>23) i = 23;
        if (i>2) min_val_[phs] = fltr_par_[phs].min[i];
        //printf("tol=%d, min_val_[%d]=%f, fltr_par_[%d].min[%d]=%f\n", 
        //        fltr_par_[phs].tol, phs,min_val_[phs], phs, i, fltr_par_[phs].min[i]);
    }
}
