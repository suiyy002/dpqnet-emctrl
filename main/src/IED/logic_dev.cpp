/*! \file sv_shmem.cpp
    \brief .
*/

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <sys/time.h>

using namespace std;

LogicDev::LogicDev(int idx, const OneChannel **chnl)
{
    index_ = idx;
    one_chnnl_ = chnl;
    memset(&channel_, 0, sizeof(channel_));
    prmchg_cnt_ = 1;
    rce_func_ = new RceFunc(idx);
    delay_cnt_ = 0;
}

LogicDev::~LogicDev()
{
    delete rce_func_;
}

/*!
Read logical device parameter default value

    Output:  para
*/
void LogicDev::DefaultPara(ParamLD *para)
{
    memset(&parm_ld_, 0, sizeof(parm_ld_));
    parm_ld_.version = 1;
    for (int i=0; i<3; i++) {
        parm_ld_.capacity[i] = 100;
    }
    
    memcpy(para, parm_ld_, sizeof(parm_ld_));
}

/*!
Refresh logical device data

    Input:  chnl-- Channel object [kChannelTol]
*/
void LogicDev::RefreshData()
{
    const MeasureValue *meas_val;
    int vidx = channel_.idx[0];
    if (vidx) {  //voltage channel
        int i = vidx-1;
        if (one_chnnl_[i]->chl_mode()) RefreshFreq(one_chnnl_[i]);
    }
    int cidx = channel_.idx[1];
    if (cidx) {  //current channel
        int i = cidx-1;
        if (one_chnnl_[i]->chl_mode()) RefreshFreq(one_chnnl_[i]);
    }
    if (vidx&&cidx) {
        
    }
}

void LogicDev::RefreshFreq(OneChannel *chnl)
{
    const FreqValue *frq_val = chnl->frq_val();
    if (frq_val->freq_up) {
        chnl->set_freq_up(0);
        float fi = frq_val->freq;
        fi /= 100;
        shmem_srv().SetReal(index_, kRealFreq, &fi, frq_val->time);
        
        GuiMsgFreq gui_frq;
        gui_frq.freq = fi*1000;
        gui_frq.time = frq_val->time;
        messageq_guis().Push(kCmdFrequency, sizeof(gui_frq), &gui_frq, &index_);
    }
    if (frq_val->freqst_up) {
        chnl->set_freqst_up(0);
        float fi[4];
        for (int i=0; i<4; i++) {
            fi[i] = frq_val->freqst;
            fi[i] /= 100;
        }
        shmem_srv().SetStatis(index_, kStatisFreq, fi, frq_val->timest);
    }
}

/*!
Description: Pick 10 cycle value of certain harmonic

    Input:  src -- fft value. [3][cnt*2], 2n=real, 2n+1=image
            order -- order of harmonic.0=DC, 1=1/10,2=2/10... 10=fundamental,... 20=2nd...
    Output: des
*/
void LogicDev::Pick10CycHrm(int loops, int *src, int cnt, int order)
{
    float fr, fi;

    for (int i=0; i<3; i++) {
        fr = *(src+order*2);    //*src * 2.0 / FFTSampMaxNum;
        fi = *(src+order*2+1);  //*src * 2.0 / FFTSampMaxNum;
        harm_10cyc_[i][loops]->modulus = fr * fr + fi * fi;
        harm_10cyc_[i][loops]->real = fr;
        harm_10cyc_[i][loops]->image = fi;
        src += cnt*2;
    }
}

/*!
Get logical device parameter -- parm_ld_

    Input:  chg -- change count
    Output: chg
    Return: parameter. NULL=parameter not change
*/
const uint8_t *LogicDev::GetParm(int *chg)
{
    if (*chg==prmchg_cnt_) {
        return NULL;
    } else {
        *chg = prmchg_cnt_;
        return &parm_ld_;
    }
}

/*!
Set logical device parameter -- parm_ld_

    Input:  parm -- physical parameter
    Output: chg
*/
void LogicDev::SetParm(uint8 *parm, int *chg)
{
    memcpy(&parm_ld_, data, sizeof(parm_ld_));
    prmchg_cnt_++;
    if (chg) {
        *chg = prmchg_cnt_;
    }
    rce_func_->set_dre_attr(parm_ld_.dre_tol, parm_ld_.flt_num);
}

/*!
Handle rapid change event

    Input:  chnl-- Channel object [kChannelTol]
*/
void LogicDev::HandleRce()
{
    int idx[2];
    idx[0] = channel_.idx[0];   //voltage
    idx[1] = channel_.idx[1];   //current
    int end = 1;
    for (int i=0; i<2; i++) {   //0=voltage, 1=current
        if (!idx[i]) continue; //channel not exist
        if (!rce_func_->trigger_cause()) {
            IdentifyRce(i, one_chnnl_[idx[i]-1]);
            if (rce_func_->trigger_cause()) {
                RefreshRcePar();
                rce_func_->set_trig_phs(one_chnnl_[idx[i]-1]->trig_phs());
            }
        } else {
            if ( !RceEnd(i, one_chnnl_[idx[i]-1]) ) end = 0;
        }
    }
    if (trigger_cause() && trigger_cause()<kTrigManual) {
        if (end) {
            if (delay_cnt_++>=2) {
                if (one_chnnl_[idx[0]-1]->rce_intr()) {
                    rce_func_->set_event_type(kEvtTypeIntr);
                }
                EndRce();
            }
        } else {
            delay_cnt_ = 0;
        }
    }
}

/*!
Identiry rce cause & type

    Input:  vc -- 0=voltage, 1=current
            chnl -- OneChannel object.
*/
void LogicDev::IdentifyRce(int vc, OneChannel *chnl)
{
    const RceInfo *rce_inf[3];
    const timeval *ptmv = NULL;
    int i;
    for (i=0; i<3; i++) {
        rce_inf[i] = chnl->rce_info(i);
        if (rce_inf[i]->stat) {
            if ( !ptmv || TimevalCmp(&rce_inf[i]->stm, ptmv)<0) {
                ptmv = rce_inf[i]->stm;
                break;
            }
        }
    }
    if (i<3) {  //limit trigger
        if (vc) {   //current channel
            rce_func_->set_trigger_cause(kTrigILmt);
        } else {    //voltage channel
            rce_func_->set_trigger_cause(kTrigVLmt);
        }
        if (rce_inf[i]->hpn.dipstr) {
            rce_func_->set_event_type(kEvtTypeDip);
        } else {
            rce_func_->set_event_type(kEvtTypeSwell);
        }
        rce_func_->set_ttime(ptmv);
    } else {    //change rate trigger?
        for (i=0; i<3; i++) {
            if (rce_inf[i]->ratestr) {
                if ( !ptmv || TimevalCmp(&rce_inf[i]->stm, ptmv)<0) {
                    ptmv = rce_inf[i]->stm;
                    break;
                }
            }
        }
        if (i<3) {
            if (vc) {   //current channel
                rce_func_->set_trigger_cause(kTrigIStr);
            } else {    //voltage channel
                rce_func_->set_trigger_cause(kTrigVVar);
            }
            rce_func_->set_ttime(ptmv);
        }
    }
}

/*!
Set rapid change event sampling data

    Input:  vc -- 0=voltage, 1=current
            sv -- sampling value. [0-2]:A-C
            par -- rce parameter.
*/
inline void LogicDev::SetRceData(int vc, int32_t **sv, RceParam *par)
{
    rce_func_->SetDuration(&par->time);
    if (rce_func_->overtime()) {
        if (rce_func_->trigger_cause()==kTrigManual) {
            EndRce();
        }
        return;
    }
    rce_func_->SetRceData(vc, sv, par->cnt, &par->time);
}

/*!
End event

    Input:  chnl-- Channel object[kChannelTol]
*/
void LogicDev::EndRce()
{
    uint32_t extr = -1;
    int idx = channel_.idx[0];
    if (idx) {
        extr = one_chnnl_[idx-1]->rce_extre();
    }
    rce_func_->EndRce(extr);
    one_chnnl_[idx-1]->ClearRce();
}

/*!
detect whether an event is end

    Input:  vc -- 0=voltage, 1=current
            chnl -- OneChannel object
    Return: 0=not end, 1=end
*/
void LogicDev::RceEnd(int vc, OneChannel *chnl)
{
    const RceInfo *rce_inf;
    const timeval *ptmv = NULL;
    for (int i=0; i<3; i++) {
        rce_inf = chnl->rce_info(i);
        if (rce_inf->varend) {
            if ( !ptmv || TimevalCmp(&rce_inf->etm, ptmv)>0) {
                ptmv = rce_inf[i]->etm;
            }
        } else break;
    }
    if (i==3) { //event end
        rce_func_->SetDuration(ptmv);
        return 1;
    } else {
        return 0;
    }
}

/*!
Manual trigger record wave

    Input:  type -- 0=stop, 1=mannaul start, other=steady start
    Return: 0=success, -1=have event happening, -2=state not change.
*/
int LogicDev::ManualTrigger(int type)
{
    int ret = rce_func_->ManualTrigger(type);
    if (ret) return ret;
    if (type) { //start
        timeval tmv;
        gettimeofday(&tmv, NULL);
        rce_func_->set_ttime(&tmv);
        RefreshRcePar();
        rce_func_->set_trig_phs(0);
    } else {    //stop
         EndRce();
    }
    return ret;
}

inline void LogicDev::RefreshRcePar()
{                
    int idx[2];
    idx[0] = channel_.idx[0];   //voltage
    idx[1] = channel_.idx[1];   //current
    rce_func_->set_chnl_idx(idx[0], idx[1]);
    if (idx[0]) rce_func_->SetRatio(0, one_chnnl_[idx[0]-1]->parm_chnl()->trns_rto);
    if (idx[1]) rce_func_->SetRatio(1, one_chnnl_[idx[1]-1]->parm_chnl()->trns_rto);
}
