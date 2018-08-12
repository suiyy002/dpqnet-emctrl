#include "volt_variation.h"
#include "harmfunc.h"
#include "other_func.h"
#include "save_func.h"
#include "../base_func/utility.h"
#include "../GUI/form/mainform.h"
#include "../ComTraDE/comtrade_func.h"
#include <cmath>
#include <fcntl.h>
#include <unistd.h>
using namespace std;

const char * EvntInfoFile = "save/transt_sv/event_info_"; //事件信息存储文件

RceFunc::RceFunc(int idx)
{
    memset(rce_stat_, 0, sizeof(rce_stat_));
    ld_idx_ = idx;
    comtrd_fun_ = new ComtradeFunc(idx);
    memset(psvbuf_, 0, sizeof(psvbuf_));
    buf_cnt_ = 0;

    evnt_smmry_info_ = new LoopBuffer<EvntSmmryInfo>(100);
    char fname[64];
    sprintf(fname, "%sld%d.txt", EvntInfoFile, idx);
    FILE *fd = fopen(fname, "r");
    if (fd) {
        EvntSmmryInfo info;
        int n;
        fscanf(fd, "%d\n", &n);
        for (int i = 0; i < n; i++) {
            fgets(info.str, 48, fd);
            char * pch = strtok(info.str, "\n");
            evnt_smmry_info_->push((EvntSmmryInfo*)pch);
        }
        fclose(fd);
    }
    qvvr_soe_ = new LoopBuffer<PqmQvvr>(8);
    pthread_mutex_init (&mutex_,NULL);
}

RceFunc::~RceFunc()
{
    pthread_mutex_destroy(&mutex_);
    delete qvvr_soe_;
    delete evnt_smmry_info_;
    delete comtrd_fun_;
}

/*!
Set rapid change event sampling data

    Input:  vc -- 0=voltage, 1=current
            sv -- sampling value. [0-2]:A-C
            cnt -- count of sv
            time -- time of 1st sv
*/
void RceFunc::SetRceData(int vc, int32_t **sv, int cnt, timeval *time)
{
    if (!rce_stat_.rtime.tv_sec) {
        memcpy(&rce_stat_.rtime, time, sizeof(timeval));
        if (trigger_cause()==kTrigVLmt) PushQvvr(0);
        comtrd_fun_->IniSaveData(time, &rce_stat_.ttime, chnl_idx_);
        memset(psvbuf_, 0, sizeof(psvbuf_));
        buf_cnt_ = 0;
    }
        
    if (!vc && buf_cnt_) {
        comtrd_fun_->SaveDataFile(buf_cnt_, psvbuf_[0], psvbuf_[1]);
        memset(psvbuf_, 0, sizeof(psvbuf_));
        buf_cnt_ = 0;
    }
    
    buf_cnt_ = cnt;
    psvbuf_[vc] = savebuf_[vc][0];
    for (int i=0; i<3; i++) {
        int32_t *psv = sv[i];
        int16_t *pbuf = &psvbuf_[vc][i];
        for (int n=0; n<cnt; n++) {
            *pbuf = *psv * ratio_[vc] + 0.5;
            psv++; pbuf += 3;
        }
    }
}

/*!
Set Ratio of PT or CT

    Input:  vc -- 0=voltage, 1=current
            ps -- [0-1]:Primary,Secondary
*/
void RceFunc::SetRatio(int vc, const uint32_t *ps)
{
    float fi = ps[0];
    ratio_[vc] = ps[1] / fi;
    
    comtrd_fun_->set_ptct(vc, ps); 
}

/*!
End event

    Input:  extr -- extremum. unit:10mV
*/
void RceFunc::EndRce(uint32_t extr)
{
    rce_stat_.extre = extr;
    comtrd_fun_->set_charcs(rce_stat_.extra, rce_stat_.dur);
    comtrd_fun_->set_rec_dev_id(phy_dev().device_id());
    if (trigger_cause()==kTrigVLmt) PushQvvr(1);
    comtrd_fun_->EndSave(CauseOfEvent[rce_stat_.trig_c], TypeOfEvent[rce_stat_.event_t]);
    SetEventSummary(1);
    memset(rce_stat_, 0, sizeof(rce_stat_));
}

/*!
set event duration

    Input:  tmv -- time of event finished
*/
void RceFunc::SetDuration(timeval *tmv)
{
    int dur = (tmv->tv_sec - rce_stat_.ttime.tv_sec)*1000;
    dur += (tmv->tv_usec - rce_stat_.ttime.tv_usec)/1000;
    if (dur>rce_stat_.dur) rce_stat_.dur = dur;
    memcpy(&rce_stat_.etime, tmv, sizeof(timeval));
}

/*!
Push qvvr data into qvvr_soe_

    Input:  type -- 0=start, 1=end
*/
void RceFunc::PushQvvr(int type)
{
    PqmQvvr qvvr;
    memset(&qvvr, 0, sizeof(qvvr));
    
    qvvr.happen = 1;
    if (!type) {    //start
        qvvr.varstr = 1;
        qvvr.str_time = qvvr.end_time = rce_stat_->ttime;
    } else {        //end
        qvvr.varstr = 0;
        qvvr.varend = 1;
        switch (rce_stat_.event_t) {
            case kEvtTypeIntr: qvvr.intrstr = 1; break;
            case kEvtTypeDip: qvvr.dipstr = 1; break;
            case kEvtTypeSwell: qvvr.swlstr = 1; break;
        }
        qvvr.str_time = qvvr.end_time = rce_stat_->etime;
        qvvr.vva = rce_stat_.extre;
        qvvr.vvatm = rce_stat_.dur;
    }
    
    pthread_mutex_lock(&mutex_);
    qvvr_soe_->push(&qvvr);
    pthread_mutex_unlock(&mutex_);
}

/*!
Refresh QVVR data to share memory
*/
void RceFunc::Qvvr2Shm()
{
    if (!qvvr_soe_->data_num()) return;
    if (shmem_srv().qvvr_happen(ld_idx_)) return;
    
    PqmQvvr qvvr;
    pthread_mutex_lock(&mutex_);
    qvvr_soe_->pop(&qvvr);
    pthread_mutex_unlock(&mutex_);

    shmem_srv().ShmemCpy(kQvvrSOE, &qvvr, ld_idx_);
    shmem_srv().IncDataUp(1, ld_idx_);
    shmem_srv().set_qvvr_ok(ld_idx_);
}

/*!
Refresh SOE data to share memory
*/
void RceFunc::Soe2Shm()
{
    Qvvr2Shm();
    comtrd_fun_->Rdre2Shm();
}

/*!
Fetch &convert summary infomation of event will be displayed

    Input:  type -- 0=clear, 1=set
*/
void RceFunc::SetEventSummary(int type)
{
    int n, m;
    EvntSmmryInfo info;
    if (type) {
        tm tmi;
        LocalTime(&tmi, &rce_stat_.ttime.tv_sec);
        
        char tm_str[20];
        const char *ptype;    //type of event
        n = strftime(tm_str, 20, "%y%m%d_%H%M%S", &tmi);
        sprintf(&tm_str[n], ".%03d", (rce_stat_.ttime.tv_usec + 500) / 1000);
        
        const char *ptype = TypeOfEvent[rce_stat_.event_t]; //type of event
        const char *pcause = CauseOfEvent[rce_stat_.trig_c]; //Cause of event
        float val = rce_stat_.extre/100000; //10mV -> kV
        float dur = rce_stat_.dur/1000; //ms -> s

        n = FloatFmt(val, 4, 3);
        m = FloatFmt(dur, 4, 3);
        sprintf(info.str, "%s %-4s %-5s %6.*f %6.*f", tm_str, pcause, ptype, n, val, m, dur);
        evnt_smmry_info_->push(&info);
    } else {
        evnt_smmry_info_->clear();
    }

    char fname[64];
    sprintf(fname, "%sld%d.txt", EvntInfoFile, ld_idx_);
    FILE *fd = fopen(fname, "r+");
    if (!fd) fd = fopen(fname, "w+");
    //fseek(fd, 0, SEEK_SET);
    n = evnt_smmry_info_->data_num();
    fprintf(fd, "%d\n", n);
    evnt_smmry_info_->seek(0);
    for(int i = 0; i < n; i++) {
        evnt_smmry_info_->read(&info);
        fprintf(fd, "%s\n", info.str);
    }
    fclose(fd);
}

/*!
Get event summary information

    Input:  max -- maximum number be returned
    Output: info
    Return: number of info
*/
int RceFunc::GetEvntInfo(int max, EvntSmmryInfo *info)
{
    int num = evnt_smmry_info_->data_num();
    int n = num > max ? num - max : 0;
    num -= n;

    evnt_smmry_info_->seek(n);
    for(int i = 0; i < num; i++) {
        evnt_smmry_info_->read(info);
        info++;
    }
    return num;
}

/*!
Manual trigger record wave

    Input:  type -- 0=stop, 1=mannaul start, other=steady start
    Return: 0=success, -1=have event happening, -2=state not change.
*/
int RceFunc::ManualTrigger(int type)
{
    if (type) {
        if (trigger_cause()) return -1;
        if (type==1) rce_stat_.trig_c = kTrigManual;
        else rce_stat_.trig_c = kTrigSteady;
    } else {
        if (!trigger_cause()) return -2;
        rce_stat_.trig_c = kTrigNone;
    }
    return 0;
}

/*!
Set disturbance record attribute

    Input:  flt -- fault number
*/
void RceFunc::SetDreAttr(uint8_t fltn)
{
    comtrd_fun_->SetDreAttr(phy_dev().rce_tol, fltn); 
    flt_num_ = comtrd_fun_->flt_num();
    max_dur_ = phy_dev().rce_max_dur();
    max_dur_ *= 1000;
    
}


