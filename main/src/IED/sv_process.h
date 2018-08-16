/*! \file sv_process.h
    \brief SV--Sample Value process functions.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _SV_PROCESS_H_
#define _SV_PROCESS_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>

struct ReSampSource {
    int32_t *src1[3];   //block1 start address. [0-2]:Phase A-C
    int cnt1;           //block1 count
    int32_t *src2[3];   //block2 start address. [0-2]:Phase A-C
    int cnt2;           //block2 count.
    timeval stm;    //time of first sampling point
    int valid;      //data validity. 0=valid, 1=invalid, 2=no data
    
    //switch block
    uint8_t resync; //re-synchronized at every RTC 10 min tick. 0=no, 1=need
    uint8_t cnt_cp; //count of cycle be completed when re-synchronized
    int32_t *src1_sw[3]; //block1 start address. Phase A-C
    int cnt1_sw; //count of sample point in block1
    int32_t *src2_sw[3]; //block1 start address. Phase A-C
    int cnt2_sw; //count of sample point in block2
    timeval stm_sw;

    int IniSrc(bool *sw, int *cnt, int pnt, int32_t (*val)[3][12800]);
    void SetSrc(int pnt, int32_t (*val)[3][12800], int type);
};

struct RceSource {   //datat source used to handle rce(rapid change event)
    int32_t *src1[3];   //block1 start address. [0-2]:Phase A-C
    int cnt1;           //block1 count
    int32_t *src2[3];   //block2 start address. [0-2]:Phase A-C
    int cnt2;           //block2 count.
    timeval stm;    //time of first sampling point.
    int cnt_cyc;        //total count of cycle will be used rms calculate
    int cnt_pnt[10];    //count of sampling point per cycle
    
    int IniSrc(bool sw, int cycs, int pnt, int32_t (*val)[3][12800]);
};

class SvProcess
{
protected:

public:
    SvProcess(class OneChannel *pchnl);
    ~SvProcess();

    void FluctSV(LoopBufSV<FluctBuf> *des, SmpBuf *src);
    int DetectCycs(int chnl, LoopBufSV<int> *des, ResmplInfo *src);
    void GetRsmplSrc(struct ReSampSource* rsrc, int chl, ResmplInfo* resvinf, int num10);
    int Read10cycs(LoopBufSV<int> *zcp);
    int ReSample(LoopBufSV<ResmplBuf> *des, ResmplInfo* resvinf, LoopBufSV<int> *cycinf);
    void Interpolate(int (*des)[3][kHrmSmpNum], ReSampSource* rs_src);
    
private:
    int Filter(int val){ return val; };
    void RecalcSumV1pn2(int chl, int phs, int pnts);
    void CalcRceRms(int chl, RceSource* rce_src);

    class OneChannel *one_chnnl_[kChannelTol];
    
    ReSampSource resmp_src_[kChannelTol];   //data source for re-sample
    RceSource rce_source_[kChannelTol];     //data sournce for rapid change event

    LoopBuffer<float> *v1pn2_[kChannelTol][3]; //v1p^2, square of sample value. [0-2]:A-C. v1p--voltage one point. 
    float sum_v1pn2_[kChannelTol][3];  //sum of v1pn2_

    int loop_cnt_;
};

#endif //_SV_PROCESS_H_
