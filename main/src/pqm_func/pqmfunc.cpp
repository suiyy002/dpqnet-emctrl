#include<cmath>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include <fcntl.h>
#include <unistd.h>

#include "pqmfunc.h"
#include "harmfunc.h"
#include "other_func.h"
#include "../include/flick.h"
#include "../device/device.h"
#include "../GUI/view.h"
#include "../base_func/utility.h"
#include "../IPC/shmemfunc.h"
#include "volt_variation.h"
//#include "../EEW/ee_warning.h"
//#include "../GUI/form/mainform.h"

using namespace std;

static const int AutoAdjustTimes = 10;//自动校准的次数
static const int GetDCValWaitMax = 50;//获取直流分量的最大等待次数

Cpqm_func *pqmfunc;

int HRM_MAX_NUM_CUR = 50;
int HRM_INTER = 1;

Cpqm_func::Cpqm_func()
{
    syspara = prmcfg->syspara();
    debug_para = prmcfg->debug_para();

    stdy_z_buf_ = new Steady_Z_Data;
    memset((unsigned char*)stdy_z_buf_->interhui, 0, 6 * INTERH_SIZE * (INTERH_MAX_RESO - 1) * 2);
    stdy_z_buf_->hmvalid = 0;
    memset(stdy_z_buf_->frequency, 0, sizeof(stdy_z_buf_->frequency));
    fluct_sv_buf = new unsigned short[StSTEP / 80 * 2 * 3];
    alarm_word_ = 0;

    auto_adj = 0;

    add_listener_prd_type(Cpqm_func::lstner_prd_type);

    harmfunc->SetUnits(GetReservPoint(0));
    last_10min_ = next_10min_ = 0;
    //printf("call read_time()\n");
    read_time();
    save_cyc10_ = false;
    memset(&hrm10_buf_, 0, sizeof(hrm10_buf_));
    sync_time_en_ = true;
    real_quality_ = 0;
}

Cpqm_func::~Cpqm_func()
{
    delete [] fluct_sv_buf;
    delete stdy_z_buf_;
}

/*!
Description:  get address of real data buffer

    Input:        type -- 0=A phase voltage;1=B phase voltage;...
                          3=A phase current;...5=C phase current
    Output:       num -- real data number
    Return:       address of real data buffer
*/
short * Cpqm_func::get_real_pt(int type, int &num)
{
    num = stdy_z_buf_->real_data_size / 2;

    if (type > 2) {
        type %= 3;
        return (short*)stdy_z_buf_->smpi[type];
    } else {
        return (short*)stdy_z_buf_->smpu[type];
    }
}
//-------------------------------------------------------------------------

/*!
Description: Get point of reserved data in daram

    Input:  type -- 0=Head reserve data; 1=Tail reserve data
    Output: num -- data size in byte
    Return: start address of data
*/
short * Cpqm_func::GetReservPoint(int type, int * num)
{
    if (type) {
        if (num != NULL) *num = TRSRV_SIZE;
        return (short*)stdy_z_buf_->treserve;
    } else {
        if (num != NULL) *num = HRSRV_SIZE;
        return (short*)stdy_z_buf_->hreserve;
    }
}

/*!
谐波数据相位调零
*/
void Cpqm_func::adjust_phs_zero(unsigned short phs)
{
    unsigned short *pu, *pi;
    int i, j;
    unsigned short zero_angle[MAX_HARM_NUM]; //零输入时的相角

    for (i = 0; i < MAX_HARM_NUM; i++) {
        zero_angle[i] = 2700; //对于cos, 相当于sin的0度
    }

    unsigned long lphs = phs;
    for (j = 0; j < 3; j++) {
        pu = stdy_z_buf_->huph[j];
        pi = stdy_z_buf_->hiph[j];
        if (stdy_z_buf_->huam[j][1]) { //电压基波不为0
            for (i = 0; i < 26; i++) {
                *pu += (lphs * i) % 3600;
                if (*pu >= 3600) *pu -= 3600;
                pu++;
            }
        } else { //电压基波为0,相位归0
            memcpy(stdy_z_buf_->huph[j], zero_angle, 52);
        }
        if (stdy_z_buf_->hiam[j][1]) { //电流基波不为0
            for (i = 0; i < 26; i++) {
                *pi += (lphs * i) % 3600;
                if (*pi >= 3600) *pi -= 3600;
                pi++;
            }
            //2006-10-24
            if (stdy_z_buf_->hiph[j][1] < CUR_PHS_MODIFY) { //此句很必要，否则相位有可能为负。2006-10-18
                stdy_z_buf_->hiph[j][1] += 3600;
            }
            stdy_z_buf_->hiph[j][1] -= CUR_PHS_MODIFY; //修正电流基波相位
            //debug
            //stdy_z_buf_->hiph[j][1] += 2700;
            //stdy_z_buf_->hiph[j][1] %= 3600;

        } else { //电流基波为0,相位归0
            memcpy(stdy_z_buf_->hiph[j], zero_angle, 52);
        }
    }
    if (harm_valid & 0x1) { //26~50次谐波有效
        for (j = 0; j < 3; j++) {
            pu = stdy_z_buf_->huph26[j];
            pi = stdy_z_buf_->hiph26[j];
            if (stdy_z_buf_->huam[j][1]) { //电压基波不为0
                for (i = 0; i < 25; i++) {
                    *pu += (lphs * (i + 26)) % 3600;
                    if (*pu >= 3600) *pu -= 3600;
                    pu++;
                }
            } else { //电压基波为0,相位归0
                memcpy(&stdy_z_buf_->huph26[j][0], zero_angle, 50);
            }
            if (stdy_z_buf_->hiam[j][1]) { //电流基波不为0
                for (i = 0; i < 25; i++) {
                    *pi += (lphs * (i + 26)) % 3600;
                    if (*pi >= 3600) *pi -= 3600;
                    pi++;
                }
            } else { //电流基波为0,相位归0
                memcpy(&stdy_z_buf_->hiph26[j][0], zero_angle, 50);
            }
        }
    }
    if (harm_valid & 0x2 && syspara->inter_enable) { //间谐波数据有效&&间谐波功能开启
        pu = (unsigned short*)stdy_z_buf_->interhui;
        pu ++;
        for (i = 0; i < 6 * 51 * 15; i++) {
            j = i % 357;
            *pu += (lphs * (j / 6)) % 3600;
            if (*pu >= 3600) *pu -= 3600;
            pu += 2;
        }
    }
    
    int hrm10cy = prmcfg->harmnm10cyc()/10;
    for (j = 0; j < 3; j++) {
        pu = stdy_z_buf_->harm_10cyc[j];
        pu++;
        if (stdy_z_buf_->huam[j][1]) { //电压基波不为0
            for (i = 0; i < 15; i++) {
                *pu += (lphs * hrm10cy) % 3600;
                *pu += 900; //调整为sin
                if (*pu >= 3600) *pu -= 3600;
                pu += 2;
            }
        } else { //电压基波为0,相位归0
            for (i = 0; i < 15; i++) {
                *pu = 0;
                pu += 2;
            }
        }
        pi = stdy_z_buf_->harm_10cyc[j+3];
        pi++;
        if (stdy_z_buf_->hiam[j][1]) { //电流基波不为0
            for (i = 0; i < 15; i++) {
                *pi += (lphs * hrm10cy) % 3600;
                *pi += 900; //调整为sin
                if (*pi >= 3600) *pi -= 3600;
                pi += 2;
            }
        } else { //电流基波为0,相位归0
            for (i = 0; i < 15; i++) {
                *pi = 0;
                pi += 2;
            }
        }
    }
}

/*!
Handle harmonic data

    Called by:  hdl_sample
*/
void Cpqm_func::harm_hdl()
{
    static unsigned int iorder = 0;
    unsigned short phs_offset = (6300 - stdy_z_buf_->huph[0][1]) % 3600; //相位偏移量，用于相位调零
    harm_refresh_mon++;
    //6300=3600+2700
    harm_valid = stdy_z_buf_->hmvalid;
    stdy_z_buf_->hmvalid = 0;
    if (!(HRM_MAX_NUM_CUR & 0x20)) clrbit(harm_valid, 0);
    if (!HRM_INTER)    clrbit(harm_valid, 1);
    save_func->set_harm_valid(harm_valid);

    if (!(debug_para->debug_enable & 0x1)) { //不处于调试状态
        adjust_phs_zero(phs_offset);    //谐波数据相位调零
    }
    Harm10cycHdl();
    hrm_time_ = hrm10_buf_.time[14];
    harmfunc->SetRecTime(&hrm_time_);
    if (prmcfg->aggrgt_time_en()) SynTimestamp(&hrm_time_);

    harmfunc->SetUnits(GetReservPoint(0));

    //向对象harmfunc中写入谐波幅值
    harmfunc->write_hamp((unsigned short*)stdy_z_buf_->huam, (unsigned short*)stdy_z_buf_->hiam);
    //向对象harmfunc中写入谐波相位
    harmfunc->write_hphs((unsigned short*)stdy_z_buf_->huph, (unsigned short*)stdy_z_buf_->hiph);
    //向对象harmfunc中写入扩展谐波幅值
    harmfunc->write_exthamp((unsigned short*)stdy_z_buf_->huam26, (unsigned short*)stdy_z_buf_->hiam26, harm_valid);
    //向对象harmfunc中写入扩展谐波相位
    harmfunc->write_exthphs((unsigned short*)stdy_z_buf_->huph26, (unsigned short*)stdy_z_buf_->hiph26, harm_valid);

    //向对象harmfunc中写入间谐波幅值相位
    if (harm_valid & 0x2 && syspara->inter_enable) {
        harmfunc->set_intrh_reso_id(1); //(stdy_z_buf_->interh_reso); // 默认间谐波为 1/10
        harmfunc->write_interhamp((unsigned short*)stdy_z_buf_->interhui);
        //printf("interhui[0]=%d %d %d %d\n", stdy_z_buf_->interhui[0][0][0], stdy_z_buf_->interhui[0][0][1]
        //                        , stdy_z_buf_->interhui[0][0][2], stdy_z_buf_->interhui[0][0][3]);
        
    }
    harmfunc->CalcDerivedData();
    harmfunc->AlarmCheck(alarm_word_);

    other_func->SetRms((unsigned short*)&stdy_z_buf_->rms_u, (unsigned short*)&stdy_z_buf_->rms_i);
    other_func->SetUnbalance((unsigned short *)&stdy_z_buf_->imbal_u, (unsigned short *)&stdy_z_buf_->imbal_i);
    other_func->SetFluct();
    other_func->AlarmCheck(alarm_word_, hrm_time_.tv_sec);
    set_alarm(0, alarm_word_);     //Control relay for switch out
    WriteReal2Shm();
    read_switch_in();   //Read switch input on or off
    if (alarm_word_ && prmcfg->trigger_enable(0)) { //稳态事件触发录波
        ScanStdyEvent(alarm_word_);
        volt_variation->ManualTrigger(37);   //触发手动录波
    }
    if (!alarm_word_ || !prmcfg->trigger_enable(0)|| !volt_variation->stdy_event()) {
        if (volt_variation->manual_state()==37) volt_variation->ManualTrigger(0);  //关闭手动录波
    }
    auto_adjust();  //精度自动校准
    get_sample_dc_val();    //获取采样电路特征直流分量
}

void Cpqm_func::WriteReal2Shm()
{
    PqmMmxu mmxu_real;
    PqmMsqi msqi_real;
    PqmMhai mhai_real;
    PqmMhaiIntr mhai_in_real;

    memset(&mmxu_real, 0, sizeof(PqmMmxu));
    memset(&msqi_real, 0, sizeof(PqmMsqi));
    memset(&mhai_real, 0, sizeof(PqmMhai));
    memset(&mhai_in_real, 0, sizeof(PqmMhaiIntr));
    
    harmfunc->GetShmReal(&mmxu_real, &mhai_real, &mhai_in_real);
    other_func->GetShmReal(&mmxu_real, &msqi_real);
    mmxu_real.time = mmxu_real.hz_time = hrm_time_.tv_sec;
    msqi_real.time = hrm_time_.tv_sec;
    mhai_real.time = hrm_time_.tv_sec;
    mhai_in_real.time = hrm_time_.tv_sec;
    
    shmem_func().ShmemCpy(kMmxuReal, &mmxu_real);
    shmem_func().ShmemCpy(kMsqiReal, &msqi_real);
    shmem_func().ShmemCpy(kMhaiReal, &mhai_real);
    shmem_func().ShmemCpy(kMhaiInReal, &mhai_in_real);
    WriteQuality2Shm();
    shmem_func().IncDataUp(0);
}

void Cpqm_func::WriteQuality2Shm()
{
    unsigned short q = 0;
    if (volt_variation->GetEventMark(kRealMark)) q = Q_Questionable|Q_Inaccurate;
    real_quality_ = q;
    shmem_func().SetQuality(kMmxuReal, q, 0, 0);
    shmem_func().SetQuality(kMmxuReal, q, 0, 1);
    shmem_func().SetQuality(kMsqiReal, q);
    shmem_func().SetQuality(kMhaiReal, q);
    shmem_func().SetQuality(kMhaiInReal, q);
}

//-------------------------------------------------------------------------
//Start adjust sample precision automaticly
void Cpqm_func::start_ato_adj()
{
    auto_adj = 0xb64b;
    m_progress_cnt = AutoAdjustTimes;
    for (int i = 0; i < 3; i++) {
        ato_adj_utol[i] = 0;
        ato_adj_itol[i] = 0;
    }
}
//-------------------------------------------------------------------------

//Read adjust automaticly is run or stop
int Cpqm_func::get_adj_stat(int *cnt)
{
    *cnt = m_progress_cnt;
    if (!auto_adj) {
        return 0;
    } else {
        return 1;
    }
}
//-------------------------------------------------------------------------

//Cancel adjust sample precision automaticly
void Cpqm_func::cancel_ato_adj()
{
    auto_adj = 0;
}

//Adjust sample precision automaticly
void Cpqm_func::auto_adjust()
{
    int i;
    float fi, fj;

    if (auto_adj == 0xb64b) {
        if (prmcfg->sys_para_sg()->vc_adj_enable & 0x1) { //电压自动校准使能
            for (i = 0; i < 3; i++) {
                if (prmcfg->sys_para_sg()->phs_adj_enable & (0x1 << i)) {
                    ato_adj_utol[i] += harmfunc->u_rms(2, i);
                }
            }
        }
        if (prmcfg->sys_para_sg()->vc_adj_enable & 0x2) { //电流自动校准使能
            for (i = 0; i < 3; i++) {
                if (prmcfg->sys_para_sg()->phs_adj_enable & (0x1 << i)) {
                    ato_adj_itol[i] += harmfunc->i_rms(2, i);
                }
            }
        }
        m_progress_cnt--;
        if (m_progress_cnt <= 0) { //自动校准结束
            if (prmcfg->sys_para_sg()->vc_adj_enable & 0x1) { //电压自动校准使能
                for (i = 0; i < 3; i++) {
                    if (prmcfg->sys_para_sg()->phs_adj_enable & (0x1 << i)) {
                        fi = ato_adj_utol[i] / AutoAdjustTimes;
                        fi *= 100;
                        fj = prmcfg->sys_para_sg()->v_datum * prmcfg->get_mdfy_para(0, i);//*syspara->v_res_ratio[i];
                        prmcfg->set_mdfy_para(0, fj / fi, i);
                    }
                }
            }
            if (prmcfg->sys_para_sg()->vc_adj_enable & 0x2) { //电流自动校准使能
                for (i = 0; i < 3; i++) {
                    if (prmcfg->sys_para_sg()->phs_adj_enable & (0x1 << i)) {
                        fi = ato_adj_itol[i] / AutoAdjustTimes;
                        fi *= 100;
                        fj = prmcfg->sys_para_sg()->c_datum * prmcfg->get_mdfy_para(1, i);//*syspara->c_res_ratio[i];
                        prmcfg->set_mdfy_para(1, fj / fi, i);
                    }
                }
            }
            auto_adj = 0;
        }
    }
}

//Start get sample circuit character dc value
void Cpqm_func::start_get_dc_val()
{
    m_get_dc_val = 0xb64b;
    m_progress_cnt = GetDCValWaitMax;
    prmcfg->set_update(DaramFlagUpdate);
    prmcfg->set_daram_sign(StartGetDCVal);
}

//Description: 得到"获取直流分量过程"的当前状态
//Output: cnt,当前进程计数
//Return: 0=完成，1=正在进行，2=超时
int Cpqm_func::get_dcval_stat(int *cnt)
{
    *cnt = m_progress_cnt;
    if (!m_get_dc_val) {
        return 0;
    } else if (m_get_dc_val == 0xb64b) {
        return 1;
    } else {
        return 2;
    }
}

//Cancel "get sample circuit character dc value"
void Cpqm_func::cancel_dc_val()
{
    m_get_dc_val = 0;
}

//Description: 获取采样电路特征直流分量
void Cpqm_func::get_sample_dc_val()
{
    int i;
    float fi, fj;
    unsigned short *phead;

    if (m_get_dc_val == 0xb64b) {
        m_progress_cnt--;
        if (m_progress_cnt <= GetDCValWaitMax - 4) { //延迟4×3秒后开始侦测采样板的"直流流分量测算完成"通知
            phead = stdy_z_buf_->hreserve;
            if (phead[0x66] == 0x465) {
                for (i = 0; i < 6; i++) {
                    prmcfg->sys_para_sg()->character_dc[i] = phead[0x60 + i];
                }
                m_get_dc_val = 0;
            } else if (m_progress_cnt <= 0) {
                m_get_dc_val = 1;
            }
        }
    }
}

//Handle fluctuation data
void Cpqm_func::fluct_hdl()
{
    //if (!prmcfg->pst_enable()) return;

    memcpy(fluct_sv_buf, &stdy_z_buf_->flct[0][0], StSTEP / 80 * 4);    //StSTEP/80=32
    memcpy((char *)fluct_sv_buf + StSTEP / 80 * 4, &stdy_z_buf_->flct[1][0], StSTEP / 80 * 4);
    memcpy((char *)fluct_sv_buf + StSTEP / 80 * 4 * 2, &stdy_z_buf_->flct[2][0], StSTEP / 80 * 4);
    other_func->SetPst((float*)fluct_sv_buf);
}


// Description: read real data
// Input: null;
// Output: bufp - 实时数据缓存;
//     sz - 读取实时数据的大小(byte)
// Return: null;
void Cpqm_func::read_real(char **bufp, int &sz)
{
    int i, j, n;
    unsigned short hrm_buf[HrmRcdSz / 2];
    unsigned short hrmext_buf[HrmextRcdSz / 2];
    int hrm_bsz = 640; //基本谐波占用的字节数
    int have_ext;

    have_ext = 0;
    int intsz = 0;

    //---------- 读取扩展谐波数据 -------------------------------------------------
    if (harm_valid & 0x1) { //扩展谐波有效
        for (j = 0; j < 3; j++) {
            for (i = 0; i < 25; i++) {
                n = j * 25 * 4 + i * 4;
                hrmext_buf[n] = stdy_z_buf_->huam26[j][i];
                hrmext_buf[n + 1] = stdy_z_buf_->huph26[j][i];
                hrmext_buf[n + 2] = stdy_z_buf_->hiam26[j][i];
                hrmext_buf[n + 3] = stdy_z_buf_->hiph26[j][i];
            }
        }
        have_ext = 1;
    }
    //---------- 读取间谐波数据 ---------------------------------------------------
    unsigned char * intbuf;
    if ((harm_valid & 0x2) && syspara->inter_enable) { //间谐波有效
        //stdy_z_buf_->interhui[0][50][0] = 1234; 
        intbuf = harmfunc->make_inter_struct(intsz, (unsigned short*)stdy_z_buf_->interhui);
    }
    //---------- 读取0~25次谐波 ---------------------------------------------------
    for (j = 0; j < 3; j++) {
        for (i = 0; i < 26; i++) {
            n = j * 26 * 4 + i * 4;
            hrm_buf[n] = stdy_z_buf_->huam[j][i];
            hrm_buf[n + 1] = stdy_z_buf_->huph[j][i];
            hrm_buf[n + 2] = stdy_z_buf_->hiam[j][i];
            hrm_buf[n + 3] = stdy_z_buf_->hiph[j][i];
        }
    }
    n += 4;
    memcpy(&hrm_buf[n], harmfunc->refresh_time(), sizeof(time_t));

    hrm_buf[n + 3] = other_func->frequency(0);
    hrm_buf[n + 4] = real_quality_;

    hrm_buf[635/2] &= 0xff;
    hrm_buf[635/2] |= (harmfunc->units_type(0) | (harmfunc->units_type(1) << 2) | (harmfunc->units_type(2) << 4))<<8;
//---------- 构造全部数据 -----------------------------------------------------
    sz = hrm_bsz + HrmextRcdSz * have_ext + intsz;
    *bufp = new char[sz];
    memcpy(*bufp, hrm_buf, hrm_bsz);
    if (have_ext) { //有扩展谐波数据
        memcpy(*bufp + hrm_bsz, hrmext_buf, HrmextRcdSz);
        (*bufp)[638] = 0x66;
    } else {
        (*bufp)[638] = 0;
    }
    if (intsz) { //有间谐波数据
        memcpy(*bufp + hrm_bsz + HrmextRcdSz * have_ext, intbuf, intsz);
        (*bufp)[637] = 0x66;
    } else {
        (*bufp)[637] = 0;
    }
}

#define HARM_ALM 0x01ff
#define WARP_ALM 0x0800
#define FREQ_ALM 0x1000
#define UNBLC_ALM 0x6000
#define Pst_ALM 0x8000

#define HARM_LED 0x0010
#define WARP_LED 0x0020
#define FREQ_LED 0x0040
#define UNBLC_LED 0x0100
#define Pst_LED 0x0200
#define MCTRL_LED 0x0001

//单元报警
void Cpqm_func::indicator_light()
{
    int indicate_type;
    static int hcnt = 0;
    static unsigned int cnti = 0, cntj = 4;
    unsigned short usi;

    if (hcnt == harm_refresh_mon) { //谐波计数未变化
        cnti++;
    } else {
        cnti = 0;
        hcnt = harm_refresh_mon;
    }
    if (cnti >= 16) {
        indicate_type = 1;//谐波数据停止刷新
        cnti = 16;
    } else {
        indicate_type = 0;//谐波数据正常刷新
    }
    if (--cntj <= 0) {
        usi = 0;
        if (alarm_word_ & HARM_ALM) {
            usi |= HARM_LED;
        }
        if (alarm_word_ & WARP_ALM) {
            usi |= WARP_LED;
        }
        if (alarm_word_ & FREQ_ALM) {
            usi |= FREQ_LED;
        }
        if (alarm_word_ & UNBLC_ALM) {
            usi |= UNBLC_LED;
        }
        if (alarm_word_ & Pst_ALM) {
            usi |= Pst_LED;
        }
        if (!indicate_type) {
            usi |= MCTRL_LED;
        }
        refresh_daram(&usi, 2, 2);
        cntj = 4;
    }
}

/*!
Description:handle per 0.5s
*/
void Cpqm_func::HalfSecHandle()
{
    other_func->SetFrequency(stdy_z_buf_->frequency);
    
/*    int idx = hrm10_buf_.head;
    float fi = other_func->frequency(0);
    hrm10_buf_.freq[idx] = fi/100;
    hrm10_buf_.head++;
    if (hrm10_buf_.head>=6) hrm10_buf_.head = 0;
*/
    read_time();
    save_func->SaveHandle();

}

void Cpqm_func::read_time()
{
    now_t_ = time(NULL);
    tm tmi;
    LocalTime(&tmi, &now_t_);
    strftime(time_str_, 20, "%Y-%m-%d %H:%M:%S", &tmi);

    if (now_t_ < last_10min_ || now_t_ >= next_10min_) {
        //printf("now_t_=%d next_10min_=%d\n", now_t_, next_10min_);
        if (now_t_ == next_10min_) { //It's time to restart fft
            //printf("now_t_=%d\n", now_t_);
            if (prmcfg->aggrgt_time_en()) {
                unsigned short usi = 1;
                refresh_daram(&usi, 2, 4);
            }
        }
        save_func->ChgTimeFrame(now_t_, 600, &last_10min_, &next_10min_);  //更改时间片
        //printf("last_10min_=%d, next_10min_=%d\n", last_10min_, next_10min_ );
    }

    //Manual Record wave
	static time_t rcd_start_time;  //Manual record wave start time
    static bool start=false;
    if (prmcfg->manual_rec_enable() && volt_variation->manual_state()==1) {
        if (!start) {
            rcd_start_time = now_t_;
            start = true;
        }
        if (now_t_ - rcd_start_time >= prmcfg->transt_rcd_time()) {
            printf("now_t=%d, rcd_start_time=%d, prmcfg->transt_rcd_time()=%d\n", now_t_, rcd_start_time, prmcfg->transt_rcd_time());
            volt_variation->ManualTrigger(0);   //关闭手动录波
            char chi = 0;
            shmem_func().SetRdre(kRdreRcdTrg, &chi, NULL);
        }
    } else {
        start = false;
    }
}

void Cpqm_func::lstner_prd_type(int new_prd_type)
{
    HRM_MAX_NUM_CUR = calc_harm_num(new_prd_type);
    HRM_INTER = chk_harm_inter(new_prd_type);
    TRANST_VALID = chk_transt_valid(EQUIP_TYPE);
    if (pqmfunc != NULL && !TRANST_VALID) {
        pqmfunc->close_func_transt();
    }
    pqm_view().ChangeFormPara(VW_MAIN, new_prd_type);
}

//关闭暂态功能
void Cpqm_func::close_func_transt(void)
{
    syspara->transt_monitor = 0;
    syspara->transt_tb_enable = 0;
    
    volt_variation->ManualTrigger(0);   //关闭发手动录波
    prmcfg->set_manual_rec_enable(false);
}

#define BATTERY_RANGE 200  //2.0V
#define BATTERY_LOW 630    //6.3V
//Description:  Read battery power energy
//Input:        volt - 电源电压,单位0.01V
int Cpqm_func::read_battery_power(int volt)
{
    short *buf;
    int qnty;

    buf = GetReservPoint(0);
    qnty = buf[0xF2];
    if (qnty == 0 || qnty == 3) {  //充满
        qnty = BATTRY_POWER_FULL;
    } else if (qnty == 1) {  //正在充电
        qnty = BATTRY_CHARGING;
    } else if (qnty == 2) {  //将要充满
        qnty = BATTRY_TRICKLE_CHARGE;
    } else { //计算剩余电量
        float u = volt * qnty / 255 * 2; //对应电池电压
        qnty = ((u - BATTERY_LOW) / BATTERY_RANGE) * ((u - BATTERY_LOW) / BATTERY_RANGE) * 100; //此处用平方因为电量与电压的平方成正比
        if (qnty > 100) qnty = 100;
        if (u < BATTERY_LOW) qnty = 0;
        //printf("u=%3.2f, qnty=%d\n", u, qnty);
    }
#ifdef PQNet2xx_3xx
    qnty = 100;
#endif
    power_qnty_ = qnty;
    return qnty;
}

void Cpqm_func::ScanStdyEvent(int alarm)
{
    volt_variation->set_stdy_event(0);
    if (alarm&0x3f && prmcfg->trigger_enable(2)) volt_variation->set_stdy_event(2);   //Harmonic voltage(THDu&HRu)
    if (alarm&0x1c0 && prmcfg->trigger_enable(3)) volt_variation->set_stdy_event(3);  //Harmonic current
    if (alarm&0xe00 && prmcfg->trigger_enable(4)) volt_variation->set_stdy_event(4);  //Voltage deviation
    if (alarm&0x1000 && prmcfg->trigger_enable(1)) volt_variation->set_stdy_event(1); //Frequency deviation
    if (alarm&0x2000 && prmcfg->trigger_enable(5)) volt_variation->set_stdy_event(5); //Voltage unbalance
    if (alarm&0x4000 && prmcfg->trigger_enable(6)) volt_variation->set_stdy_event(6); //Negative sequence current
}

/*!
harmonic 10 cycle value handle
*/
void Cpqm_func::Harm10cycHdl()
{
    int u = harmfunc->units(0);
    int c = harmfunc->units(1);
    
    //printf("\n");
    for (int i=0; i<15; i++) {
        for (int j=0; j<3; j++) {
            hrm10_buf_.data[i][j*2] = stdy_z_buf_->harm_10cyc[j][i*2];
            hrm10_buf_.data[i][j*2] /= u;
            hrm10_buf_.data[i][j*2+1] = stdy_z_buf_->harm_10cyc[j][i*2+1];
            hrm10_buf_.data[i][j*2+1] /= 10;
        }
        for (int j=3; j<6; j++) {
            hrm10_buf_.data[i][j*2] = stdy_z_buf_->harm_10cyc[j][i*2];
            hrm10_buf_.data[i][j*2] /= c;
            hrm10_buf_.data[i][j*2+1] = stdy_z_buf_->harm_10cyc[j][i*2+1];
            hrm10_buf_.data[i][j*2+1] /= 10;
        }
        for (int j=0; j<3; j++) {
            hrm10_buf_.rms[i][j*2] = (float)unzip_int(stdy_z_buf_->rms_10cyc[i][j*2])/u;
            hrm10_buf_.rms[i][j*2+1] = (float)unzip_int(stdy_z_buf_->rms_10cyc[i][j*2+1])/c;
            //printf("%5.3f %5.3f;",hrm10_buf_.rms[i][j*2], hrm10_buf_.rms[i][j*2+1]);
        }
        
        hrm10_buf_.time[i].tv_sec = stdy_z_buf_->harm_time[i][0];
        hrm10_buf_.time[i].tv_sec <<= 16;
        hrm10_buf_.time[i].tv_sec += stdy_z_buf_->harm_time[i][1];
        hrm10_buf_.time[i].tv_usec = stdy_z_buf_->harm_time[i][2]*100;
    }
#if 0
    int dif = hrm10_buf_.time[14].tv_sec - now_t_;
    int dif2 = hrm10_buf_.time[13].tv_sec - now_t_;
    if (dif>7 || dif2>7) {
        for (int i=0; i<15; i++) {
            printf("%x %x %x\n", stdy_z_buf_->harm_time[i][0], stdy_z_buf_->harm_time[i][1], stdy_z_buf_->harm_time[i][2]);
        }
        tm tmi;
        char str[20];
        LocalTime(&tmi, &hrm10_buf_.time[13].tv_sec);
        strftime(str, 20, "%Y-%m-%d %H:%M:%S", &tmi);
        printf("hrm10_buf_.time[13]=%s.%d\n", str, hrm10_buf_.time[13].tv_usec);
        LocalTime(&tmi, &hrm10_buf_.time[14].tv_sec);
        strftime(str, 20, "%Y-%m-%d %H:%M:%S", &tmi);
        printf("hrm10_buf_.time[14]=%s.%d\n", str, hrm10_buf_.time[14].tv_usec);
    }
#endif

    //printf("%x %x %x\n", stdy_z_buf_->harm_time[14][0], stdy_z_buf_->harm_time[14][1], stdy_z_buf_->harm_time[14][2]);
    //printf("\n");
    if (save_cyc10_||hrm10_buf_.on_off) {
        hrm10_buf_.on_off = save_cyc10_;
        if (save_cyc10_len_++>200) {    //最长记录10分钟
            hrm10_buf_.on_off = save_cyc10_ = false;
        }
        notice_pthread(kTTSave, SAVEINFO, kHarmCyc10, &hrm10_buf_);
    }
}

/*!
Synchronize timestamp of harmonic per 10minutes
*/
void Cpqm_func::SynTimestamp(timeval *time)
{
    tm tmi;
    GmTime(&tmi, &time->tv_sec);
    if (!sync_time_en_) {
        int m = tmi.tm_min % 10;
        if (m==0&&(tmi.tm_sec>2&&tmi.tm_sec<6)) { //synchronize per 10 min
            sync_time_en_=true;
        }
    }
    long us_l, us_r;
    if (sync_time_en_) {
        sync_time_en_ = false;
        us_l = time->tv_usec % 200000;
        us_r = 200000 - us_l;
        //printf("us_l=%d us_r=%d time->tv_usec=%d\n", us_l, us_r, time->tv_usec);
        if (us_r<us_l) {
            offset_us_ = us_r;
        } else {
            offset_us_ = -us_l;
        }
    }
    time->tv_usec += offset_us_;
    if (time->tv_usec<0) {
        time->tv_sec -= 1;
        time->tv_usec += 1000000;
    } else if (time->tv_usec>=1000000) {
        time->tv_sec += 1;
        time->tv_usec -= 1000000;
    }
    //printf("offset_us_=%d time->tv_usec=%d\n", offset_us_, time->tv_usec);
    
}