/*! \file param_cfg.cpp
    \brief parameter configurator.
*/

#include<cmath>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include <unistd.h>
#include <stdlib.h>
#include <zlib.h>

#include "prmconfig.h"
#include "../thread/pthread_mng.h"
#include "../device/device.h"
#include "../base_func/conversion.h"
#include "../IPC/shmemfunc.h"
#include "../base_func/time_cst.h"

using namespace std;

extern int TRANST_VALID;
static const char * kPhyParamFile = "save/phy_param.cfg"; //physical device parameter config file
static const char * kLDParamFile = "save/ld_param"; //logical device parameter config file
static const char * kChnlParamFile = "save/chnl_param"; //logical device parameter config file

static const char * kSysConfFile = "save/pqm3.sys.conf"; //系统配置文件
static const char * ModifyFile = "save/pqm3_modify.conf"; //精度修正参数配置文件
static const char * DbgConfFile = "save/pqm3.dbg.conf"; //Debug配置文件
static const char * TrstRcdFile = "save/trst.save.rcd"; //暂态录波文件汇总记录
static const unsigned int DebugFileMax = 6; //debug日志文件数据最大保存数0~8

ParamCfg & param_cfg()
{
    static ParamCfg cfg;
    return cfg;
}

ParamCfg::ParamCfg()
{
}

ParamCfg::~ParamCfg()
{
}

/*!
Read physical device parameter from config file

    Input:  phdev
    Output: phdev
    Return: 0=succes, <0=failure. -1:create, -2:compress, -3:write
*/
int ParamCfg::ReadPhyParam(PhyDev *phdev)
{
    unsigned long uli = sizeof(PHDParam);
    uli += uli*0.01+12;
    uint8_t buf[uli];

    PHDParam phd_pr;
    FILE *fd = fopen(kPhyParamFile, "rb");
    bool create = false;
    if (fd) {
        int i = fread(buf, uli, 1, fd);
        i = uncompress((uint8_t*)&phd_pr, &uli, buf, i);
        if (i==Z_OK && uli==sizeof(PHDParam)) {
            if (phd_pr.version == 1) {
                //for old version upgrade
                //create = true;
            }
            phdev->set_phd_para(&phd_pr);
        } else {
            phdev->DefaultPara(&phd_pr);
            create = true;
        }
        fclose(fd);
    } else {
        phdev->DefaultPara(&phd_pr);
        create = true;
    }
    int ret = 0;
    if (create) {
        fd = fopen(kPhyParamFile, "wb");
        if (fd) {
            int i = compress(buf, &uli, (uint8_t*)&phd_pr, sizeof(phd_pr));
            if (i==Z_OK) {
                i = fwrite(buf, uli, 1, fd);
                if (i!=uli) {
                    printf("Writing file %s failure!\n", kPhyParamFile);
                    ret = -3;
                }
            } else {
                printf("Compress phd_pr failure!\n");
                ret = -2;
            }
            fclose(fd);
        } else {
            printf("Create file %s failure!\n", kPhyParamFile);
            ret = -1;
        }
    }
    return ret;
}

/*!
Read logical device parameter from config file

    Input:  ld
            idx -- index of LD. 1=LD1,2=LD2...
    Output: ld
    Return: 0=succes, <0=failure. -1:create, -2:compress, -3:write
*/
int ParamCfg::ReadLDParam(LogicDev *ld, int idx)
{
    unsigned long uli = sizeof(LDParam);
    uli += uli*0.01+12;
    uint8_t buf[uli];

    LDParam ld_pr;
    char fname[128];
    sprintf(fname, "%s%d.cfg", kLDParamFile, idx);
    FILE *fd = fopen(fname, "rb");
    bool create = false;
    if (fd) {
        int i = fread(buf, uli, 1, fd);
        i = uncompress((uint8_t*)&ld_pr, &uli, buf, i);
        if (i==Z_OK && uli==sizeof(LDParam)) {
            if (ld_pr.version == 1) {
                //for old version upgrade
                //create = true;
            }
            ld->SetParm(&ld_pr);
        } else {
            ld->DefaultPara(&ld_pr);
            create = true;
        }
        fclose(fd);
    } else {
        ld->DefaultPara(&ld_pr);
        create = true;
    }
    int ret = 0;
    if (create) {
        fd = fopen(fname, "wb");
        if (fd) {
            int i = compress(buf, &uli, (uint8_t*)&ld_pr, sizeof(ld_pr));
            if (i==Z_OK) {
                i = fwrite(buf, uli, 1, fd);
                if (i!=uli) {
                    printf("Writing file %s failure!\n", fname);
                    ret = -3;
                }
            } else {
                printf("Compress ld_pr failure!\n");
                ret = -2;
            }
            fclose(fd);
        } else {
            printf("Create file %s failure!\n", fname);
            ret = -1;
        }
    }
    return ret;
}


/*!
Read channel parameter from config file

    Input:  chl
            idx -- index of channel. 1=channel1,2=channel2...
    Output: chl
    Return: 0=succes, <0=failure. -1:create, -2:compress, -3:write
*/
int ParamCfg::ReadChnlParam(OneChannel *chl, int idx)
{
    unsigned long uli = sizeof(ChannelParam);
    uli += uli*0.01+12;
    uint8_t buf[uli];

    ParamChnl chl_pr;
    char fname[128];
    sprintf(fname, "%s%d.cfg", kLDParamFile, idx);
    FILE *fd = fopen(fname, "rb");
    bool create = false;
    if (fd) {
        int i = fread(buf, uli, 1, fd);
        i = uncompress((uint8_t*)&chl_pr, &uli, buf, i);
        if (i==Z_OK && uli==sizeof(ChannelParam)) {
            if (chl_pr.version == 1) {
                //for old version upgrade
                //create = true;
            }
            chl->set_parm_chnl(&chl_pr);
        } else {
            chl->DefaultPara(&chl_pr);
            create = true;
        }
        fclose(fd);
    } else {
        chl_pr->DefaultPara(&chl_pr);
        create = true;
    }
    int ret = 0;
    if (create) {
        fd = fopen(fname, "wb");
        if (fd) {
            int i = compress(buf, &uli, (uint8_t*)&chl_pr, sizeof(chl_pr));
            if (i==Z_OK) {
                i = fwrite(buf, uli, 1, fd);
                if (i!=uli) {
                    printf("Writing file %s failure!\n", fname);
                    ret = -3;
                }
            } else {
                printf("Compress chl_pr failure!\n");
                ret = -2;
            }
            fclose(fd);
        } else {
            printf("Create file %s failure!\n", fname);
            ret = -1;
        }
    }
    return ret;
}

