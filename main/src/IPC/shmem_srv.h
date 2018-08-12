#ifndef _SHMEM_SRV_H_
#define _SHMEM_SRV_H_
#include "share_mem.h"
#include "../pqm_func/prmconfig.h"
#include <pthread.h>
#include<cstdio>
#include<cstdlib>
#include<cstring>

enum ShmemRealType {
	kRealFreq,
};
enum ShmemStatisType {
	kStatisFreq,
};


enum ShmemDataType {
	kMhaiReal, kMhaiInReal, kMmxuReal, kMsqiReal,
	kMhaiStat, kMhaiInStat, kMmxuStat, kMsqiStat,
	kQvvrSOE, kRdreSOE, kMflkData
};
enum MflkType {	kMflkPst, kMflkPlt, kMflkFluc, kMflkFlucf };
enum LphdType {	kLphdPwrOn, kLphdPwrOff };
enum RdreType {	kRdreRcdmd, kRdreFltnm, kRdreRcdTrg};
enum QvvrType {	kQvvrVva, kQvvrVvatm, kQvvrVarstr, kQvvrVarend,
                kQvvrDipstr, kQvvrSwlstr, kQvvrIntrstr };
enum MmxuType {	kMmxuHz, kMmxuPhv, kMmxuPpv, kMmxuA, kMmxuW, kMmxuVar, kMmxuVa,
                kMmxuPf, kMmxuHzdev, kMmxuPhvdev, kMmxuPpvdev,};

#define Q_Questionable 0xc0
#define Q_Inaccurate 0x4000

class ShmemSrv
{
public:
    ShmemSrv();
    ~ShmemSrv();
    
    void TreatShareCmd(void);
    void ShmemCpy(ShmemDataType datatype, void * tofrom, int idx, int type=0);
    void SetQuality(ShmemDataType datatype, unsigned short q, int type=0, int subclass=0);
    void SetAlm(AlmType type, unsigned short *buf, int num, time_t time);
    void IniAlmTime(time_t time);
    void SetMflk(MflkType type, float *data, int mode, time_t time=0);
    void SetLphd(LphdType type, void *data, time_t time);
    void SetRdre(RdreType type, void *data, timeval *time);
    void SetMmxu(MmxuType type, float *data, int stype, time_t time=0);
    void IncDataUp(int type);
    
    //Accessors
    char *station_name() { return pPqmshm->StationName; }
    char *monitor_point() { return pPqmshm->MonitorPoint; }
    char *ied_name() { return pPqmshm->IEDName; }
    char *ld_name(int idx) { return pPqmshm->ld_data[idx].LDName; }
    char dipstr() { return pPqmshm->qvvr.dipstr; };
    int heartbeat_cnt() { return heartbeat_cnt_; };
    char intrstr() { return pPqmshm->qvvr.intrstr; };
    char swlstr() { return pPqmshm->qvvr.swlstr; };
    char quit_cmd() { if (pPqmshm->quit_cmd==31729) return 1; else return 0; };
    char qvvr_happen(int idx) { return pPqmshm->ld_data[idx].qvvr.happen; };
    char rdre_happen(int idx) { return pPqmshm->ld_data[idx].rdre.happen; };

    //Mutators
    void set_qvvr_ok(int idx) { if(!pPqmshm->ld_data[idx].qvvr_ok) pPqmshm->ld_data[idx].qvvr_ok = 1; };
    void set_rdre_ok(int idx) { if(!pPqmshm->ld_data[idx].rdre_ok) pPqmshm->ld_data[idx].rdre_ok = 1; };
    void set_rdre_ok() { if(!pPqmshm->qvvr_ok) pPqmshm->qvvr_ok = 1; };
    void set_statistic_ok() { if(!pPqmshm->statistic_ok) pPqmshm->statistic_ok = 1; };
    void set_stat_note(bool val) { stat_note_=val; };
    bool stat_note() { return stat_note_; };
protected:

private:
    void init_SG();
    void SelectSG(shm_APICmd SGtype);
	void ConfirmEditSG();
    void RecordWave(bool type);
    void get_pqdift_share();
    
    ShmemSvr61850 *pshmem_; //pointer to share memory
    pthread_mutex_t mutex_; //mutex lock
	SysParaSG * sys_para_sg;
	SysPara *syspara;
	TrstRcd *trst_rcd;
	bool stat_note_;    //statistic note. note WriteReal2Shm to stop invoking inc_data_up 
	int heartbeat_cnt_;    //heartbeat count
};

ShmemSrv &shmem_srv();

#endif //_SHMEM_SRV_H_


