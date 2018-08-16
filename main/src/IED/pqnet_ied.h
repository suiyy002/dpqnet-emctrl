/*! \file pqnet_ied.h
    \brief PQNet IED -- Intelligent Electronic Device.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _PQNET_IED_H_
#define _PQNET_IED_H_
#include <cstdio>
#include <cstdlib>
#include <cstring>

static const int kPstSmpNum = 320;  //sample data number for Pst per 0.2s(¡Ö10cycle)

class PqnetIED
{
protected:
struct ResmplBuf {
    timeval t1st[kChannelTol];   //time of 1st point
    int reserved[kChannelTol];
    int val[kChannelTol][3][kHrmSmpNum];  //[0-2]:A-C
};

struct FluctBuf {   //Voltage fluctuation sample data. pst input
    time_t t1st;    //time of 1st point
    int val[kChannelTol][3][kPstSmpNum];  //[0-3]:channel; [0-2]:A-C
};

struct CycsInfo {
    LoopBufSV<int> *zcp;   //number of sample points per cycle
    int point;      //start point for next process
};
struct SV_1sBlock;

struct ResmplInfo {
    LoopPointer<SV_1sBlock> *sv_buf;   //sample value buffer
    int point; //zero crossing point in 1st 1s block be read last time
};

public:
    PqnetIED();
    ~PqnetIED();

    void ReadSV(int wait);
    void PqnetIED::HandleSV(int wait);
    int ManualTrigger(int ld, int type);

    //Accessors
    const uint8_t *prm_chnl(int idx, int &chg) { return one_channel_[idx]->GetParm(chg); };
    const uint8_t *prm_ld(int idx, int &chg) { return logic_dev_[idx]->GetParm(chg); };
    //Mutators
    void set_prm_chnl(int idx, uint8 *data) { one_channel_[idx]->SetParm(chg, data); };
    void set_prm_ld(int idx, uint8 *data) { logic_dev_[idx]->SetParm(chg, data); };
private:
    int CallFPGA(void *fft_rbuf, void *fft_tbuf, void *pst_rbuf, void *pst_tbuf);
    void IniLDChnnl();
    void DeriveChnlAttr(ChnnlAttr* attr, LDChnnlInfo *chinf);
    void HandleRce();

    LoopBufSV<SV_1sBlock> *smplv_buf_;
    LoopBufSV<ResmplBuf> *resv_buf_;
    LoopBufSV<FluctBuf> *fluct_buf_;

    ResmplInfo cycs_pnt_[kChannelTol];    //address array of sv will be used for detect cycle. [0-3]:channel
    ResmplInfo resv_inf_[kChannelTol];    //address array of sv will be used for resampling. [0-3]:channel
    LoopBufSV<int> *cycs_inf_[kChannelTol];   //number of sampling point per cycle. [0-3]:channel

    class SvProcess *resample_;
    class LogicDev *logic_dev_[kChannelTol];
    class OneChannel *one_channel_[kChannelTol];
    class SpiDev *spi_dev_;
    class SpiApi *spi_api_;
    
    int fft_rbuf_[kChannelTol][3][640*2];   //fft data receive buffer.  [0-(kChannelTol-1)]:channel;
                                            //[0-2]:PhaseA-C; 2n=real, 2n+1=image
    float pst_rbuf_[kChannelTol][3][8]; //fluctuation data receive buffer.  [0-3]:channel; [0-2]:PhaseA-C;

    int order_10cyc_;   //the harmonic that need output continue 10cycle data
    int matchw_delay_;  //channel type match warning delay.
};

PqnetIED & pqnet_ied();

#endif //_PQNET_IED_H_
