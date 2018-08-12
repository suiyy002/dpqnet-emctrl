#include<cmath>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include "shmemfunc.h"
#include "../pqm_func/pqmfunc.h"
#include "../pqm_func/volt_variation.h"
#include "../GUI/view.h"
#include "../comm/protocol/AppPrtclB.h"

using namespace std;

#define SQRT30 sqrt(3.0)
#define NORMAL_RMS_U (syspara->line_para.loopPar[0].IT[1] /SQRT30)
#define NORMAL_RMS_I (syspara->line_para.loopPar[1].IT[1]/SQRT30)

ShmemSrv &shmem_srv()
{
	static ShmemSrv shm_srv;
	return shm_srv;
}

ShmemSrv::ShmemSrv()
{
    pthread_mutex_init(&mutex_, NULL); //Initailez share memory mutex
    pshmem_ = InitShmSvr61850(); //Create share memory and initialize.
    pshmem_->request_cmd = kNoneCmd;
    sys_para_sg = prmcfg->sys_para_sg();
    syspara = prmcfg->syspara();
    trst_rcd = prmcfg->trst_rcd();
    pshmem_->brcb_up = 1;
    pshmem_->urcb_up = 1;
    //printf("PQMShmem=%d\n", sizeof(PQMShmem));
    pshmem_->rdre.rcdtrg = 0;
    pshmem_->rdre.rcdmade = 0;
    pshmem_->rdre.fltnum = 0;

    time_t time1 = time(NULL);
    memset(&pshmem_->ggio, 0, sizeof(PqmGgio));
    for (int i; i<113; i++) {
        pshmem_->ggio.Alm_t[i] = time1;
    }
    heartbeat_cnt_ = 0;
}

ShmemSrv::~ShmemSrv()
{
    FreeShareMemmory(pshmem_);
    pthread_mutex_destroy(&mutex_); 
}

/*!
Description:copy data to sharemem or from sharemem

    Input:  datatype -- for detail to see ShmemDataType
            q -- value of quality
            type -- 0=maximum, 1=average, 2=minimum, 3=CP95
            subclass -- subclass of datatype
*/
void ShmemSrv::SetQuality(ShmemDataType datatype, unsigned short q, int type, int subclass)
{
    switch(datatype) {
        case kMhaiReal:
            pshmem_->mhai_real.q = q;
            break;
        case kMhaiInReal:
            pshmem_->mhai_in_real.q = q;
            break;
        case kMmxuReal:
            if (subclass) pshmem_->mmxu_real.hz_q = q;
            else pshmem_->mmxu_real.q = q;
            break;
        case kMsqiReal:
            pshmem_->msqi_real.q = q;
            break;
        case kMhaiStat:
            pshmem_->mhai_stat[type].q = q;
            break;
        case kMhaiInStat:
            pshmem_->mhai_in_stat[type].q = q;
            break;
        case kMmxuStat:
            if (subclass) pshmem_->mmxu_stat[type].hz_q = q;
            else pshmem_->mmxu_stat[type].q = q;
            break;
        case kMsqiStat:
            pshmem_->msqi_stat[type].q = q;
            break;
        case kMflkData:
            pshmem_->mflk.q[subclass] = q; //subclass:0=pst,1=plt,2=fluct
            break;
        default:
            break;
    }
}

/*!
Description:copy data to sharemem or from sharemem

    Input:  datatype -- for detail to see ShmemDataType
            tofrom -- data be copy to sharemem
            idx -- LD index
            type -- 0=maximum, 1=average, 2=minimum, 3=CP95
    Output: tofrom -- data be copy from sharemem            
*/
void ShmemSrv::ShmemCpy(ShmemDataType datatype, void * tofrom, int idx, int type)
{
    void * pshm;
    bool beto = true;
    int count;
    struct timeval tmp_time;
    int i;
    
    switch(datatype) {
        case kMhaiReal:
            pshm = &pshmem_->ld_data[idx].mhai_real;
            count = sizeof(PqmMhai);
            break;
        case kMhaiInReal:
            pshm = &pshmem_->ld_data[idx].mhai_in_real;
            count = sizeof(PqmMhaiIntr);
            break;
        case kMmxuReal:
            pshm = &pshmem_->ld_data[idx].mmxu_real;
            count = sizeof(PqmMmxu);
            break;
        case kMsqiReal:
            pshm = &pshmem_->ld_data[idx].msqi_real;
            count = sizeof(PqmMsqi);
            break;
        case kMhaiStat:
            pshm = &pshmem_->ld_data[idx].mhai_stat[type];
            count = sizeof(PqmMhai);
            break;
        case kMhaiInStat:
            pshm = &pshmem_->ld_data[idx].mhai_in_stat[type];
            count = sizeof(PqmMhaiIntr);
            break;
        case kMmxuStat:
            pshm = &pshmem_->ld_data[idx].mmxu_stat[type];
            count = sizeof(PqmMmxu);
            break;
        case kMsqiStat:
            pshm = &pshmem_->ld_data[idx].msqi_stat[type];
            count = sizeof(PqmMsqi);
            break;
        case kQvvrSOE:
            pshm = &pshmem_->ld_data[idx].qvvr;
            count = sizeof(PqmQvvr);
            break;
        case kRdreSOE:
            pshm = &pshmem_->ld_data[idx].rdre;
            count = sizeof(PqmRdre);
            break;
        default:
            break;
    }
    pthread_mutex_lock(&mutex_);   //¼ÓËø
    if (beto) { // write to share memory
        memcpy(pshm, tofrom, count);
    } else {    // read from share memory
        memcpy(tofrom, pshm, count);
    }
    pthread_mutex_unlock(&mutex_); //½âËø
}

void ShmemSrv::ConfirmEditSG()
{
    int n = sys_para_sg->edit_SG;
    if ((n > (sys_para_sg->total_SG)) || (n <= 0)) {
        pshmem_->resp_rec_num = -1;
        return;
    }

    memcpy(&sys_para_sg->paraSG[n - 1], pshmem_->data, sizeof(SysPara));
    if (n==sys_para_sg->active_SG) {
       	memcpy(syspara, &sys_para_sg->paraSG[n - 1], sizeof(SysPara));
        pshmem_->resp_rec_num = sizeof(SysPara);
    } else {
        pshmem_->resp_rec_num = 0;
    }
    notice_pthread(kTTSave, SAVEINFO, kSysParam, NULL);
}

void ShmemSrv::TreatShareCmd()
{
    if (pshmem_->request_cmd>0) {
        heartbeat_cnt_++;
        //printf("heartbeat_cnt_=%d, cmd = %d\n", heartbeat_cnt_, pshmem_->request_cmd);
    }
    switch (pshmem_->request_cmd) {
        case kInitSG:
            init_SG();
            break;
        case kSelectActSG:
        case kSelectEditSG:
            SelectSG(pshmem_->request_cmd);
            break;
        case kConfEditSG:
            ConfirmEditSG();
            break;
        case kStartRcdWave:
            RecordWave(true);
            printf("received kStartRcdWave\n");
            break;
        case kEndRcdWave:
            printf("received kEndRcdWave\n");
            RecordWave(false);
        default:
            break;
    }
    pshmem_->response_cmd = pshmem_->request_cmd;
    pshmem_->request_cmd = kNoneCmd;
}

/*!
Description:Start or end record wave

    Input:  type -- true=start, false=end
*/
void ShmemSrv::RecordWave(bool type)
{
    if (type) pshmem_->resp_rec_num = -1;
    else pshmem_->resp_rec_num = 0;

    if (prmcfg->manual_rec_enable()) {
        if (!volt_variation->ManualTrigger(type)) {
            gettimeofday(&pshmem_->rdre.trig_time, NULL);
            pshmem_->rdre.rcdtrg = type;
            pshmem_->resp_rec_num = 0;
        }
    }
            pshmem_->resp_rec_num = 0;
}

/*------------------------------------------------------------------------------
Description:Select the SG ,and fetch the data of this SG
Input:      SGtype -- kSelectActSG,kSelectEditSG
------------------------------------------------------------------------------*/
void ShmemSrv::SelectSG(shm_APICmd SGtype)
{
    bool be_save = false;
    int n = pshmem_->data[0];
    
    switch (SGtype) {
        case kSelectActSG:
            if (n>sys_para_sg->total_SG || n <= 0) {
    	    	pshmem_->resp_rec_num = -1;
                return;
            }
            if (sys_para_sg->active_SG!=n) {
                sys_para_sg->active_SG = n;
            	memcpy(syspara, &sys_para_sg->paraSG[n - 1], sizeof(SysPara));
                be_save = true;
            }
            break;
        case kSelectEditSG:
            if (n>sys_para_sg->total_SG || n < 0) {
    	    	pshmem_->resp_rec_num = -1;
                return;
            }
            if (sys_para_sg->edit_SG!=n) {
                sys_para_sg->edit_SG = n;
                be_save = true;
            } 
            break;
        default:
            return;
    }
          
    if (!n) {
	    memset(pshmem_->data, 0, sizeof(SysPara));
    } else {
        memcpy(pshmem_->data, &sys_para_sg->paraSG[n-1], sizeof(SysPara));
    }
    pshmem_->resp_rec_num = sizeof(SysPara);
    if (be_save) notice_pthread(kTTSave, SAVEINFO, kSysParam, NULL);
}

void ShmemSrv::init_SG(void)
{
    pshmem_->data[0] = sys_para_sg->total_SG;
    pshmem_->data[1] = sys_para_sg->active_SG;
    pshmem_->data[2] = sys_para_sg->edit_SG;
    pshmem_->resp_rec_num = 3;
}

/*!
Description:Set Alm value

    Input:  type --
            buf -- set value buffer
            num -- number of value
*/
void ShmemSrv::SetAlm(AlmType type, unsigned short *buf, int num, time_t time)
{
    unsigned short * pbuf = (unsigned short *)buf;
    int sz = kAlmRmsVUplimt - kAlmHRU;
    char alm[sz];
    if (type==kAlmTHDu) {
        memset(alm, 0, sz);
        for (int i=0;i<num;i++) {
            int order = *pbuf&0xff;    //harmonic order
            if ((*pbuf>>8)%2) order += 48;   //current
            else order -= 1; //voltage
            alm[order] = 1;
            pshmem_->ggio.Alm_t[type+order] = time;
            pbuf++;
        }
    } else {
        memset(alm, 0, num);
        unsigned short usi = *buf;
        //printf("SetAlm type=%d, usi=%d\n", type, usi);
        for (int i=0;i<num;i++) {
            if ((usi>>i)&1) {
                pshmem_->ggio.Alm_t[type] = time;
                alm[i] = 1;
            }
        }
        sz = num;
    }
    memcpy(&pshmem_->ggio.Alm_stVal[type], alm, sz);
    if (!pshmem_->ggio_ok) pshmem_->ggio_ok = 1;
}

/*!
    Called by:  thread_timer
*/
void ShmemSrv::IniAlmTime(time_t time)
{
    for (int i=0; i<=kAlmRmsADiflimt; i++) pshmem_->ggio.Alm_t[i] = time;
}

/*!
Description:Set MFLK data

    Input:  type --
            data -- set value buffer address
            mode -- 0 = Wye circuit, 1= Delta circuit
            time -- refresh time
*/
void ShmemSrv::SetMflk(MflkType type, float *data, int mode, time_t time)
{
    switch (type) {
        case kMflkPst:
            pshmem_->mflk.st_time = time;
            //if (mode) memcpy(pshmem_->mflk.pppst, data, sizeof(float)*3);
            //else memcpy(pshmem_->mflk.phpst, data, sizeof(float)*3);
            memcpy(pshmem_->mflk.pppst, data, sizeof(float)*3);
            memcpy(pshmem_->mflk.phpst, data, sizeof(float)*3);
            if (!pshmem_->flicker_ok) pshmem_->flicker_ok = 1;
            break;
        case kMflkPlt:
            pshmem_->mflk.lt_time = time;
            //if (mode) memcpy(pshmem_->mflk.ppplt, data, sizeof(float)*3);
            //else memcpy(pshmem_->mflk.phplt, data, sizeof(float)*3);
            memcpy(pshmem_->mflk.ppplt, data, sizeof(float)*3);
            memcpy(pshmem_->mflk.phplt, data, sizeof(float)*3);
            break;
        case kMflkFluc:
            //if (mode) memcpy(pshmem_->mflk.ppfluc, data, sizeof(float)*3);
            //else memcpy(pshmem_->mflk.phfluc, data, sizeof(float)*3);
            memcpy(pshmem_->mflk.ppfluc, data, sizeof(float)*3);
            memcpy(pshmem_->mflk.phfluc, data, sizeof(float)*3);
            break;
        case kMflkFlucf:
            //if (mode) memcpy(pshmem_->mflk.ppflucf, data, sizeof(float)*3);
            //else memcpy(pshmem_->mflk.phflucf, data, sizeof(float)*3);
            memcpy(pshmem_->mflk.ppflucf, data, sizeof(float)*3);
            memcpy(pshmem_->mflk.phflucf, data, sizeof(float)*3);
        default:
            break;
    }
}

/*!
Description:Set LPHD data

    Input:  type --
            data -- set value buffer address
            time -- refresh time
*/
void ShmemSrv::SetLphd(LphdType type, void *data, time_t time)
{
    switch (type) {
        case kLphdPwrOn:
            memcpy(&pshmem_->lphd.pwr_on, data, sizeof(char));
            pshmem_->lphd.on_time = time;
            if (!pshmem_->lphd_ok) pshmem_->lphd_ok = 1;
            break;
        case kLphdPwrOff:
            memcpy(&pshmem_->lphd.pwr_off, data,  sizeof(char));
            pshmem_->lphd.off_time = time;
            break;
        default:
            break;
    }
}

/*!
Description:Set RDRE data

    Input:  type --
            data -- set value buffer address
            time -- refresh time
*/
void ShmemSrv::SetRdre(RdreType type, void *data, timeval *time)
{
    char chi;
    switch (type) {
        case kRdreRcdmd:
            SafeSetShm(kDRcdMade, data);
            if (time) {
                memcpy(&pshmem_->rdre.time, time, sizeof(timeval));
            }
            memcpy(&chi, data, 1);
            if (!chi) pshmem_->rdre.happen = 1;
            if (chi&&!pshmem_->rdre_ok) pshmem_->rdre_ok = 1;
            break;
        case kRdreFltnm:
            memcpy(&pshmem_->rdre.fltnum, data,  sizeof(int));
            break;
        case kRdreRcdTrg:
            pshmem_->rdre.rcdtrg = *(char*)data;
            break;
        default:
            break;
    }
}

/*!
Description:Set MMXU data

    Input:  type --
            data -- set value buffer address
            stype -- stat type. 0=maximum, 1=average, 2=minimum, 3=CP95            
            time -- refresh time
*/
void ShmemSrv::SetMmxu(MmxuType type, float *data, int stype, time_t time)
{
    switch (type) {
        case kMmxuHz:
            memcpy(&pshmem_->mmxu_stat[stype].hz, data, sizeof(float));
            if(time) {
                pshmem_->mmxu_stat[stype].hz_time = time;
            }
            break;
        case kMmxuHzdev:
            memcpy(&pshmem_->mmxu_stat[stype].hzdev, data, sizeof(float));
            break;
        case kMmxuA:
            memcpy(pshmem_->mmxu_stat[stype].a_mag, data, sizeof(float)*3);
            if(time) {
                pshmem_->mmxu_stat[stype].time = time;
            }
            break;
        case kMmxuPhv:
            memcpy(pshmem_->mmxu_stat[stype].phv_mag, data, sizeof(float)*3);
            break;
        case kMmxuPpv:
            memcpy(pshmem_->mmxu_stat[stype].ppv_mag, data, sizeof(float)*3);
            break;
        case kMmxuPhvdev:
            memcpy(pshmem_->mmxu_stat[stype].phvdev, data, sizeof(float)*3);
            break;
        case kMmxuPpvdev:
            memcpy(pshmem_->mmxu_stat[stype].ppvdev, data, sizeof(float)*3);
            break;
        case kMmxuW:
            memcpy(pshmem_->mmxu_stat[stype].w, data, sizeof(float)*4);
            break;
        case kMmxuVar:
            memcpy(pshmem_->mmxu_stat[stype].var, data, sizeof(float)*4);
            break;
        case kMmxuVa:
            memcpy(pshmem_->mmxu_stat[stype].va, data, sizeof(float)*4);
            break;
        case kMmxuPf:
            memcpy(pshmem_->mmxu_stat[stype].pf, data, sizeof(float)*4);
            break;
        default:
            break;
    }
}

/*!
Increase Report control block data update count

    Input:  type -- 0=real, 1=stat
            idx -- LD index
*/
void ShmemSrv::IncDataUp(int type, int idx)
{
    char chi;
    if (!type) {
        chi=pshmem_->ld_data[idx].urcb_up;
        chi++;
        SafeSetShm(kUrcbUpdate, &chi, idx); 
    } else {
        chi=pshmem_->ld_data[idx].brcb_up;
        chi++;
        SafeSetShm(kBrcbUpdate, &chi, idx);
    }
};


/*!
Set realtime data

    Input:  idx -- LD index. 0=LD1, 1=LD2
            type --
            data -- set value buffer address
            time -- refresh time
*/
void ShmemSrv::SetReal(int idx, ShmemRealType type, void *data, time_t time)
{
    switch (type) {
        case kRealFreq:
            memcpy(&pshmem_->ld_data[idx].mmxu_real.hz, data, sizeof(float));
            pshmem_->ld_data[idx].mmxu_real.hzdev = pshmem_->ld_data[idx].mmxu_real.hz-50;
            pshmem_->ld_data[idx].mmxu_real.hz_time = time;
            break;
        default:
            break;
    }
}

/*!
Set statistic data

    Input:  idx -- LD index. 0=LD1, 1=LD2
            type --
            data -- set value buffer address
            time -- refresh time
*/
void ShmemSrv::SetStatis(int idx, ShmemStatisType type, float *data, time_t time)
{
    int i;
    switch (type) {
        case kStatisFreq:
            for (i=0; i<4; i++) {
                pshmem_->ld_data[idx].mmxu_stat[i].hz = data[i];
                pshmem_->ld_data[idx].mmxu_stat[i].hzdev = data[i]-50;
                pshmem_->ld_data[idx].mmxu_stat[i].hz_time = time;
            }
            break;
        default:
            break;
    }
}
