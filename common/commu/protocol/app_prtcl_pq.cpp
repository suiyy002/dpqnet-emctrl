#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_prtcl_pq.h"
#include "../../pqm_func/pqmfunc.h"
#include "../../thread/pthread_mng.h"
#include "../../GUI/view.h"
#include "../../device/device.h"
#include "../../base_func/conversion.h"
#include "../../EEW/ee_warning.h"
#include "../../base_func/utility.h"
#include "../../security/secur_mng.h"
#include "../../security/user_manage.h"
#include "../../base_func/timer_cstm.h"

AppPrtclPQ::AppPrtclPQ(int hdsz): kDataHdSz(hdsz)
{
    trst_rcd = prmcfg->trst_rcd();
    syspara = prmcfg->syspara();

    f_transtu = f_transti = NULL;
    ptx_bufd = NULL;
    p_freq_rcd_buf = NULL;
    p_imbal_rcd_buf = NULL;
    p_warp_rcd_buf = NULL;

    hrmrcdp = new char*[HarmBufNum];
    memset(hrmrcdp, 0, sizeof(hrmrcdp)*HarmBufNum);
    iwaitcnt = 0;
    logged_ = false;
    user_id_ = 0;
    secur_mng_ = new SecurMng;
    secur_mng_->GenRandKey();
    ps_err_cnt_ = 5;
    timer_lock_ = new TimerCstm;
    timer_logout_ = new TimerCstm;
    timer_setime_ = new TimerCstm;
    timer_inidev_ = new TimerCstm;
}

AppPrtclPQ::~AppPrtclPQ()
{
    delete timer_inidev_;
    delete timer_setime_;
    delete timer_logout_;
    delete timer_lock_;
    if (logged_) usr_mng().SetAudit("login", usr_mng().usr_name(user_id_), "Remote logout", 0);
    delete secur_mng_;
    if (f_transtu) {
        fclose(f_transtu);
        if (f_transti != NULL)  fclose(f_transti);
    }
    if (ptx_bufd) {
        delete [] ptx_bufd;
    }

    char **cppi = hrmrcdp;
    for (int i=0; i<HarmBufNum; i++) {
        if (*cppi != NULL) delete [] *cppi;
        cppi++;
    }
}

//---------------------------------------------------------------------------
// 读取并发送实时数据。
// input:  rbuf: 接收到的数据
//         lastharm:=0 取实时数据 =1 取最后保存的谐波数据
int AppPrtclPQ::tx_real(uchar *rbuf, int lastharm)
{
    char *tbuf;
    int sz;

    if (ptx_bufd != NULL) {
        delete [] ptx_bufd;
        ptx_bufd = NULL;
    }
    pqmfunc->read_real(&tbuf, sz);
    ptx_bufd = new uchar[kDataHdSz+sz];
    memcpy(&ptx_bufd[kDataHdSz], tbuf, sz);
    delete [] tbuf;

    //Equipment type and firmware's version
    ptx_bufd[kDataHdSz+634] = prmcfg->pqm_type();
    //ptx_bufd[kDataHdSz+635] = VER_1st;
    //ptx_bufd[kDataHdSz+636] = VER_2nd;
    ptx_bufd[kDataHdSz+639] = 0;
#if 1
    unsigned short si[4];
    memcpy(&si, &ptx_bufd[kDataHdSz+1040], 8);
    if(si[3]>3600) 
        printf("C26_U_phs_I_phs=%5d %5d %5d %d\n", si[0], si[1], si[2], si[3]);
#endif
    return sz;
}

// 设置或读取参数
int AppPrtclPQ::set_para(uchar *rbuf, unsigned int rsz, int cmd)
{
    unsigned long uli;
    int i, k;
    int retval = 1;
    char stri[32];
    int aryi[4];
    void *pvoid;

    switch (cmd) {
        case CMD_SETDEVNUM: //设置单元编号
            prmcfg->sys_para_sg()->device_num = rbuf[0];
            usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "CMD_SETDEVNUM", 1);
            break;
        case CMD_SAVE_SPACE: //设置数据存储间隔
            if (rbuf[0]) {  //写
                usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "CMD_SAVE_SPACE", 1);
                k = rbuf[1] << 8;
                k += rbuf[2];
                syspara->harm_rcd_space = AdjSaveSpc(k, 0);
                if (rsz >= 9) {
                    k = (rbuf[3] << 8) + rbuf[4];
                    if (syspara->freq_rcd_space != k) {
                        syspara->freq_rcd_space = AdjSaveSpc(k, 1);
                    }
                    k = (rbuf[5] << 8) + rbuf[6];
                    if (syspara->unbalance_rcd_space != k) {
                        syspara->unbalance_rcd_space = AdjSaveSpc(k, 0);
                    }
                    k = (rbuf[7] << 8) + rbuf[8];
                    if (syspara->voltdv_rcd_space != k) {
                        syspara->voltdv_rcd_space = AdjSaveSpc(k, 0);
                    }
                }
            }
            k = syspara->harm_rcd_space;
            rbuf[1] = k >> 8;
            rbuf[2] = k & 0xff;
            k = syspara->freq_rcd_space;
            rbuf[3] = k >> 8;
            rbuf[4] = k & 0xff;
            k = syspara->unbalance_rcd_space;
            rbuf[5] = k >> 8;
            rbuf[6] = k & 0xff;
            k = syspara->voltdv_rcd_space;
            rbuf[7] = k >> 8;
            rbuf[8] = k & 0xff;
            retval = 9;
            break;
        case CMD_SETBAUDRATE: //设置串口波特率
            uli = rbuf[0];
            uli += rbuf[1] << 8;
            uli += rbuf[2] << 16;
            uli += rbuf[3] << 24;
            //printf("rbuf[4]=%d,baudrate=%ld\n", rbuf[4], uli);

            if (rbuf[4] < 2) { //串口号合法
                if (rbuf[4] == 0) { //RS-485
                    prmcfg->sys_para_sg()->BaudRate[rbuf[4]] = uli;
                    prmcfg->set_update(BaudrateUpdate);
                } else if (rbuf[4] == 1) {  //IRIG-B
                    //prmcfg->sys_para_sg()->BaudRate[rbuf[4]] = 1200;
                    //prmcfg->set_update(BaudrateUpdate);
                }
            }
            break;
        case CMD_IP_ADDR: //设置IP相关参数
            if (rbuf[0]) { //写
                if (rbuf[1]<5) {    //IP, Netmask, Gateway
                    sprintf(stri, "%d.%d.%d.%d", rbuf[2], rbuf[3], rbuf[4], rbuf[5]);
                    set_net_para(rbuf[1], stri);
                    apply_net_para(); //seapex 2009-10-10
                    prmcfg->set_update(SocketPortIPUpdate);
                } else if (rbuf[1]==11||rbuf[1]==12) {   //NTP IP&Port
                    switch(rbuf[1]) {
                        case 11:
                            sprintf(stri, "%d.%d.%d.%d", rbuf[2], rbuf[3], rbuf[4], rbuf[5]);
                            break;
                        case 12:
                            sprintf(stri, "%d", (rbuf[2]<<8)+rbuf[3]);
                            break;
                    }
                    set_ntp_para(rbuf[1]-10, stri);
                    apply_ntp_para(); 
                } else if (rbuf[1]==10) { //Port
                    k = (rbuf[2]<<8)+rbuf[3];
                    prmcfg->set_socket_server_port(k);
                    prmcfg->set_update(SocketPortIPUpdate);
                }
                usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "CMD_IP_ADDR", 1);
            } else { //读
                if (rbuf[1]<5) {    //IP, Netmask, Gateway
                    get_net_para(rbuf[1], stri);
                    memset(aryi, 0, sizeof(aryi));
                    sscanf(stri, "%d.%d.%d.%d", &aryi[0], &aryi[1], &aryi[2], &aryi[3]);
                } else if (rbuf[1]==11||rbuf[1]==12) {   //NTP IP&Port
                    get_ntp_para(rbuf[1]-10, stri);
                    memset(aryi, 0, sizeof(aryi));
                    switch (rbuf[1]) {
                        case 11:
                            sscanf(stri, "%d.%d.%d.%d", &aryi[0], &aryi[1], &aryi[2], &aryi[3]);
                            break;
                        case 12:
                            sscanf(stri, "%d", &k);
                            aryi[0] = k>>8;
                            aryi[1] = k&0xff;
                            break;
                    }
                } else if (rbuf[1]==10) {    //Port
                    k = prmcfg->socket_server_port();
                    aryi[0] = k>>8;
                    aryi[1] = k&0xff;
                }
            }
            for (i = 0;i < 4;i++) rbuf[i+2] = aryi[i];
            retval = 6;
            break;
        case CMD_PORT_NUM: //设置端口号
            if (rbuf[0]) { //写
                k = rbuf[1] << 8;
                k += rbuf[2];
                prmcfg->set_socket_server_port(k);
                prmcfg->set_update(SocketPortIPUpdate);
                usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "CMD_PORT_NUM", 1);
            } else { //读
                k = prmcfg->socket_server_port();
                rbuf[1] = k >> 8;
                rbuf[2] = k & 0xff;
            }
            retval = 3;
            break;
        case CMD_SET_PROTOCOL: //设置通讯协议
            if (rbuf[0]) { //写
                k = 1;
                for (i = 0;i < 2;i++) {
                    prmcfg->set_comm_protocol(i, rbuf[1+i] > k ? k : rbuf[1+i]);
                }
                prmcfg->set_update(SerialPrtclUpdate);
                prmcfg->set_update(SocketPrtclUpdate);
                usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "CMD_SET_PROTOCOL", 1);
            }
            rbuf[1] = prmcfg->comm_protocol(0);
            rbuf[2] = prmcfg->comm_protocol(1);
            retval = 3;
            break;
        case CMD_SAVE_TYPE: //保存类型
            if (rbuf[0]) { //写
                syspara->hrm_save_type = rbuf[1] > 1 ? 1 : rbuf[1];
                syspara->freq_save_type = rbuf[2] > 1 ? 1 : rbuf[2];
                syspara->unbalance_save_type = rbuf[3] > 1 ? 1 : rbuf[3];
                syspara->voltdv_save_type = rbuf[4] > 1 ? 1 : rbuf[4];
                usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "CMD_SAVE_TYPE", 1);
            }
            rbuf[1] = syspara->hrm_save_type;
            rbuf[2] = syspara->freq_save_type;
            rbuf[3] = syspara->unbalance_save_type;
            rbuf[4] = syspara->voltdv_save_type;
            retval = 5;
            break;
        case CMD_SYSHIDE_PARAM: //系统及隐藏参数
            if (rbuf[0]) { //写
                prmcfg->set_zero_thr(0, rbuf[1]);
                prmcfg->set_zero_thr(1, rbuf[2]);
                prmcfg->set_pst_enable(rbuf[3]);
                prmcfg->set_save_wave_en(rbuf[4]);
                k = rbuf[5]<<8;
                k += rbuf[6];
                if (k!=prmcfg->harmrec_svmax()) {
                    prmcfg->set_harmrec_svmax(k);
                    notice_pthread(kTTSave, SAVEINFO, kAlterTrigger, NULL);
                }
                prmcfg->set_harmrec_sven(rbuf[7]);
                prmcfg->set_aggrgt_time_en(rbuf[8]);
                prmcfg->set_uextrm(rbuf[9]);
                prmcfg->set_fltrcpx(rbuf[10]);
                if (rbuf[11]==0) prmcfg->set_hrtbt_61850(0);
                else prmcfg->set_hrtbt_61850(0x79);
                prmcfg->SetSecurityEn(rbuf[12]);
                prmcfg->set_time_diff(rbuf[13]);
                usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "CMD_SYSHIDE_PARAM", 1);
            } else { //读
                rbuf[1] = prmcfg->zero_thr(0);  //voltage zero input threshold
                rbuf[2] = prmcfg->zero_thr(1);  //Current zero input threshold
                rbuf[3] = prmcfg->pst_enable();
                rbuf[4] = prmcfg->save_wave_en();
                rbuf[5] = prmcfg->harmrec_svmax()>>8;
                rbuf[6] = prmcfg->harmrec_svmax()&0xff;
                rbuf[7] = prmcfg->harmrec_sven();
                rbuf[8] = prmcfg->aggrgt_time_en();
                rbuf[9] = prmcfg->uextrm();     //Extrema voltage
                rbuf[10] = prmcfg->fltrcpx();    //Extrema voltage filter CPxx
                if (prmcfg->hrtbt_61850()==0x79) rbuf[11] = 1;
                else rbuf[11] = 0;
                rbuf[12] = prmcfg->security_en();
                rbuf[13] = prmcfg->time_diff();
            }
            retval = 14;
            break;
        case CMD_TIMESYN_PARAM: //对时参数
            if (rbuf[0]) { //写
                prmcfg->set_gps_single_type(rbuf[1]);
                syspara->gps_pulse_type = rbuf[2];
                k = rbuf[3]<<8;
                k += rbuf[4];
                syspara->proof_time_intr[0] = k;
                k = rbuf[5]<<8;
                k += rbuf[6];
                syspara->proof_time_intr[1] = k;
                k = rbuf[7]<<8;
                k += rbuf[8];
                syspara->b_time_intr = k;
                prmcfg->set_timezone(rbuf[9]);
                prmcfg->set_timezone(rbuf[10], 1);
                usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "CMD_TIMESYN_PARAM", 1);
            } else { //读
                rbuf[1] = prmcfg->gps_single_type();    //GPS signal type
                rbuf[2] = syspara->gps_pulse_type;      //Pulse singal type
                rbuf[3] = syspara->proof_time_intr[0]>>8;   //minutes pluse interval
                rbuf[4] = syspara->proof_time_intr[0]&0xff;
                rbuf[5] = syspara->proof_time_intr[1]>>8;   //seconds pluse interval
                rbuf[6] = syspara->proof_time_intr[1]&0xff;
                rbuf[7] = syspara->b_time_intr>>8;         //IRIG-B interval
                rbuf[8] = syspara->b_time_intr&0xff;
                rbuf[9] = prmcfg->timezone();
                rbuf[10] = prmcfg->timezone(1);
            }
            retval = 11;
            break;
        case CMD_CONNECTION: //接线方式
            if (rbuf[0]) { //写
                prmcfg->set_connect_type(rbuf[1]);
                prmcfg->set_update(DaramUpdate);
                usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "CMD_CONNECTION", 1);
            } else { //读
                rbuf[1] = prmcfg->connect_type();
            }
            retval = 2;
            break;
        case CMD_DC_COMPONENT: //获取直流分量
            k = pqmfunc->get_dcval_stat(&i);
            if (!k) pqmfunc->start_get_dc_val();
            rbuf[0] = k;
            retval = 1;
            usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "CMD_DC_COMPONENT", 1);
            break;
        case CMD_CAP_RA: //γ-α系数
        case CMD_CAP_THR: //Capacitor warning threshold param
        case CMD_RA_PARAM: //EEW Gamma-Alpha Warning parameter
        case CMD_TRNSF_PARAM: //EEW Gamma-Alpha Warning parameter
            k = 1;
            switch (cmd) {
                case CMD_CAP_RA:
                    pvoid = pee_warning->p_eew_ra_thr();
                    retval = sizeof(short)*24*3;
                    break;
                case CMD_CAP_THR:
                    pvoid = pee_warning->p_eew_cap_param();
                    retval = sizeof(EEWCapParam);
                    k = 3; //update daram too
                    break;
                case CMD_RA_PARAM:
                    pvoid = pee_warning->p_eew_ra_param();
                    retval = sizeof(EEWraParam);
                    k = 3; //update daram too
                    break;
                case CMD_TRNSF_PARAM:
                    pvoid = pee_warning->p_eew_t_param();
                    retval = sizeof(EEWTParam);
                default:
                    break;
            }
            if (rbuf[0]) {  //写
                memcpy(pvoid, &rbuf[1], retval);
                pee_warning->set_para_update(k);
            } else {        //读
                memcpy(&rbuf[1], pvoid, retval);
            }
            retval += 1;
            break;
        default:
            break;
    }
    memcpy(&m_tx_buffer[kDataHdSz], rbuf, retval);
    return retval;
}
//---------------------------------------------------------------------------

// 设置系统参数
int AppPrtclPQ::set_syspar(uchar *rbuf, unsigned int rsz)
{
    long li, lk;
    float fi;
    unsigned short usi;
    int k = 0, i, j;

    syspara->line_para.CUlevel = rbuf[k++];
    lk = rbuf[k++];
    li = lk << 24;
    lk = rbuf[k++];
    li += lk << 16;
    lk = rbuf[k++];
    li += lk << 8;
    li += rbuf[k++];
    if (li > MAX_PT1) li = MAX_PT1;
    if (li > 0) syspara->line_para.PT1 = li;
    lk = rbuf[k++];
    li = lk << 8;
    li += rbuf[k++];
    if (li > MAX_PT2) li = MAX_PT2;
    if (li > 0) syspara->line_para.PT2 = li;
    lk = rbuf[k++];
    li = lk << 8;
    li += rbuf[k++];
    if (li > MAX_CT1) li = MAX_CT1;
    //电流钳使能是，不要设置电流钳CT的变比
    if (li > 0 && !prmcfg->sys_para_sg()->current_clamp_enable) {
        syspara->line_para.CT1 = li;
        prmcfg->set_cclamp_para(3, syspara->line_para.CT1);
    }
    int ct2_ext = 0;
    usi = rbuf[k++];
    if (usi != 0xff) {
        if (usi > 0 && !prmcfg->sys_para_sg()->current_clamp_enable) syspara->line_para.CT2 = usi;
    } else {
        ct2_ext = 1;
    }

    lk = rbuf[k++];
    li = lk << 16;
    lk = rbuf[k++];
    li += lk << 8;
    fi = (float)(li + rbuf[k++]);
    syspara->line_para.Short_cap = fi / 10;
    lk = rbuf[k++];
    li = lk << 16;
    lk = rbuf[k++];
    li += lk << 8;
    fi = (float)(li + rbuf[k++]);
    syspara->line_para.User_cap = fi / 10;
    lk = rbuf[k++];
    li = lk << 16;
    lk = rbuf[k++];
    li += lk << 8;
    fi = (float)(li + rbuf[k++]);
    syspara->line_para.Supp_cap = fi / 10;
    lk = rbuf[k++];
    li = lk << 8;
    fi = (float)(li + rbuf[k++]);
    syspara->line_para.Volt_warp[0] = fi / 10;
    lk = rbuf[k++];
    li = lk << 8;
    fi = (float)(li + rbuf[k++]);
    syspara->line_para.Volt_warp[1] = -fi / 10;

    usi = rbuf[k++];
    usi |= rbuf[k++] << 8;
    if (usi > 1000) usi = 1000;
    syspara->vvr_limit[1] = usi;
    usi = rbuf[k++];
    usi |= rbuf[k++] << 8;
    if (usi < 1000) usi = 1000;
    if (usi > 9999) usi = 9999;
    syspara->vvr_limit[0] = usi;
    syspara->transt_monitor = rbuf[k++];
    prmcfg->set_inter_enable(rbuf[k++]);  //29
    usi = rbuf[k++];
    usi |= rbuf[k++] << 8;  //30 31
    prmcfg->set_inter_limit(usi);
    syspara->unbalance_thr[0] = rbuf[k++] << 8; //32
    syspara->unbalance_thr[0] |= rbuf[k++];  //33
    syspara->unbalance_thr[1] = rbuf[k++] << 8; //34
    syspara->unbalance_thr[1] |= rbuf[k++]; // 35
    syspara->neg_sequence_Ithr = rbuf[k++] << 8; //36
    syspara->neg_sequence_Ithr |= rbuf[k++]; // 37
    usi = rbuf[k++] << 8;
    usi |= rbuf[k++];
    syspara->tb_low_limit = usi;
    usi = rbuf[k++] << 8;
    usi |= rbuf[k++];
    syspara->tb_high_limit = usi;
    syspara->transt_tb_enable = rbuf[k++] & 7;
    usi = rbuf[k++] << 8;
    usi |= rbuf[k++];
    syspara->transt_end_num = (usi / 5) * 5;
    usi = rbuf[k++] << 8;
    usi |= rbuf[k++];
    if (usi < 2) usi = 2;
    else if (usi > 60) usi = 60;
    prmcfg->set_transt_rcd_time(usi);
    usi = rbuf[k++] & 1;
    prmcfg->set_manual_rec_enable(usi);
    usi = rbuf[k++] << 8;
    usi |= rbuf[k++];
    if (ct2_ext) {
        if (usi > MAX_CT2) usi = MAX_CT2;
        if (usi > 0) syspara->line_para.CT2 = usi;
    }
    usi = rbuf[k++] << 8;
    usi |= rbuf[k++];
    if (usi>0) prmcfg->set_reset_hyst_time(usi);
    //printf("rsz=%d\n", rsz);
    if (rsz > 60) {
        usi = rbuf[k++] << 8;
        usi |= rbuf[k++];
        if (usi > 1000) usi = 1000;
        syspara->vvr_limit[2] = usi;
        usi = rbuf[k++] << 8;
        usi |= rbuf[k++];
        syspara->sc_limit = usi;
        syspara->start_cur_en = rbuf[k++];
        usi = rbuf[k++] << 8;
        usi |= rbuf[k++];
        syspara->ivr_limit[1] = usi;
        usi = rbuf[k++] << 8;
        usi |= rbuf[k++];
        syspara->ivr_limit[0] = usi;
        syspara->transt_i_monitor = rbuf[k++];
        prmcfg->set_rcd_wv_fmt(rbuf[k++]);
        syspara->fluct_enable = rbuf[k++];
        usi = rbuf[k++] << 8;
        usi |= rbuf[k++];
        syspara->fluct_db = usi;
        prmcfg->set_inharm_type(rbuf[k++]);
    }
    prmcfg->set_update(DaramUpdate);
    m_tx_buffer[kDataHdSz] = rbuf[0];
    usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "CMD_WTSET", 1);
    return 1;
}
//---------------------------------------------------------------------------


// 读取并发送系统参数
int AppPrtclPQ::tx_syspar(uchar *rbuf)
{
    int i, j, k;
    unsigned short usi;
    j = kDataHdSz;

    //m_tx_buffer[j++] = rbuf[0];
    m_tx_buffer[j++] = syspara->line_para.CUlevel + 1;
    m_tx_buffer[j++] = syspara->line_para.PT1 >> 24;
    m_tx_buffer[j++] = syspara->line_para.PT1 >> 16;
    m_tx_buffer[j++] = syspara->line_para.PT1 >> 8;
    m_tx_buffer[j++] = syspara->line_para.PT1 & 0xff;
    m_tx_buffer[j++] = syspara->line_para.PT2 >> 8;
    m_tx_buffer[j++] = syspara->line_para.PT2 & 0xff;
    m_tx_buffer[j++] = syspara->line_para.CT1 >> 8;
    m_tx_buffer[j++] = syspara->line_para.CT1 & 0xff;
    if (syspara->line_para.CT2 >= 255)
        m_tx_buffer[j++] = 0xff;
    else
        m_tx_buffer[j++] = syspara->line_para.CT2;
    i = int(syspara->line_para.Short_cap * 10);
    m_tx_buffer[j++] = i >> 16;
    m_tx_buffer[j++] = i >> 8;
    m_tx_buffer[j++] = i & 0xff;
    i = int(syspara->line_para.User_cap * 10);
    m_tx_buffer[j++] = i >> 16;
    m_tx_buffer[j++] = i >> 8;
    m_tx_buffer[j++] = i & 0xff;
    i = int(syspara->line_para.Supp_cap * 10);
    m_tx_buffer[j++] = i >> 16;
    m_tx_buffer[j++] = i >> 8;
    m_tx_buffer[j++] = i & 0xff;
    i = int(syspara->line_para.Volt_warp[0] * 10);
    m_tx_buffer[j++] = i >> 8;
    m_tx_buffer[j++] = i & 0xff;
    i = -int(syspara->line_para.Volt_warp[1] * 10);
    m_tx_buffer[j++] = i >> 8;
    m_tx_buffer[j++] = i & 0xff;
    m_tx_buffer[j++] = prmcfg->sys_para_sg()->passwd >> 24;
    m_tx_buffer[j++] = prmcfg->sys_para_sg()->passwd >> 16;
    m_tx_buffer[j++] = prmcfg->sys_para_sg()->passwd >> 8;
    m_tx_buffer[j++] = prmcfg->sys_para_sg()->passwd;
    memcpy(&m_tx_buffer[j], &syspara->vvr_limit[1], 2);
    j += 2;
    memcpy(&m_tx_buffer[j], &syspara->vvr_limit[0], 2);
    j += 2;
    m_tx_buffer[j++] = syspara->transt_monitor;
    m_tx_buffer[j++] = prmcfg->inter_enable();
    usi = prmcfg->inter_limit();
    memcpy(&m_tx_buffer[j], &usi, 2);
    j += 2;
    m_tx_buffer[j++] = syspara->unbalance_thr[0] >> 8;
    m_tx_buffer[j++] = syspara->unbalance_thr[0] & 0xff;
    m_tx_buffer[j++] = syspara->unbalance_thr[1] >> 8;
    m_tx_buffer[j++] = syspara->unbalance_thr[1] & 0xff;
    k = j;
    m_tx_buffer[j++] = syspara->neg_sequence_Ithr >> 8;
    m_tx_buffer[j++] = syspara->neg_sequence_Ithr & 0xff;
    //add by gjq 2009-08-07
    m_tx_buffer[j++] = syspara->tb_low_limit >> 8;
    m_tx_buffer[j++] = syspara->tb_low_limit & 0xff;
    m_tx_buffer[j++] = syspara->tb_high_limit >> 8;
    m_tx_buffer[j++] = syspara->tb_high_limit & 0xff;
    m_tx_buffer[j++] = syspara->transt_tb_enable;
    m_tx_buffer[j++] = syspara->transt_end_num >> 8;
    m_tx_buffer[j++] = syspara->transt_end_num & 0xff;
    m_tx_buffer[j++] = prmcfg->transt_rcd_time() >> 8;
    m_tx_buffer[j++] = prmcfg->transt_rcd_time() & 0xff;
    m_tx_buffer[j++] = prmcfg->manual_rec_enable();
    if (syspara->line_para.CT2 >= 255) {
        m_tx_buffer[j++] = syspara->line_para.CT2 >> 8;
        m_tx_buffer[j++] = syspara->line_para.CT2 & 0xff;
    } else {
        m_tx_buffer[j++] = 0;
        m_tx_buffer[j++] = 0;
    }
    m_tx_buffer[j++] = prmcfg->reset_hyst_time()>>8;
    m_tx_buffer[j++] = prmcfg->reset_hyst_time()&0xff;
    m_tx_buffer[j++] = syspara->vvr_limit[2]>>8;  //short time interrupt limit
    m_tx_buffer[j++] = syspara->vvr_limit[2]&0xff;
    m_tx_buffer[j++] = syspara->sc_limit>>8;      //startup current limit
    m_tx_buffer[j++] = syspara->sc_limit&0xff;
    m_tx_buffer[j++] = syspara->start_cur_en;
    m_tx_buffer[j++] = syspara->ivr_limit[1]>>8;  //current variation low limit
    m_tx_buffer[j++] = syspara->ivr_limit[1]&0xff;
    m_tx_buffer[j++] = syspara->ivr_limit[0]>>8;  //current variation high limit
    m_tx_buffer[j++] = syspara->ivr_limit[0]&0xff;
    m_tx_buffer[j++] = syspara->transt_i_monitor;
    m_tx_buffer[j++] = prmcfg->rcd_wv_fmt();
    m_tx_buffer[j++] = syspara->fluct_enable;
    m_tx_buffer[j++] = syspara->fluct_db>>8;  //fluctuation deadband
    m_tx_buffer[j++] = syspara->fluct_db&0xff;
    m_tx_buffer[j++] = prmcfg->inharm_type();
    
#if 0
    printf("m_tx_buffer spos=%d data=", k);
    for (i = k; i < j; i++)
        printf("%02X ", m_tx_buffer[i]);
    printf("\n");
#endif
    return j - kDataHdSz;
}
//---------------------------------------------------------------------------

// 设置单元时间
void AppPrtclPQ::set_time(uchar *rbuf)
{
    if (!timer_setime_->TimeOut()) return;
    timer_setime_->Start(60*5);

    time_t tm_ti = time(NULL);
    struct tm tmi;
    GmTime(&tmi, &tm_ti);

    switch(syspara->gps_single_type) {
        case 3: //关
            tmi.tm_sec = rbuf[0];
        case 2: //脉冲对时,不校秒
            tmi.tm_min = rbuf[1];
            tmi.tm_hour = rbuf[2];
            tmi.tm_mday = rbuf[3];
            tmi.tm_mon = rbuf[4]-1;
        default:    //B码+脉冲或者B码,只校年
            tmi.tm_year = rbuf[5];
            if (tmi.tm_year < 90) {
                tmi.tm_year += 100;
            }
    }
    pqm_view().set_time_buf(&tmi);
    usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "Set time", 1);
    notice_pthread(kTTDisplay, COMMINFO, SET_TIME, NULL);
}
//---------------------------------------------------------------------------

//Description: 处理自定义限值的读写命令
//Input: rbuf, 接收到的数据，第1个字节是命令
int AppPrtclPQ::custom_limit_handle(uchar *rbuf)
{
    long li;
    int i, k, retval;

    k = kDataHdSz;
    m_tx_buffer[k++] = rbuf[0];
    if (rbuf[0]) { //写
        k = 1;
        syspara->limit_type = rbuf[k++];
        syspara->freq_limit[0] = rbuf[k++] << 8;
        syspara->freq_limit[0] |= rbuf[k++];
        syspara->freq_limit[1] = rbuf[k++] << 8;
        syspara->freq_limit[1] |= rbuf[k++];
        syspara->pst_limit = rbuf[k++] << 8;
        syspara->pst_limit |= rbuf[k++];
        syspara->pst_limit *= 10;
        for (i = 0;i < 25;i++) {
            li = rbuf[k++] << 8;
            li |= rbuf[k++];
            syspara->harm_ulimit[i] = li;
        }
        for (i = 0;i < 24;i++) {
            li = rbuf[k++] << 8;
            li |= rbuf[k++];
            syspara->harm_ilimit[i] = li;
        }
        retval = 1;
        usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "CMD_CUSTOM_LIMIT", 1);
    } else { //读
        m_tx_buffer[k++] = syspara->limit_type;
        m_tx_buffer[k++] = syspara->freq_limit[0] >> 8;
        m_tx_buffer[k++] = syspara->freq_limit[0];
        m_tx_buffer[k++] = syspara->freq_limit[1] >> 8;
        m_tx_buffer[k++] = syspara->freq_limit[1];
        
        m_tx_buffer[k++] = (syspara->pst_limit/10) >> 8;
        m_tx_buffer[k++] = syspara->pst_limit/10;
        for (i = 0;i < 25;i++) {
            m_tx_buffer[k++] = syspara->harm_ulimit[i] >> 8;
            m_tx_buffer[k++] = syspara->harm_ulimit[i];
        }
        for (i = 0;i < 24;i++) {
            m_tx_buffer[k++] = syspara->harm_ilimit[i] >> 8;
            m_tx_buffer[k++] = syspara->harm_ilimit[i];
        }
        retval = k;
    }
    return retval;
}

/* ---------------------------------------------------------------------------
Description:处理CVT修正系数的读写命令
Input: 		rbuf, 接收到的数据，第1个字节是命令
Return:		响应命令的字节数
--------------------------------------------------------------------------- */
int AppPrtclPQ::cvt_modify_handle(uchar *rbuf)
{
    long li;
    int i, k, retval;

    k = kDataHdSz;
    m_tx_buffer[k++] = rbuf[0];
    if (rbuf[0]) { //写
        k = 1;
        syspara->cvt_modify_group = rbuf[k++];
        syspara->cvt_modify_unit = rbuf[k++];
        prmcfg->set_cvt_modify_en(rbuf[k++]);
        for (i = 0;i < 49;i++) {
            li = rbuf[k++] << 8;
            li |= rbuf[k++];
            prmcfg->set_cvt_modify(i, li);
        }
        retval = 1;
        usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "CMD_CVT_MODIFY", 1);
    } else { //读
        m_tx_buffer[k++] = syspara->cvt_modify_group;
        m_tx_buffer[k++] = syspara->cvt_modify_unit;
        m_tx_buffer[k++] = prmcfg->cvt_modify_en();
        for (i = 0;i < 49;i++) {
            m_tx_buffer[k++] = prmcfg->cvt_modify(i) >> 8;
            m_tx_buffer[k++] = prmcfg->cvt_modify(i);
        }
        retval = k;
    }
    return retval;
}
//---------------------------------------------------------------------------

//Description: 稳态超限时，触发暂态录波使能命令的处理
int AppPrtclPQ::trigger_able(uchar *rbuf)
{
    if (rbuf[0]) { //写
        syspara->trigger_enable = rbuf[1];
        usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "CMD_TRIG_ENABLE", 1);
    }
    m_tx_buffer[kDataHdSz] = rbuf[0];
    m_tx_buffer[kDataHdSz+1] = syspara->trigger_enable;
    return 2;
}

int AppPrtclPQ::tx_smp_ver(unsigned char *rbuf)
{
    unsigned short dsp_ver;
    unsigned short dsp_type;

    if (rbuf[0]==173) { //写
        return 1;
    } else {    //读
        read_dsp_ver(&dsp_ver, &dsp_type);
        m_tx_buffer[kDataHdSz] = prmcfg->pqm_type();
    // *(unsigned long *)(m_tx_buffer+kDataHdSz+1)=(prmcfg->sys_para_sg()->device_sn);///这样传输数据会错位。long会把前面覆盖掉
        memcpy(m_tx_buffer + kDataHdSz + 1, &(prmcfg->sys_para_sg()->device_sn), 4);
        m_tx_buffer[kDataHdSz+5] = dsp_ver >> 12;
        m_tx_buffer[kDataHdSz+6] = (dsp_ver & 0x0fc0) >> 6;
        m_tx_buffer[kDataHdSz+7] = dsp_ver & 0x003f;
    // printf("%d--%d--%d\n",m_tx_buffer[kDataHdSz+5],m_tx_buffer[kDataHdSz+6],m_tx_buffer[kDataHdSz+7]);
        m_tx_buffer[kDataHdSz+8] = VER_1st;
        m_tx_buffer[kDataHdSz+9] = VER_2nd;
        m_tx_buffer[kDataHdSz+10] = VER_3rd;
        m_tx_buffer[kDataHdSz+11] = VER_sn;
        memcpy(m_tx_buffer + kDataHdSz + 12, &dsp_type, 2);
        return 32;
    }
}
//---------------------------------------------------------------------------
int AppPrtclPQ::tx_smp_data(unsigned char *rbuf)
{
    unsigned short k[3];
    k[0] = *(unsigned short*)rbuf;
    k[1] = *(unsigned short*) & rbuf[2];
    k[2] = *(unsigned short*) & rbuf[4];
    ///第一包，建文件
    if (k[1] == 1)
        fp = fopen(UpdspFile, "wb+");


    ///存储数据并设置回应值
    if (k[1] <= k[0]) {
        fwrite(rbuf + 6, 1, k[2], fp);
        m_tx_buffer[kDataHdSz] = 1;
    } else {///升级失败
        m_tx_buffer[kDataHdSz] = 0;
        printf("download file failure!\n");
    }
    if (k[1] == k[0]) { ///接收完成
        fclose(fp);
        updsp_syn = 1;
    }
    return 1;
}

int AppPrtclPQ::tx_smp_suc(unsigned char *rbuf)
{
    if (updsp_syn == 0) {   //send up firmware end
        get_dsp_update_state();
    }
    int k = updsp_flag();
    if (k==DSP_RET_SUC) {
        usr_mng().SetAudit("app", usr_mng().usr_name(user_id_), "Update dsp firmware", 1);
    } else if (k>DSP_RET_UPDATING) {
        usr_mng().SetAudit("app", usr_mng().usr_name(user_id_), "Update dsp firmware", 2);
    }
    if (k!=DSP_RET_UPDATING) set_updsp_flag(DSP_RET_UPDATING);
    m_tx_buffer[kDataHdSz] = k;

    return 1;
}

int AppPrtclPQ::TxRandkey()
{
    //printf("recieved cmd:CMD_RANDKEY\n");
    memcpy(&m_tx_buffer[kDataHdSz], secur_mng_->rand_key(), 16);
    //printf("%02x%02x%02x%02x\n", m_tx_buffer[kDataHdSz], m_tx_buffer[kDataHdSz+1], m_tx_buffer[kDataHdSz+2], m_tx_buffer[kDataHdSz+3]);
    return 16;
}

int AppPrtclPQ::TxAuditLog(uchar *rbuf)
{
    int num = 20;
    if (rbuf[1]) num = rbuf[1];
    ptx_bufd = new uchar[kDataHdSz + 4 + num*64];
    //printf("rbuf=%d %d %d %d\n", rbuf[0], rbuf[1], rbuf[2], rbuf[3]);
    int start_p = (rbuf[2]<<8) + rbuf[3];
    num = usr_mng().ReadAuditRec((char*)&ptx_bufd[kDataHdSz+4], rbuf[0], start_p, num);
    ptx_bufd[kDataHdSz] = rbuf[0];
    ptx_bufd[kDataHdSz+1] = num;

    int k = 1;
    if (start_p==0) {
        if (strcmp("auditor", usr_mng().usr_name(user_id_))) k = 2;
        if (rbuf[0]) {
            usr_mng().SetAudit("audit", usr_mng().usr_name(user_id_), "Read audit alarm event", k);
        } else {
            usr_mng().SetAudit("audit", usr_mng().usr_name(user_id_), "Download audit log", k);
        }
    }

    return (num * 64 + 4);
}

int AppPrtclPQ::Identify(uchar *rbuf)
{
    secur_mng_->UpdateKey();
    int k = secur_mng_->Authenticate(rbuf);
    m_tx_buffer[kDataHdSz] = k;
    if (k) usr_mng().SetAudit("warning", "system", "Identity authentication", 2);
    return 1;
}

/*!
    Return: 0=passed, 1=user not exist, 2=passwd error
*/
int AppPrtclPQ::Login(uchar *rbuf)
{
    char pswd[33], pswdl[33];
    uint32_t sm4buf[4];
    memcpy(sm4buf, &rbuf[17], 16);
    secur_mng_->DecryptoStr(pswd, sm4buf);
    memcpy(sm4buf, &rbuf[33], 16);
    secur_mng_->DecryptoStr(pswdl, sm4buf);
        
    int k;
    if (rbuf[0]==0) {       //Login
        if (ps_err_cnt_==0) {
            if (!timer_lock_->TimeOut()) {
                return 2;  //Access denied
            } else {
                ps_err_cnt_ = 5;
            }
        }
        user_id_ = usr_mng().FindUser((char*)&rbuf[1]);
        if (user_id_>=0) {
            k = usr_mng().CheckPasswd(pswd, user_id_, 1);
            if (!k) {
                ps_err_cnt_ = 5;
                logged_ = true;
                timer_logout_->Start(prmcfg->lcm_dely_time()*60);
            } else {
                k++;    //passwd error
                if (--ps_err_cnt_<=0) {
                    timer_lock_->Start(60);
                }
            }
        } else {
            k = 1;
        }
        usr_mng().SetAudit("login", (char*)&rbuf[1], "Remote login", k+1);
        if (ps_err_cnt_<=0) usr_mng().SetAudit("warning", usr_mng().usr_name(user_id_), "Cont remote login err", 0);
    } else if (rbuf[0]==1) {   //Add user
        if (user_id_==0) {  //admin
            k = usr_mng().AddUser((char*)&rbuf[1], pswdl, pswd);
        } else {
            k = 2;  //Access denied
        }
        usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "Add user", k+1);
    } else if (rbuf[0]==2) {   //Delete user
        if (user_id_==0) {  //admin
            k = usr_mng().DelUser((char*)&rbuf[1]);
        } else {
            k = 2;  //Access denied
        }
        usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "Delete user", k+1);
    }
    m_tx_buffer[kDataHdSz] = rbuf[0];
    m_tx_buffer[kDataHdSz+1] = k;
    //printf("k=%d\n", k);
    return 2;
}

int AppPrtclPQ::GetUsers()
{
    int i, n = usr_mng().usr_num();
    m_tx_buffer[kDataHdSz] = n;
    for (i=0; i<n; i++) {
        strcpy((char*)&m_tx_buffer[kDataHdSz+1+i*16], usr_mng().usr_name(i));
    }
    return n*16+1;
}

int AppPrtclPQ::ChgPasswd(uchar *rbuf)
{
    char pswd[33];
    uint32_t buf[4];
    memcpy(buf, &rbuf[1], 16);
    secur_mng_->DecryptoStr(pswd, buf);
    int k;
    if (rbuf[0]==0) k = usr_mng().CheckPasswd(pswd, user_id_, 2);
    else k = usr_mng().CheckPasswd(pswd, user_id_, rbuf[0]);
    
    if (k==0) {
        memcpy(buf, &rbuf[17], 16);
        secur_mng_->DecryptoStr(pswd, buf);
        usr_mng().SetPasswd(pswd, user_id_, rbuf[0]);
        m_tx_buffer[kDataHdSz] = 0;
    } else {
        m_tx_buffer[kDataHdSz] = 1;
    }
    usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "Change password", k+1);
    return 1;
}

int AppPrtclPQ::SetDeviceSn(uchar *rbuf)
{
    char pswd[33];
    secur_mng_->DecryptoStr(pswd, (uint32_t*)&rbuf[4]);
    int k = usr_mng().CheckPasswd(pswd, user_id_, 1);
    
    //printf("ChgPasswd, k=%d\n", k);
    if (k==0) {
        memcpy(&(prmcfg->sys_para_sg()->device_sn), &rbuf[0], 4);
        char *key = (char*)malloc(36);
        secur_mng_->DecryptoStr(key, (uint32_t*)&rbuf[20]);
        //printf("key=%s\n", key);
        notice_pthread(kTTSave, SAVEINFO, kSaveSM4Key, key);
        m_tx_buffer[kDataHdSz] = 0;
    } else {
        m_tx_buffer[kDataHdSz] = 1;
    }
    usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "Device sn", k+1);
    return 1;
}

int AppPrtclPQ::IniDev(uchar *rbuf, char *update)
{
    char pswd[33];
    secur_mng_->DecryptoStr(pswd, (uint32_t*)&rbuf[0]);
    int k = usr_mng().CheckPasswd(pswd, user_id_, 1);
    
    if (k==0) {
        if (timer_inidev_->TimeOut()) {
            timer_inidev_->Start(60);
            *update = 2;
            usr_mng().SetAudit("warning", usr_mng().usr_name(user_id_), "Device initialize", 1);
        }
        m_tx_buffer[kDataHdSz] = 0;
    } else {
        m_tx_buffer[kDataHdSz] = 1;
        usr_mng().SetAudit("warning", usr_mng().usr_name(user_id_), "Device initialize", 2);
    }
    return 1;
}

