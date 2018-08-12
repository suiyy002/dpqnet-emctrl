#ifndef _PHY_DEV_H_
#define _PHY_DEV_H_
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "param_phd.h"
#include "logic_dev.h"

class PhyDev
{
public:
    PhyDev();
    ~PhyDev();
    
    void DefaultPara(ParamPHD *para);
    const uint8_t *GetParm(int *chg);
    void SetParm(int *chg, uint8 *parm);
    
    //Accessors
    const char * device_id() { return parm_phd_.device_id; };
    uint8_t freq_measpc() { return parm_phd_.freq_measpc; };
    void ldchnl_inf( LDChnnlInfo *data ) { memcpy(data, parm_phd_.ldchnl_inf, sizeof(LDChnnlInfo)*kChannelTol); };
    const uint16_t *stts_spc() { return parm_phd_.stts_spc; };
    uint16_t rce_max_dur() { return parm_phd.rce_max_dur; };
    uint8_t rce_tol() { return parm_phd.rce_tol; };
    //Mutators

private:
    void DefaultNetPara();

    ParamPHD parm_phd_;
    int prmchg_cnt_;    //parameter changed count

};

PhyDev & phy_dev();

#endif //_PHY_DEV_H_
