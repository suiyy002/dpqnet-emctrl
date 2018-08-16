#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <sys/time.h>

#include "one_channel.h"
#include "phy_dev.h"

using namespace std;
const static int kRvcIntrvl = 32;   //Rapid Voltage Change event detect interval. unit:sample point number
const static int kRltvChgIntrvl = kHrmSmpNum/kRvcIntrvl/10; //relative change event detect interval (¡Ö 1cycle).
                                                            //unit:kRvcIntrvl sample point.
const static int kSmpFreq = (kHrmSmpNum+5)/10;  //sample frequency. unit:number/cycle

OneChannel::OneChannel()
{
    memset(&chnnl_attr_, 0, sizeof(chnnl_attr_));
    memset(&meas_par3s_, 0, sizeof(meas_par3s_));
    
    for (int i=0; i<3; i++) {
        rce_rmsn2_[i] = new LoopBuffer<float>(kHrmSmpNum/kRvcIntrvl);
        rce_sv_[i] = new LoopPointer<int32_t>(15);
        rce_par_[i] = new LoopBuffer<int32_t>(15);
    }
    prmchg_cnt_ = 1;
    memset(rce_info_, 0, sizeof(rce_info_));
    smp_frq_ = 12800;
    rce_max_ = 0;
    rce_min_ = 2e16;
}

OneChannel::~OneChannel()
{
    for (int i=0; i<3; i++) {
        delete rce_par_[i];
        delete rce_sv_[i];
        delete rce_rmsn2_[i];
    }
}

/*!
Read channel parameter default value

    Output:  para
*/
void OneChannel::DefaultPara(ParamChnl *para)
{
    memset(&parm_chnl_, 0, sizeof(parm_chnl_));
    parm_chnl_.version = 1;
    if (chl_type()==1) {   //voltage channel
        trns_rto[0] = 10000;
        trns_rto[1] = 100;
    } else {            //current channel
        trns_rto[0] = 600;
        trns_rto[1] = 5;
    }
    
    memcpy(para, parm_chnl_, sizeof(parm_chnl_));    
}

/*!
measure frequecy, be called per 10cycle

    Input:  cnt10 -- number of sample point in 10cycle
            tm -- time of start point
*/
void OneChannel::MeasureFreq(int cnt10, long tm)
{
    FreqParam * frqp = &frq_par_;
    
    frqp->freq_cnt += cnt10;
    frqp->cnt++
    int up = 0;
    if (frqp->frqmspc==1) {  //1s time interval
        if (frqp->cnt%5 == 0) up = 1;
    } else {    //3s or 10s time interval
        int n = frqp->tmi->TimeOut(tm);
        if (n==1) {
            up = 1;
        } else if (n==2) {
            frqp->cnt = 0;
            frqp->freq_cnt = 0;
        }
    }
    if (up) {
        float fi = frqp->freq_cnt;
        if (fi>0) frqp->frq_val.freq = 128000*frqp->cnt/fi;   //(256*10*frqp->cnt/fi)*50
        else frqp->frq_val.freq = 0;
        frqp->frq_val.freq_up = 1;
        frqp->frq_val.freq_tm = tm;
        frqp->cnt = 0;
        frqp->freq_cnt = 0;
        StatisFreq();
    }
}

/*!
Frequency statistics

    Input:  cnt10 -- number of sample point in 10cycle
            tm -- time of start point
*/
void OneChannel::StatisFreq()
{
    int n = phy_dev().stts_spc(kPQParaFreq);
    if (frq_par_.space!=n) {
        frq_par_.tmist->set_interval(n);
        frq_par_.space = n;
        n /= frq_par_.frqmspc*20;
        if (frq_par_.datast) delete frq_par_.datast;
        frq_par_.datast = new DataStatis<int>(n, CompareInt);
    }
    frq_par_.datast->SetData(frq_par_.freq, frq_par_.freq*frq_par_.freq);
    n = frqp->tmist->TimeOut(frqp->frq_val.time);
    if (n==0) return;
    if (n==1) {
        frqp->datast->GetData(frqp->frq_val.freqst);
        frqp->frq_val.freqst_up = 1;
        frqp->frq_val.timest = frqp->frq_val.time;
    }
    frqp->datast->IniData();
}

/*!
Measure data statistics

    Input:  tm -- current 3s data time
*/
void OneChannel::StatisData(time_t tm)
{
    StatisRms(tm);
    StatisParam * frqp = &frq_par_;
    int up = 0;

        int n = frqp->tmi->TimeOut(tm);
        if (n==1) {
            up = 1;
        } else if (n==2) {
            frqp->cnt = 0;
            frqp->freq_cnt = 0;
        }

    if (up) {
        float fi = frqp->freq_cnt;
}

/*!
Rms & voltage deviation statistics

    Input:  tm -- current 3s data time
*/
void OneChannel::StatisRms(time_t tm)
{
    int n = phy_dev().stts_spc(kPQParaUdev);
    if (sta_rms_.space!=n) {
        sta_rms_.space = n;
        sta_rms_.tmi->set_interval(n);
        n /= 3*20;
        for (int i=0; i<3; i++) {
            if (sta_rms_.rms_st[i]) delete sta_rms_.rms_st[i];
            sta_rms_.rms_st[i] = new DataStatis<float>(n, CompareFloat);
            for (int j=0; j<2; j++) {
                if (sta_rms_.udev_st[i][j]) delete sta_rms_.udev_st[i][j];
                sta_rms_.udev_st[i][j] = new DataStatis<float>(n, CompareFloat);
            }
        }
    }
    for (int i=0; i<3; i++) {
        sta_rms_.rms_st->SetData(meas_par3s_.val.rmsn2[i]);
        for (int j=0; j<2; j++) {
            sta_rms_.udev_st->SetData(meas_par3s_.val.u_devn2[i][j]);
        }
    }
    n = sta_rms_.tmi->TimeOut(tm);
    if (n==0) return;
    if (n==1) {
        for (int i=0; i<3; i++) {
            sta_rms_.rms_st[i]->GetData(sta_rms_.val.rmsn2[i]);
            for (int j=0; j<2; j++) {
                sta_rms_.udev_st[i][j]->GetData(sta_rms_.val.u_devn2[i][j]);
            }
        }
        sta_rms_.val.update = 1;
        sta_rms_.val.time = tm;
    }
    for (int i=0; i<3; i++) {
        sta_rms_.rms_st[i]->IniData();
        for (int j=0; j<2; j++) {
            sta_rms_.udev_st[i][j]->IniData();
        }
    }
}

/*!
Harmonic statistics

    Input:  tm -- current 3s data time
*/
void OneChannel::StatisHarm(time_t tm)
{
    int n = phy_dev().stts_spc(kPQParaHrm);
    if (sta_hrm_.space!=n) {
        sta_hrm_.space = n;
        sta_hrm_.tmi->set_interval(n);
        n /= 3*20;
        for (int i=0; i<3; i++) {
            for (int j=0; j<51; j++) {
                if (sta_hrm_.real[i][j]) {
                    delete sta_hrm_.real[i][j];
                    delete sta_hrm_.image[i][j];
                }
                sta_hrm_.real[i][j] = new DataStatis<float>(n, CompareFloat);
                sta_hrm_.image[i][j] = new DataStatis<float>(n, CompareFloat);
            }
        }
    }
    for (int i=0; i<3; i++) {
        for (int j=0; j<51; j++) {
            sta_hrm_.real[i][j]->SetData(meas_par3s_.harm3s[i][j].real);
            sta_hrm_.image[i][j]->SetData(meas_par3s_.harm3s[i][j].image);
        }
    }
    n = sta_hrm_.tmi->TimeOut(tm);
    if (n==0) return;
    if (n==1) {
        float ftmp[2][4];
        for (int i=0; i<3; i++) {
            for (int j=0; j<51; j++) {
                sta_hrm_.real[i][j]->GetData(ftmp[0]);
                sta_hrm_.image[i][j]->GetData(ftmp[1]);
                for (int k=0; k<4; k++) {
                    sta_hrm_.val.harm[i][j][k].real = ftmp[0][k];
                    sta_hrm_.val.harm[i][j][k].image = ftmp[1][k];
                }
            }
        }
        sta_hrm_.val.update = 1;
        sta_hrm_.val.time = tm;
    }
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            sta_hrm_.real[i][j]->IniData();
            sta_hrm_.image[i][j]->IniData();
        }
    }
}
/*!
Measure 3s interval value

    Input:  ssrc -- resample value in 10cycles
            scnt -- count of ssrc
            tm -- timestamp of 1st sample point in 10cycles
            hsrc -- [0-2]:PhaseA-C; 2n=real, 2n+1=image
*/
void OneChannel::MeasureData3s(int (*ssrc)[3][kHrmSmpNum], int scnt, time_t tm, int (*hsrc)[3][640*2])
{
    MeasureRms(ssrc, scnt);
    PostFft(hsrc);
    CComplexNum fund[3]; //fundamental component
    for (int i=0; i<3; i++) {
        fund[i].real = hsrc[i][0];
        fund[i].image = hsrc[i][1];
    }
    MeasureSeq(fund);
    
    int cnt = ++meas_par3s_.cnt;
    if (cnt < 15) return;
    for (int i=0; i<3; i++) {
        meas_par3s_.val.rmsn2[i] = meas_par3s_.sum_rmsn2[i] / cnt;
        meas_par3s_.val.u_devn2[i][0] = meas_par3s_.sum_u_devn2[i][0] / cnt;
        meas_par3s_.val.u_devn2[i][1] = meas_par3s_.sum_u_devn2[i][1] / cnt;
        meas_par3s_.val.seq[i] = meas_par3s_.sum_seq[i] / cnt;
        for (int j=0; j<640; j++) {
            meas_par3s_.harm3s[i][j] = meas_par3s_.sum_harm[i][j] / cnt;
        }
    }
    MeasureHarm(&meas_par3s_);
    meas_par3s_.val.time = tm;
    meas_par3s_.update = 1;
    memset(&meas_par3s_.cnt, 0, sizeof(meas_par3s_)-sizeof(meas_par3s_.val));

    StatisData(tm);
}

/*!
Measure harmonic 3s value

    Input:  par3s -- &meas_par3s_;
*/
void OneChannel::MeasureHarm(MeasParam3s * par3s)
{
    int i, j;
    for (i=0; i<3; i++) {   //DC & fundmental component
        for (j=0; j<2; j++) {
            par3s->val.ihrm[i][j] = par3s->harm3s[i][j*10];
        }
    }        
    int type = phy_dev().hrm_type();
    if (type==1) {          //Component
        for (i=0; i<3; i++) {
            for (j=2; j<51; j++) {
                par3s->val.harm[i][j] = par3s->harm3s[i][j*10];
            }
        }        
    } else if (type==2) {   //Group
        CComplexNum cmxi, cmxj={0, 0};
        for (i=0; i<3; i++) {
            for (j=1; j<50; j++) {
                cmxi = par3s->harm3s[i][j*10+5];
                cmxi.real /= 2; cmxi.image /= 2;
                cmxj.real += cmxi.real; cmxj.image += cmxi.image;
                for (int k=0; k<9; k++) {
                    cmxi = par3s->harm3s[i][j*10+5+k];
                    cmxj.real += cmxi.real; cmxj.image += cmxi.image;
                }
                cmxi = par3s->harm3s[i][j*10+15];
                cmxi.real /= 2; cmxi.image /= 2;
                cmxj.real += cmxi.real; cmxj.image += cmxi.image;
                par3s->val.harm[i][j] = cmxj;
            }
        }
    } else {                //Subgroup
        CComplexNum cmxi, cmxj={0, 0};
        for (i=0; i<3; i++) {
            for (j=1; j<50; j++) {
                for (int k=0; k<3; k++) {
                    cmxi = par3s->harm3s[i][j*10+9+k];
                    cmxj.real += cmxi.real; cmxj.image += cmxi.image;
                }
                par3s->val.harm[i][j] = cmxj;
            }
        }
    }
    if (chl_type()==1) { //voltage harmonic ratio
        for (i=0; i<3; i++) {
            float f1 = par3s->val.harm[i][1].real + par3s->val.harm[i][1].image;
            float fthd = 0, fodd = 0;
            for (j=2; j<51; j++) {
                CComplexNum cmxh = par3s->val.harm[i][j];
                float fi = cmxh.real + cmxh.image;
                par3s->val.hr[i][j] = sqrt(fi/f1)*100;
                fthd += fi/f1;
                if (j%2) fodd += fi/f1;
            }
            par3s->val.thd[i] = sqrt(fthd)*100;
            par3s->val.thdodd[i] = sqrt(fodd)*100;
            par3s->val.thdevn[i] = sqrt(fthd-fodd)*100;
        }
    }
    MeasureInterHarm(par3s);
}

/*!
Measure interharmonic 3s value

    Input:  par3s -- &meas_par3s_;
*/
void OneChannel::MeasureInterHarm(MeasParam3s * par3s)
{
    int i, j;
    int type = phy_dev().ihrm_type();
    if (type==1) {          //Group
        CComplexNum cmxi, cmxj={0, 0};
        for (i=0; i<3; i++) {
            for (j=0; j<51; j++) {
                for (int k=1; k<9; k++) {
                    cmxi = par3s->harm3s[i][j*10+k];
                    cmxj.real += cmxi.real; cmxj.image += cmxi.image;
                }
                par3s->val.harm[i][j] = cmxj;
            }
        }
    } else if (type==2) {   //1/2
        for (i=0; i<3; i++) {
            for (j=0; j<51; j++) {
                par3s->val.harm[i][j] = par3s->harm3s[i][j*10+5];
            }
        }        
    } else {                //Subgroup
        CComplexNum cmxi, cmxj={0, 0};
        for (i=0; i<3; i++) {
            for (j=0; j<51; j++) {
                for (int k=2; k<8; k++) {
                    cmxi = par3s->harm3s[i][j*10+k];
                    cmxj.real += cmxi.real; cmxj.image += cmxi.image;
                }
                par3s->val.harm[i][j] = cmxj;
            }
        }
    }
    if (chl_type()==1) { //voltage interharmonic ratio
        for (i=0; i<3; i++) {
            float f1 = par3s->val.harm[i][1].real + par3s->val.harm[i][1].image;
            for (j=2; j<51; j++) {
                CComplexNum cmxh = par3s->val.ihrm[i][j];
                par3s->val.ihr[i][j] = sqrt((cmxh.real + cmxh.image)/f1)*100;
            }
        }
    }
}

/*!
Measure Root mean square

    Input:  src -- resample value in 10cycles
            cnt -- count input value
*/
void OneChannel::MeasureRms(int (*src)[3][kHrmSmpNum], int cnt)
{
    MeasParam3s * p3s = &meas_par3s_;
    float fi, sums10;   //sum of squares in 10cycles
    for (int i=0; i<3; i++) {
        sums10 = 0;
        for (int j=0; j<cnt; j++) {
            fi = src[i][j];
            fi *= fi;
            sums10 += fi;
        }
        sums10 /= cnt;
        p3s->sum_rmsn2[i] += sums10;
        //voltage deviation
        if (chnnl_attr_.type==1) { //be voltage channel
            fi = chnl_par_.trns_rto[0];   //din -- declare input. unit:0.1V or 0.1A
            fi *= fi;
            if (sums10>fi) {
                p3s->sum_u_devn2[i][1] += sums10;
                p3s->sum_u_devn2[i][0] += fi;
            } else {
                p3s->sum_u_devn2[i][0] += sums10;
                p3s->sum_u_devn2[i][1] += fi;
            }
        }
    }
}

/*!
Description: fft post-processing

    Input:  src -- fft result value. 2n=real, 2n+1=image
            cnt -- count input value per phase. 
*/
void OneChannel::PostFft(int (*src)[3][640*2])
{
    MeasParam3s * p3s = &meas_par3s_;
    for (int i=0; i<3; i++) {
        int *p = src[i];
        CComplexNum *p_harm = meas_par3s_.sum_harm[i];
        for ( int j = 0; j < 640; j++ ) {
            p_harm->real += *p++;
            p_harm->image += *p++;
            p_harm++;
        }
    }
}

/*!
Measure sequence component

    Input:  fund -- fundamental component
*/
void OneChannel::MeasureSeq(CComplexNum * fund)
{
    float * seq = meas_par3s_.seq;
    
    seq[0] += VectorsSum(fund, 3) / 3;
    
    RotateVec(&fund[1], 0);
    RotateVec(&fund[2], 1);
    seq[1] += VectorsSum(fund, 3) / 3;
    
    RotateVec(&fund[1], 0);
    RotateVec(&fund[2], 1);
    seq[2] += VectorsSum(fund, 3) / 3;
}

/*!
Get channel parameter -- parm_chnl_

    Input:  chg -- change count
    Output: chg
    Return: parameter. NULL=parameter not change
*/
const uint8_t *OneChannel::GetParm(int *chg)
{
    if (*chg==prmchg_cnt_) {
        return NULL;
    } else {
        *chg = prmchg_cnt_;
        return &parm_chnl_;
    }
}

/*!
Handle rapid change evnet

    Input:  rms -- rce rms^2
            rcnt -- count of rms value
            src -- sample point data source
            scnt -- count of data source
            stm -- time of 1st sample point in src
            phs -- phase. 0-2=A-C
*/
void OneChannel::HandleRce(float *rms, int rcnt, int32_t *src, int scnt, timeval stm, int phs)
{
    const int kPreSz = 2;
    rce_sv_[phs]->Push(src);
    while(rce_sv_[phs]->DataNum()>kPreSz) {
        int *p = rce_sv_[phs]->Pop();
        delete [] p;
    }
    
    RceParam par = {stm, scnt};
    rce_par_[phs]->Push(&par);
    while(rce_par_[phs]->DataNum()>kPreSz) {
        rce_par_[phs]->Pop(&par);
    }
    
    DetectEvnt(rms, rcnt, phs, stm);
}

/*!
Detect rapid change event

    Input:  rms -- rce rms^2
            rcnt -- count of rms value
            phs -- phase. 0-2=A-C
            stm -- time of 1st sample point in src
*/
void OneChannel::DetectEvnt(float *rms, int rcnt, int phs, timeval stm)
{
    float swl_lmt[2], dip_lmt[2], intr_lmt;
    time_val tmv = stm;
    float us1p = 1000000.0/smp_frq_;   //interval time between two sampling point.unit:us

    if (parm_chnl_.rce_en) {
        GetRceLimit(swl_lmt, dip_lmt, &intr_lmt);
        for (int i=0; i<rcnt; i++) {
            if (!rce_info_[phs].stat) {
                if (rms[i]<dip_lmt[0]) {
                    rce_info_[phs].hpn.dip = 1;
                } else if (rms[i]>swl_lmt[0]) {
                    rce_info_[phs].hpn.swl = 1;
                }
                if (rce_info_[phs].stat) {
                    tmv.tv_usec = stm.tv_usec + i*us1p;
                    rce_info_[phs].stm.tv_sec = tmv.tv_sec + tmv.tv_usec/1000000;
                    rce_info_[phs].stm.tv_usec = tmv.tv_usec%1000000;
                    trig_phs_ = phs;
                }
            } else {
                if (rms[i]>dip_lmt[1] && rms[i]<swl_lmt[1] && !rce_info_[phs].varend) {
                    rce_info_[phs].varend = 1;
                    tmv.tv_usec = stm.tv_usec + i*us1p;
                    rce_info_[phs].etm.tv_sec = tmv.tv_sec + tmv.tv_usec/1000000;
                    rce_info_[phs].etm.tv_usec = tmv.tv_usec%1000000;
                    if (rce_info_[phs].hpn.dip) {
                        rce_extre_ = sqrt(rce_min_);
                    } else {
                        rce_extre_ = sqrt(rce_max_);
                    }
                    rce_max_ = 0;
                    rce_min_ = 2e16;
                } else {
                    rce_info_[phs].varend = 0;
                    if (rce_info_[phs].hpn.intr==0 && rms[i]<intr_lmt) {
                        rce_info_[phs].hpn.intr = 1;
                    }
                }
            }
        }
        if (chl_type()==1) {    //voltage channel
            for (int i=0; i<rcnt; i++) {
                if (rms[i]>rce_max_) rce_max = rms[i];
                if (rms[i]<rce_min_) rce_min = rms[i];
            }
        }
    }
    
    float rms_o;
    if (parm_chnl_.rce_rate_en && !rce_info_[phs].stat) {
        GetRceLimit(swl_lmt, dip_lmt, &intr_lmt, 1);
        for (int i=0; i<rcnt; i++) {
            rce_rmsn2_[phs]->Push($rms[i]);
            if (rce_rmsn2_[phs]->DataNum()<kHrmSmpNum/kRvcIntrvl) continue;
            rce_rmsn2_[phs]->Pop(&rms_o);
            rms_o = rms[i]-rms_o;
            if (!rce_info_[phs].ratestr) {
                if (rms_o>swl_lmt[0] || rms_o<dip_lmt[0] ) {
                    rce_info_[phs].ratestr = 1;
                }
                if (rce_info_[phs].ratestr) {
                    tmv.tv_usec = stm.tv_usec + i*us1p;
                    rce_info_[phs].stm.tv_sec = tmv.tv_sec + tmv.tv_usec/1000000;
                    rce_info_[phs].stm.tv_usec = tmv.tv_usec%1000000;
                    trig_phs_ = phs;
                }
            } else {
                if (rms_o<swl_lmt[1] && rms_o>dip_lmt[1] && !rce_info_[phs].varend) {
                    rce_info_[phs].varend = 1;
                    tmv.tv_usec = stm.tv_usec + i*us1p;
                    rce_info_[phs].etm.tv_sec = tmv.tv_sec + tmv.tv_usec/1000000;
                    rce_info_[phs].etm.tv_usec = tmv.tv_usec%1000000;
                    rce_extre_ = -1;
                } else {
                    rce_info_[phs].varend = 0;
                }
            }
        }
    } else {
        for (int i=0; i<rcnt; i++) {
            rce_rmsn2_[phs]->Push($rms[i]);
        }
        while(rce_rmsn2_[phs]->DataNum()>=kHrmSmpNum/kRvcIntrvl) {
            rce_rmsn2_[phs]->Pop(&rms_o);
        }
    }
}

/*!
Get rce"rapid change event" limit.

    Input:  type -- 0=normal, 1=rate of change
    Output: high -- swell^2, (*)[2]. [1]:hysteresis, e.g. [0]~[1] is deadband
            low -- dip^2, (*)[2]. [1]:hysteresis, e.g. [0]~[1] is deadband
            intr -- interrupt^2, null for rate of change
*/
void OneChannel::GetRceLimit(float *high, float *low, float *intr, int type)
{
    float fi[3][2];
    if (type) { //rate of change
        fi[0][0] = parm_chnl.rce_rate_lmt[0];
        fi[1][0] = parm_chnl.rce_rate_lmt[1];
        fi[2][0] = 0;
    } else {
        fi[0][0] = parm_chnl.rce_limit[0];
        fi[1][0] = parm_chnl.rce_limit[1];
        fi[2][0] = parm_chnl.rce_limit[2];
    }
    
    for (int i=0; i<3; i++) {
        fi[i][0] *= parm_chnl_.trns_rto[0];
        fi[i][0] /= 1000;
    }
    fi[0][1] = fi[0][0] * 0.98; //2% hysteresis, e.g. [0][0]~[0][1] is deadband
    if (type) {
        fi[1][1] = fi[1][0] * 0.98;
    } else {
        fi[1][1] = fi[1][0] * 1.02; //2% hysteresis, e.g. [1][0]~[1][1] is deadband
    }
    for (int i=0; i<2; i++) {
        fi[i][0] *= fi[i][0];
        fi[i][1] *= fi[i][1];
    }
    
    if (type) {
        fi[1][0] = -fi[1][0];        
        fi[1][1] = -fi[1][1];        
    } else {
        *intr = fi[2][0];
    }
    memcpy(high, fi[0], sizeof(float)*2);
    memcpy(low, fi[1], sizeof(float)*2);
}

void OneChannel::ClearRce()
{
    for (int i; i<3; i++) {
        rce_info_[i].stat = 0;
        rce_info_[i].ratestr = 0;
    }
}
