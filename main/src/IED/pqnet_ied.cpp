/*! \file smpl_val.cpp
    \brief sample value pre-process.
*/

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <sys/time.h>
#include "sv_shmem.h"
#include "spi_dev.h"
#include "time_cst.h"
#include "shmem_sv_61850.h"

using namespace std;

PqnetIED & pqnet_ied()
{
	static PqnetIED ied;
	return ied;
}

PqnetIED::PqnetIED(const char *dev)
{
    smplv_buf_ = new LoopBufSV<SV_1sBlock>(12);
    fluct_buf_ = new LoopBufSV<FluctBuf>(60);
    resv_buf_ = new LoopBufSV<ReSmplBuf>(60);
    for (int i = 0; i < kChannelTol; i++) {
        cycs_pnt_[i].sv_buf = new LoopPointer<SV_1sBlock>(12);
        cycs_pnt_[i].point = 0;
        resv_inf_[i].sv_buf = new LoopPointer<SV_1sBlock>(12);
        resv_inf_[i].point = 0;
        cycs_inf_[i] = new LoopBufSV<int>(800);
    }
    spi_dev_ = new SpiDev("/dev/spidev2.0");
    spi_api_ = new SpiApi();
    param_cfg = new ParamCfg;
    param_cfg->ReadPhyParam(&phy_dev());
    IniLDChnnl();
    sv_process_ = new SvProcess(one_channel_);
    memset(fft_rbuf_, 0, sizeof(fft_rbuf_));
    memset(pst_rbuf_, 0, sizeof(pst_rbuf_));
    matchw_delay_ = 0;
}

PqnetIED::~PqnetIED()
{
    delete sv_process_;
    delete spi_api_;
    delete spi_dev_;
    for (int i = 0; i < kChannelTol; i++) {
        delete one_channel_[i];
        delete logic_dev_[i];
        delete cycs_inf_[i];
        delete resv_inf_[i].sv_buf;
        delete cycs_pnt_[i].sv_buf;
    }
    delete resv_buf_;
    delete fluct_buf_;
    delete smplv_buf_;
}

/*!
Initialze Logical Device and channel
*/
void PqnetIED::IniLDChnnl()
{
    LDChnnlInfo ch_inf[kChannelTol];
    phy_dev().ldchnl_inf(ch_inf);
    for (int i = 0; i < kChannelTol; i++) {
        logic_dev_[i] = new LogicDev(i, one_channel_);
        logic_dev_[i]->set_channel(&ch_inf[i]);
        if (logic_dev_[i]->stat()) {
            param_cfg->ReadLDParam(logic_dev_[i], i+1);
        }
        one_channel_[i] = new OneChannel();
    }
    ChnnlAttr ch_attr[kChannelTol];
    DeriveChnlAttr(ch_attr, ch_inf);
    for (int i = 0; i < kChannelTol; i++) {
        ch_attr[i].stts_spc = phy_dev().stts_spc();
        one_channel_[i]->set_chnl_attr(&ch_attr[i]);
        param_cfg->ReadChnlParam(one_channel_[i], i+1);
        one_channel_[i]->set_frqmspc(phy_dev().freq_measpc());
    }
}

/*!
Derive onechannel attribute from LD channel information

    Input:  chinf -- LD channel information
    Output: attr -- One channel attribute
*/
void PqnetIED::DeriveChnlAttr(ChnnlAttr* attr, LDChnnlInfo *chinf)
{
    memset(attr, 0, sizeof(ChnnlAttr)*kChannelTol);
    for (int i = 0; i < kChannelTol; i++) { //voltage channel
        if (!chinf[i].stat) continue;
        int j = chinf[i].idx(0);
        if (j) {
            attr[j].type = 1;
            attr[j].mode = 1;
        }
    }
    
    for (int i = 0; i < kChannelTol; i++) { //current channel
        if (!chinf[i].stat) continue;
        int j = chinf[i].idx(0);
        int k = chinf[i].idx(1);
        if (k) {
            attr[k].type = 2;
            if (j) {    //have voltage channel
                attr[j].slave_idx[attr[j].slaves] = k;
                attr[j].slaves++;
            } else {    //haven't voltage channel. current channel only
                attr[k].mode = 1;
            }
        }
    }
}

/*!
Read sample value and preprocess

    Input:  wait-- wait time. unit:ms
*/
void PqnetIED::ReadSV(int wait)
{
    //Read 1s block of sv data from share memory
    SV_1sBlock *pbuf = shmem_sv().ReadSv1sBlock();
    if (pbuf == NULL) {
        msSleep(wait);
        return;
    } else {
        SV_1sBlock *bp = smplv_buf_->PushP();
        memcpy(bp, pbuf, sizeof(SV_1sBlock));
        shmem_sv().set_rdata_cnt();
        for (int i = 0; i < kChannelTol; i++) {
            one_channel_[i]->set_smp_frq(bp->num);
            cycs_pnt_[i].sv_buf_->Push(bp);
            resv_inf_[i].sv_buf_->Push(bp);
            if (one_channel_[i]->chl_type() != bp->type[i]) {
                one_channel_[i]->set_chl_match(1);
                one_channel_[i]->set_detect_type(bp->type[i]);
            } else {
                one_channel_[i]->set_chl_match(0);
            }
        }
        ChlMatchWarn();
        sv_process_->FluctSV(fluct_buf_, bp);
    }

    //process 1-2s sample data
    for (int i = 1; i <= kChannelTol; i++) {
        if (!sv_process_->DetectCycs(i, cycs_inf_[i], &cycs_pnt_[i])) break;
    }
    while(1) {
        if (!sv_process_->ReSample(resv_buf_, resv_inf_, cycs_inf_)) break;
        HandleRce();
    }
}

/*!
Handle Rapid Change Event

        Variable: logic_dev_, one_channel_
*/
void PqnetIED::HandleRce()
{
    for (int i=0; i<kChannelTol; i++) {
        if (logic_dev_[i]->stat()) {
            logic_dev_[i]->HandleRce();
        }
    }
    for (;;) {
        int hmd = 0;    //have more data

        int32_t *sv[kChannelTol][3];
        RceParam par[kChannelTol][3];
        memset(sv, 0, sizeof(sv));
        memset(par, 0, sizeof(par));
        for (int i=0; i<kChannelTol; i++) {
            if (logic_dev_[i]->trigger_cause()) {
                int k;
                for (int vc=0; vc<2; vc++) {
                    k = logic_dev_[i]->chnl_idx(vc);
                    if (k) {
                        k--;
                        for (int j=0; j<3; j++) {
                            if (!sv[k][j]) {
                                sv[k][j] = one_channel_[k]->sce_sv(j);
                                par[k][j] = one_channel_[k]->sce_par(j);
                            }
                        }
                        if (sv[k][0]) {
                            logic_dev_[i]->SetRceData(vc, sv[k], par[k]);
                            hmd = 1;
                        }
                    }
                }
            }
        }
        if (!hmd) break;
    }
}

/*!
channel type match warning
*/
void PqnetIED::ChlMatchWarn()
{
    int match = 0;
    for (int i = 0; i < kChannelTol; i++) {
        if (one_channel_[i]->chl_match()) {
            match = 1;
            break;
        }
    }
    if (match) {
        if (matchw_delay_==0) {
            uint8_t type[kChannelTol];
            for (int i = 0; i < kChannelTol; i++) {
                type[i] = one_channel_[i]->detect_type();
            }
            messageq_guis().Push(kCmdChlMatchWarn, sizeof(type), type);
            matchw_delay_ = 3;
        }
        matchw_delay--;
    } else {
        matchw_delay_ = 0;
    }
}

/*!
    Input:  fft_tbuf -- sample data used by fft
            pst_tbuf -- sample data used by pst
    Output: fft_rbuf -- fft result data
            pst_rbuf -- pst result data
    Return: '<=0'=failure, '>0'=success
*/
int PqnetIED::CallFPGA(void *fft_rbuf, void *fft_tbuf, void *pst_rbuf, void *pst_tbuf)
{
    int num=0;
    if (fft_tbuf) {
        num = spi_api_.Transfer(spi_dev_, 3, fft_tbuf, fft_rbuf, kHrmSmpNum*3*4);
        if (num<=0) {
            return num;
        }
    }
    if (pst_tbuf) {
        num = spi_api_.Transfer(spi_dev_, 5, pst_tbuf, pst_rbuf, kPstSmpNum*3*4);
        if (num<=0) {
            return num;
        }
    }
    return num;
}

/*!
    Input:  wait-- wait time. unit:ms
*/
void PqnetIED::HandleSV(int wait)
{
    ResmplBuf * rsmp;
    FluctBuf * flct;
    int *fft_tbuf, *pst_tbuf;
    for (;;) {
        rsmp = resv_buf_->Pop();
        flct = fluct_buf_->Pop();

        if (rsmp) fft_tbuf = &rsmp->val[0][0][0];
        else fft_tbuf = NULL;
        if (flct) pst_tbuf = &flct->val[0][0][0];
        else pst_tbuf = NULL;
        
        int num = CallFPGA(fft_rbuf_, fft_tbuf, pst_rbuf_, pst_tbuf);
        if (rsmp) {
            for (int i=0; i<kChannelTol; i++) {
                one_channel_[i]->MeasureData3s(rsmp->val[i], kHrmSmpNum, &rsmp->t1st[i],
                                               fft_rbuf_[i], 640);
                //one_channel_[i]->MeasureRms(rsmp->val[i], kHrmSmpNum, &rsmp->t1st[i]);
                //one_channel_[i]->PostFft(fft_rbuf_[i], 640);
            }
            for (int i=0; i<kChannelTol; i++) {
                if (logic_dev_[i]->stat()) {
                    logic_dev_[i]->RefreshData();
                }
            }
        }
        if (num<=0) {
            if (wait) msSleep(wait);
            break;
        }
        wait = 0;
    }
}

/*!
Refresh SOE data to share memory
*/
void PqnetIED::Soe2Shm()
{
    for (int i=0; i<kChannelTol; i++) {
        if (logic_dev_[i]->stat()) {
            logic_dev_[i]->Soe2Shm();
        }
    }
}

/*!
Manual trigger record wave

    Input:  ld -- LD index. 0=all, 1=one_channel_[0], 2=one_channel_[1]...
            type -- 0=stop, 1=mannaul start, other=steady start
    Return: 0=success, -1=have event happening, -2=state not change.
*/
int PqnetIED::ManualTrigger(int ld, int type)
{
    int ret = 0;
    if (ld==0) {
        for (int i=0; i<kChannelTol; i++) {
            if (logic_dev_[i]->stat()) {
                ret = logic_dev_[i]->ManualTrigger(type);
            }
        }
    } else {
        if (!logic_dev_[ld-1]->stat()) return -2;
        ret = logic_dev_[ld-1]->ManualTrigger(type);
    }
    return ret;
}


