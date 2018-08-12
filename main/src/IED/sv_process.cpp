/*! \file smpl_val.cpp
    \brief sample value pre-process.
*/

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <sys/time.h>
#include "one_channel.h"

using namespace std;

SvProcess::SvProcess(OneChannel **pchnl)
{
    memcpy(one_chnnl_, pchnl, 4 * kChannelTol);
    for (int i=0; i<kChannelTol; i++) {
        for (int j=0; j<3; j++) {
            v1pn2_[i][j] = new LoopBuffer<float>(kSmpFreq);
        }
    }
    memset(sum_v1pn2_, 0, sizeof(sum_v1pn2_));
    loop_cnt_ = 0;
}

SvProcess::~SvProcess()
{
    for (int i=0; i<kChannelTol; i++) {
        for (int j=0; j<3; j++) {
            delete v1pn2_[i][j];
        }
    }
}

/*!
Voltage fluctuation sample value

    Input:  src -- sample value
    Output: des -- voltage fluactuation sample value
*/
void SvProcess::FluctSV(LoopBufSV<FluctBuf> *des, SV_1sBlock *src)
{
    FluctBuf buf;
    int ofst = 0;

    memset(&buf, 0, sizeof(buf));
    buf.t1st.tv_sec = src->t1st;
    buf.t1st.tv_usec = 0;
    do {
        for (int i = 0; i < kChannelTol; i++) {
            if (src->type[i] == 1) {    //is voltage channel
                for (int j = 0; j < 3; j++) {
                    for (int k = 0; k < 320; k++) {
                        buf.val[i][j][k] = src->val[i][j][ofst + k * 8];
                    }
                }
            }
        }
        ofst += 2560;   //320*8
        buf.t1st.tv_usec += 200000;   //1000000*2560/12800 = 0.2s
        des->Push(&buf);
    } while(ofst < src->num);
}

/*!
Detect whole cycle

    Input:  idx -- channel index. 1~kChannelTol
            src --
    Output: des --
    Return: 0=no more, 1=have more
*/
int SvProcess::DetectCycs(int idx, LoopBufSV<int> *des, ResmplInfo *src)
{
    static cnt0 = 0;    //weak signal detect threshold counter.
    SV_1sBlock *pb;
    int val, *pdata, posi = 20;
    idx -= 1;
    int zc_pnt = src->point;    //last zero crossing point
    int cycnum = 0; //number of sample points per cycle
    for (int j = 0; j < 2; j++) {
        pb = src->sv_buf->Read(j);
        if (!pb) return 0;
        if (!one_chnnl_[idx]->chl_mode()) return 1; //not master
        if (j == 0) {   //1st 1s sv block
            int k = src->point;
            pdata = &pb->val[idx][0][k];
            cnt = bp->num - k;
        } else {        //2nd 1s sv block
            pdata = &pb->val[idx][0][0];
            cnt = pb->num;
            src->point = zc_pnt;   //last zero crossing point
            zc_pnt = 0;
        }
        for (int i = 0; i < cnt; i++) {
            val = Filter(pdata[i]);
            if (val < 0) posi = 0;
            else posi++;
            cycnum++

            // zero crossing points. change from negativ to positive
            if (posi == 16 || cycnum > 320 ) { // 320=40Hz for 12.8kHz
                des->Push(&cycnum);
                zc_pnt += cycnum;
                cycnum = 0;
                cnt0 = 0;
            }
        }
    }
    if (zc_pnt > 0) {
        src->point = zc_pnt;
        src->sv_buf->Pop();
    }

    return 1;
}

/*!
Resample by interpolate algorithm.

    Input:  rs_src -- resample source
    Output: des3 -- resampled value. [0-2]:A-C
*/
void SvProcess::Interpolate(int (*des3)[3][kHrmSmpNum], ReSampSource* rs_src)
{
    if (rs_src->valid) {    //data is invalid
        memset(des3, 0, sizeof(int)*kHrmSmpNum*3);
        return;
    }

    //Merge 2 sector into 1 sector
    int cnt1 = rs_src->cnt1;
    int cnt2 = rs_src->cnt2;
    int scnt = cnt1 + cnt2; //number of source data
    if (scnt <= 0) return;
    int *src = new int[scnt];
    if (!src) {
        printf("system error! %s %s\n", __FILE__, __FUNCTION__);
        return;
    }
    for (int i=0; i<3; i++) {
        int32_t *src1 = rs_src->src1[i];
        int32_t *src2 = rs_src->src2[i];
        if (src1) memcpy(src, src1, cnt1 * sizeof(int));
        if (src2) memcpy(&src[cnt1], src2, cnt2 * sizeof(int));

        //Iterpolate
        int sdot, dx, dy;
        int *des = des3[i];
        int dcnt = kHrmSmpNum;  //number of result value
        *des = *src;
        des++;
        for (int k = 1; k < dcnt; k++) {
            sdot = k * scnt / dcnt;
            dx = (k * 100 * scnt + dcnt / 2) / dcnt - sdot * 100; //unit:%
            dy = ((src[sdot + 1] - src[sdot]) * dx + 50) / 100;
            *des = src[sdot] + dy;
            des++;
        }
    }
    delete [] src;
}

/*!
resample sample value.

    Input:  resvinf -- Resample information
            cycinf -- cycles information
    Output: des -- resampled result
    Return: 1=have more; 0=no more;
*/
int SvProcess::ReSample(LoopBufSV<ResmplBuf> *des, ResmplInfo* resvinf, LoopBufSV<int> *cycinf)
{
    SV_1sBlock *bufp;
    int num10, have10 = 0;
    for (int i = 0; i < kChannelTol; i++) { //detect source data parameter
        bufp = resvinf[i].sv_buf_->Read(0);
        resmp_src_[i].stm.tv_sec = bufp->t1st;
        resmp_src_[i].stm.tv_usec = (resvinf[i].point * 1000 + bufp->num / 2) / bufp->num;
        resmp_src_[i].stm.tv_usec *= 1000;
        memcpy(&rce_source_[i].stm, &resmp_src_[i].stm, sizeof(timeval));
        if ( one_chnnl_[i]->chl_mode() == 1) { //be master channel
            num10 = GetRsmplSrc(i, resvinf, cycinf);
            if (resmp_src_[i].valid < 2 && num10 > 0) {
                if (resmp_src_[i].valid) num10 = 0;
                one_chnnl_[i]->MeasureFreq(num10, &resmp_src_[i].stm.tv_sec);
                have10 = 1;
            }
        }
    }
    if (have10 == 0) return 0;

    ReSmplBuf *pbuf = des->PushP();
    for (int i = 0; i < kChannelTol; i++) {
        memcpy(&pbuf->t1st[i], &resmp_src_[i].stm, sizeof(timeval));
        Interpolate(pbuf->val[i], resmp_src_[i]);
        CalcRceRms(i, rce_source_[i]);
    }
    if(++loop_cnt_>=20) loop_cnt_ = 0;
    return 1;
}

/*!
Get data source for resample.

    Input:  chl -- voltage channel be processed
            resvinf -- Resample information
            zcp -- zero cross point information
    Variable: resmp_src_, rce_source_
    Return: sample point number in 10cycles.
*/
int SvProcess::GetRsmplSrc(int chl, ResmplInfo* resvinf, LoopBufSV<int> *zcp)
{
    if (zcp->data_num() < 10) return 0;

    SV_1sBlock *bufp = resvinf[chl].sv_buf_->Read(0);
    int pnt = resvinf[chl].point;
    ReSampSource *rs_src = &resmp_src_[chl];
    RceSource *rce_src = &rce_source_[chl];
    
    bool sw;
    int cnt0;
    int cycs = rs_src->IniSrc(&sw, &cnt0, pnt, bufp->val[chl]);
    rce_src->IniSrc(sw, cycs, pnt, bufp->val[chl]);
    int cnt = cnt0;
   
    int smfrq = bufp->num;   //sample frequency. 12800Hz or 4000Hz
    int *nmp, k=0;
    for (int i = 0; i < cycs; i++) {
        nmp = zcp->Pop();
        if (nmp) {
            cnt += *nmp;
            rce_src->cnt_pnt[i] = *nmp;
            if ( *nmp < 180 || *nmp > 320 ) { // 180≈70Hz, 320=40Hz for 12.8kHz
                rs_src->valid = 1;
            }
        } else {
            rs_src->valid = 2;
        }
        if (pnt + cnt > smfrq && !sw) {   //switch block
            rs_src->cnt1 = smfrq - pnt;
            resvinf[chl].sv_buf_->Pop();
            rs_src->cnt_cp = i;

            if (pnt + cnt < smfrq + 128) { //start from block2
                rs_src->cnt_cp += 1;
                k = pnt + cnt - smfrq;
            } else {    //start from block1
                k = pnt + cnt - *nmp;
                rs_src->cnt1_sw = smfrq - k;
                rs_src->SetSrc(k, bufp->val[chl], 2);    //src1_sw
                k = 0;
            }
            bufp = resvinf[chl].sv_buf_->Read(0);
            rs_src->SetSrc(0, bufp->val[chl], 1);    //src2
            rs_src->SetSrc(k, bufp->val[chl], 3);    //src2_sw
            memcpy(rs_src->src2_rce, rs_src->src2, sizeof(int32_t)*3);
            if (bup->t1st%600==0) { //10 minutes
                rs_src->resync = 1;
                resmp_src_[i].stm_sw.tv_sec = bufp->t1st;
                if (k) {    //start from block2
                    resmp_src_[i].stm_sw.tv_usec = (k * 1000 + bufp->num / 2) / bufp->num;
                } else {    //start from block1
                    resmp_src_[i].stm_sw.tv_sec--;
                    resmp_src_[i].stm_sw.tv_usec = ((pnt+cnt-*nmp) * 1000 + bufp->num / 2) / bufp->num;
                }
                resmp_src_[i].stm_sw.tv_usec *= 1000;
            }
            cnt = pnt + cnt - smfrq;
            sw = true;
        }
    }
    if (sw) {
        rs_src->cnt2 = cnt;
        rs_src->cnt2_sw = cnt-k;
        rce_src->cnt2 = cnt-cnt0;
        resvinf[chl].point = cnt;
    } else {
        rs_src->cnt1 = cnt;
        rce_src->cnt1 = cnt;
        resvinf[chl].point = pnt+cnt;
    }
    return rs_src->cnt1 + rs_src->cnt2;
}

/*!
Initialze data source

    Input:  pnt -- start sample point in block
            val -- 1s data block
    Output: sw -- switch block state initial value
            cnt -- count of sample point initial value
    Return: the count of cycle will be completed
*/
int ReSampSource::IniSrc(bool *sw, int *cnt, int pnt, int32_t (*val)[3][12800])
{
    int cycs;
    if (resync) {
        resync = 0;
        memcpy(&stm, &stm_sw, sizeof(timeval));
        
        memcpy(src1, src1_sw, sizeof(int32_t)*3);
        memcpy(src2, src2_sw, sizeof(int32_t)*3);
        cnt1 = cnt1_sw;
        *cnt = cnt2_sw;
        cycs = cnt_cp;
        *sw = true;
    } else {
        for (int i = 0; i < 3; i++) {
            src1[i] = &val[i][pnt];
        }
        memset(src2, 0, sizeof(int32_t)*3);
        cnt1 = *cntcnt2 = 0;
        *cnt = 0
        cycs = 10;
        *sw = false;
    }
    cnt2 = 0;
    memset(src1_sw, 0, sizeof(int32_t)*3);
    cnt1_sw = 0;
    memset(src2_sw, 0, sizeof(int32_t)*3);
    cnt2_sw = 0;
    valid = 0;
    return cycs;
}

/*!
Set data source

    Input:  pnt -- start sample point in block
            val -- 1s data block
            type -- 0=src1, 1=src2, 2=src1_sw, 3=src2_sw
*/
void ReSampSource::SetSrc(int pnt, int32_t (*val)[3][12800], int type)
{
    int32_t *src;
    switch (type) {
        case 0: src = src1; break;
        case 1: src = src2; break;
        case 2: src = src1_sw; break;
        case 3: src = src2_sw; break;
        default: return;
    }
    for (int i=0; i<3; i++) {
        src[i] = &val[i][pnt];
    }
}

/*!
Initialze data source

    Input:  sw -- switch block state. 0=no, 1=yes
            cycs -- total count of cycle will be used rms calculate
            pnt -- start sample point in block
            val -- 1s data block
    Output: sw -- switch block state initial value
            cnt -- count of sample point initial value
    Return: the count of cycle will be completed
*/
int RceSource::IniSrc(bool sw, int cycs, int pnt, int32_t (*val)[3][12800])
{
    memset(src1, 0, sizeof(int32_t)*3);
    memset(src2, 0, sizeof(int32_t)*3);
    cnt1 = cnt2 = 0;
    cnt_cyc = cycs;
    if (sw) {
        for (int i = 0; i < 3; i++) {
            src2[i] = &val[i][pnt];
        }
    } else {
        for (int i = 0; i < 3; i++) {
            src1[i] = &val[i][pnt];
        }
    }
}

/*!
Calculate rapid change event rms.

    Input:  chl -- channel index
            rce_src -- rce_source_
*/
void SvProcess::CalcRceRms(int chl, RceSource* rce_src)
{
    //Merge 2 sector into 1 sector
    int cnt1 = rce_src->cnt1;
    int cnt2 = rce_src->cnt2;
    int scnt = cnt1 + cnt2; //number of source data
    if (scnt <= 0) return;
    float *rms = new float[scnt/kRvcIntrvl+1];
    if (!rms) {
        printf("system error! %s %s\n", __FILE__, __FUNCTION__);
        return;
    }

    int n;
    //Calculate rms
    for (int i=0; i<3; i++) {
        int32_t *src = new int[scnt];
        if (!src) {
            printf("system error! %s %s\n", __FILE__, __FUNCTION__);
            return;
        }
        int32_t *src1 = rce_src->src1[i];
        int32_t *src2 = rce_src->src2[i];
        if (src1) memcpy(src, src1, cnt1 * sizeof(int));
        if (src2) memcpy(&src[cnt1], src2, cnt2 * sizeof(int));

        n = 0;
        int pos = 0, pnts;
        for (int j=0; j<rce_src->cnt_cyc; j++) {
            pnts = rce_src->cnt_pnt[j];
            for (int k=0; k<pnts; k++) {
                fi = src[pos];
                fi *= fi;
                v1pn2_[chl][i]->Push(&fi);
                sum_v1pn2_[chl][i] += fi;
                pos++;
                if (pos%kRvcIntrvl==0) {
                    while (v1pn2_[chl][i]->data_num() > pnts) {
                        v1pn2_[chl][i]->Pop(&fi);
                        sum_v1pn2_[chl][i] -= fi;
                    }
                    fi = sum_v1pn2_[chl][i]/pnts;
                    rms[n] = fi;
                    n++;
                }
            }
        }
        if (loop_cnt_==7) RecalcSumV1pn2(chl, i, pnts);
        one_chnnl_[chl]->HandleRce(rms, n, src, scnt, stm, i);
    }
    delete [] rms;
}

/*!
Recalculate sum_v1pn2, to eliminate sum error result from floatpoint error

    Input:  chl -- channel
            phs -- phase
            pnts -- count of sampling point current cycle
*/
void SvProcess::RecalcSumV1pn2(int chl, int phs, int pnts)
{
    int m = v1pn2_[chl][phs]->data_num();
    int n = m<pnts?m:pnts;
    sum_v1pn2_[chl][phs] = 0;
    v1pn2_[chl][phs]->Seek(0);
    float fi;
    for (int i=0; i<n; i++) {
        m = v1pn2_[chl][phs]->Read(&fi);
        if (!m) sum_v1pnt2_[chl][phs] += fi;
    }
}
