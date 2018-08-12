#ifndef _LOGIC_DEV_H_
#define _LOGIC_DEV_H_
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "param_ld.h"
#include "one_channel.h"
#include "pqm_data_types.h"
#include "rce_func.h"

class LogicDev
{
public:
    LogicDev(int idx, const OneChannel **chnl);
    ~LogicDev();
    
    void RefreshData();
    void HandleRce();
    void SetRceData(int vc, int32_t **sv, RceParam *par);
    int ManualTrigger(int type);
    void Soe2Shm() { rce_func_->Soe2Shm(); };

    const uint8_t *GetParm(int *chg);
    void SetParm(uint8 *parm, int *chg = NULL);
    void SetEventSummary(int type) { SetEventSummary(type); };
    int GetEvntInfo(int max, EvntSmmryInfo *info) { return GetEvntInfo(max, info); };
    
    //Accessors
    uint8_t chnl_idx(int vc) { return channel_.idx[vc]; };
    uint16_t stat() { return channel_.stat; };
    TriggerCause trigger_cause() { return rce_func_->trigger_cause(); };
    
    //Mutators
    void set_channel(LDChnnlInfo *val) { memcpy(&channel_, val, sizeof(channel_)); };
private:
    void EndRce();
    void RefreshRcePar();
    
    uint8_t index_; //LD index
    ParamLD parm_ld_;
    int prmchg_cnt_;    //parameter changed count
    LDChnnlInfo channel_;
    const OneChannel **one_chnnl_;
    FreqParam *freq_par_;
    RceFunc *rce_func_;
    int delay_cnt_; //rce end delay
};

#endif //_LOGIC_DEV_H_
