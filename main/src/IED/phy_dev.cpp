#include "prmconfig.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <sys/time.h>

using namespace std;

PhyDev & phy_dev()
{
    static PhyDev phd;
    return phd;
}

PhyDev::PhyDev()
{
    chnl_stat_ = 0;
    prmchg_cnt_ = 1;
}

PhyDev::~PhyDev()
{

}

/*!
Read physical device parameter default value

    Output:  para
*/
void PhyDev::DefaultPara(ParamPHD *para)
{
    memset(&para_phd_, 0, sizeof(para_phd_));
    para_phd_.version = 1;
    strcpy(para_phd_.device_id, "PQNet300D000001");
    para_phd_.device_model = 4002;
    para_phd_.lcm_dely_time = 5;
    para_phd_.stts_spc[kPQParaFreq] = 10;
    para_phd_.stts_spc[kPQParaHrm] = 180;
    para_phd_.stts_spc[kPQParaUnblc] = 180;
    para_phd_.stts_spc[kPQParaUdev] = 180;
    DefaultNetPara();
    memset(&para_phd_.ldchnl_inf, 0, sizeof(ldchnl_inf));
    
    memcpy(para, para_phd_, sizeof(para_phd_));
}

/*!
Read network parameter default value
*/
uint8_t ip1[4] = {192, 168, 1, 100}; 
uint8_t ip2[4] = {192, 168, 32, 100}; 
uint8_t mask[4] = {255, 255, 255, 0}; 
uint8_t mac[2] = {1, 1}; 
uint8_t gate1[4] = {192, 168, 1, 1};
uint8_t gate2[4] = {192, 168, 32, 1};
void PhyDev::DefaultNetPara()
{
    memcpy(para_phd_.ip[0], ip1, sizeof(ip1));
    memcpy(para_phd_.ip[1], ip2, sizeof(ip1));
    para_phd_.port[0] = 31024;
    para_phd_.port[1] = 51024;
    memcpy(para_phd_.mask[0], mask, sizeof(mask));
    memcpy(para_phd_.mask[1], mask, sizeof(mask));
    memcpy(para_phd_.mac[0], mac, sizeof(mac));
    memcpy(para_phd_.mac[1], mac, sizeof(mac));
    memcpy(para_phd_.gate[0], gate, sizeof(gate));
    memcpy(para_phd_.gate[1], gate1, sizeof(gate1));
}

/*!
Get physical device parameter -- parm_phd_

    Input:  chg -- change count
    Output: chg
    Return: parameter. NULL=parameter not change
*/
const uint8_t *PhyDev::GetParm(int *chg)
{
    if (*chg==prmchg_cnt_) {
        return NULL;
    } else {
        *chg = prmchg_cnt_;
        return &parm_phd_;
    }
}

/*!
Set physical device parameter -- parm_phd_

    Input:  chg -- change count
            parm -- physical parameter
    Output: chg
*/
void PhyDev::SetParm(int *chg, uint8 *parm)
{
    memcpy(&parm_phd_, data, sizeof(parm_phd_));
    prmchg_cnt_++;
    *chg = prmchg_cnt_;
}



