#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "../../thread/pthread_mng.h"
#include "../../pqm_func/pqmfunc.h"
#include "../../GUI/view.h"
#include "../../EEW/ee_warning.h"
#include "../../pqm_func/harmfunc.h"
#include "../../pqm_func/save_func.h"
#include "../../pqm_func/other_func.h"
#include "../../pqm_func/volt_variation.h"
#include "../../device/device.h"
#include "../../security/user_manage.h"
#include "app_prtcl_pqb.h"
#include "phy_prtcl_pqb.h"
#include "../../base_func/timer_cstm.h"


extern const char * TranstFile; //暂态数据存储文件
//static const int kDataHdSz = sizeof(CommDataHead); //Data Head Size

AppPrtclPqB::AppPrtclPqB(): AppPrtclPQ(sizeof(CommDataHead))
{
    m_data_hd = new CommDataHead;
}

AppPrtclPqB::~AppPrtclPqB()
{
    delete m_data_hd;
}

//---------------------------------------------------------------------------

//设置记录数据前缀的几个字节的内容
#define SET_RCD_HEAD(a) \
 a[kDataHdSz] = rbuf[1];\
 memcpy(&a[kDataHdSz+1], &fetch_rcd.left_num, sizeof(long))
//考虑到字节对齐的情况，千万不要用下行的语句替代上句
//*(long*)&m_tx_buffer[2] = fetch_rcd.left_num;


//Description: 在给定范围内定位与指定时间最接近的记录位置。
//Input: time,记录对应的时间; tol, 记录总数; pos, 最新记录所在位置
//  end,端点,0=起点,1=终点;
//Return: 记录位置
int AppPrtclPqB::locate_transt_pos(timeval &time, unsigned int tol,
                                  unsigned int pos, int end)
{
    timeval * timep;
    unsigned int idx, min, max, num;
    int i;

    min = 1;
    max = tol;
    num = (min + max) / 2;
    for (i = 0; i < 12; i++) {
        if (max - min <= 1) break;
        idx = pos >= num ? pos - num : pos + transt_max - num;
        timep = &trst_rcd->transt_sv_time[idx];

        if (timep->tv_sec < time.tv_sec ||
            timep->tv_sec == time.tv_sec &&
            timep->tv_usec <= time.tv_usec) {//timep早于等于tmvl
            max = num;
            num = (min + max) / 2;
        } else {
            min = num;
            num = (min + max) / 2;
        }
    }
    if (end) { //end point
        num = min;
        while (num <= max) {
            idx = pos >= num ? pos - num : pos + transt_max - num;
            timep = &trst_rcd->transt_sv_time[idx];
            if (timep->tv_sec < time.tv_sec ||
                timep->tv_sec == time.tv_sec &&
                timep->tv_usec <= time.tv_usec) {//timep早于等于tmvl
                return num;
            }
            num++;
        }
        return num;
    } else { //start point
        num = max;
        while (num >= min) {
            idx = pos >= num ? pos - num : pos + transt_max - num;
            timep = &trst_rcd->transt_sv_time[idx];
            if (timep->tv_sec > time.tv_sec ||
                timep->tv_sec == time.tv_sec &&
                timep->tv_usec >= time.tv_usec) {//timep晚于等于tmvl
                return num;
            }
            num--;
        }
        return num;
    }
}
//---------------------------------------------------------------------------

//Description: 统计时间范围内暂态事件数目。
//Input: start_time,起始时间; end_time,结束时间
//Variable: event_pos,事件的起始位置
//Return: 事件数目;
int AppPrtclPqB::stat_transt(timeval &start_time, timeval &end_time)
{
    unsigned int tol, pos, num;

    tol = trst_rcd->transt_tol;
    pos = trst_rcd->transt_pos;
    if (!tol) return 0;

    unsigned int head, tail;

    head = locate_transt_pos(start_time, tol, pos, 0);
    if (head < 1) return 0;
    tail = locate_transt_pos(end_time, tol, pos, 1);
    if (tail > tol) return 0;
    num = head - tail + 1;
    event_pos = (pos >= head) ? pos - head : pos + transt_max - head;
    return num;
}

//---------------------------------------------------------------------------
// 读取并发送暂态事件记录数据。
int AppPrtclPqB::tx_trnst_rec(uchar *rbuf)
{
    char filename[40];
    int pkg_sz = 128 * 6;
    long li;
    int i, j;
    timeval start_tmvl, end_tmvl;
    TRST_F_HEAD fhead;

    transt_cnt = 0;
    if (rbuf[0]) {  //首次发送的命令. (rbuf[0]==0xa5)
        memcpy(&start_tmvl, &rbuf[2], sizeof(timeval));
        memcpy(&end_tmvl, &rbuf[2 + sizeof(timeval)], sizeof(timeval));
        transt_max = prmcfg->sys_para_sg()->transt_max;
        event_num = stat_transt(start_tmvl, end_tmvl);
        event_pkg_num = 0;
        fetch_rcd.pkgsn = rbuf[1] + 4;
        //printf("event_num=%d, max=%d\n", event_num, prmcfg->sys_para_sg()->transt_max);
    }
    if (event_num) {
        if (!event_pkg_num || f_transtu == NULL) { //新的暂态事件纪录(包号为0或暂态记录文件未打开)
            i = trst_rcd->transt_pos;   //最新暂态文件的位置编号
            //防止当取暂态时，又有新的暂态发生并与正取的暂态重叠。event_pos 为当前正在取的暂态文件位置编号。
            while (((event_pos + transt_max - i) % transt_max) < 2 && event_num > 0) {
                event_num--;
                event_pos++;
                event_pos %= transt_max;
            }
            do {
                if (f_transtu != NULL) {
                    fclose(f_transtu);
                    f_transtu = NULL;
                }
                if (f_transti != NULL) {
                    fclose(f_transti);
                    f_transti = NULL;
                }
                sprintf(filename, "%sv%i.sav", TranstFile, event_pos);
                f_transtu = fopen(filename, "rb");
                if (f_transtu == NULL) { //如果暂态事件记录文件不存在，继续下一个
                    printf("File %s not exist!\n", filename);
                    event_num--;
                    event_pos++;
                    event_pos %= transt_max;
                    if (event_num) continue;
                } else {
                    sprintf(filename, "%sc%i.sav", TranstFile, event_pos);
                    f_transti = fopen(filename, "rb");
                }
                break;
            } while (1);
            if (event_num > 0) {
                if (f_transti != NULL) {
                    fseek(f_transti, 2, SEEK_SET);
                    fread(&fhead, 1, sizeof(TRST_F_HEAD), f_transti);
                    li = fhead.head_bytes + fhead.tail_bytes;
                    event_pkg_num_i = li / pkg_sz;  //总包数
                    if (f_transti) fseek(f_transti, 2 + sizeof(TRST_F_HEAD), SEEK_SET);
                } else {
                    event_pkg_num_i = 0;
                }
                fseek(f_transtu, 2, SEEK_SET);
                fread(&fhead, 1, sizeof(TRST_F_HEAD), f_transtu);
                li = fhead.head_bytes + fhead.tail_bytes;
                event_pkg_num = li / pkg_sz;//总包数
                //printf("filename:%s head_bytes:%d tail:%d event_pkg_num:%d li:%ld\n", filename, fhead.head_bytes, fhead.tail_bytes, event_pkg_num, li);
                fhead.tail_bytes = fhead.tail_bytes / pkg_sz;//尾部包数
                event_smpfrq = fhead.smpl_freq;
                event_cause = fhead.cause;
                event_uscale_ = fhead.uscale;
                event_iscale_ = fhead.iscale;
                event_end_time = fhead.etime;
                event_tail_pkg_num = fhead.tail_bytes;
            }
        }
        //if (event_num) {
            if (fetch_rcd.pkgsn == rbuf[1]) {   //如为重复取重新调整文件指针
                if (ftell(f_transtu) > pkg_sz)
                    fseek(f_transtu, -pkg_sz, SEEK_CUR);
                if (f_transti && ftell(f_transti) > pkg_sz)
                    fseek(f_transti, -pkg_sz, SEEK_CUR);
                event_pkg_num += 1;
                event_pkg_num_i += 1;
            } else { //正常进行则更新最近读取包队列
                fetch_rcd.pkgsn = rbuf[1];
            }
        //}
    }

    if (event_num && event_pkg_num) {
        memcpy(&m_tx_buffer[kDataHdSz + 3], &trst_rcd->transt_sv_time[event_pos], sizeof(timeval));
        i = kDataHdSz + 3 + sizeof(timeval);//31 11

        memmove(&m_tx_buffer[i], &event_pkg_num, sizeof(long));
        i += sizeof(long);//35 15
        memmove(&m_tx_buffer[i], &pkg_sz, sizeof(short));
        i += sizeof(short);  // 17
        m_tx_buffer[i++] = event_smpfrq; //18
        m_tx_buffer[i++] = event_cause; //19
        m_tx_buffer[i++] = event_uscale_;//电压单位
        if (event_pkg_num_i > 0 && f_transti)
            m_tx_buffer[i++] = event_iscale_;//电流单位   //21
        else
            m_tx_buffer[i++] = 0;//电流单位   //21
        memmove(&m_tx_buffer[i], &event_tail_pkg_num, sizeof(long));
        i += sizeof(long);//25 45
        memmove(&m_tx_buffer[i], &event_end_time, sizeof(timeval));
        i += sizeof(timeval); //33  53
        fread(&m_tx_buffer[i], 1, pkg_sz, f_transtu);
        i += pkg_sz; // 33+768  53+768=
        if (event_pkg_num_i > 0 && f_transti)
            fread(&m_tx_buffer[i], 1, pkg_sz, f_transti);
        else
            memset(&m_tx_buffer[i], 0, pkg_sz);
        event_pkg_num--;
        event_pkg_num_i--;
    } else { //所有的暂态事件记录发送完毕
        pkg_sz = 0;
        i = kDataHdSz + 3 + sizeof(timeval);
        *(int*)&m_tx_buffer[i] = 0;//剩余包数
        i += sizeof(int);
        memmove(&m_tx_buffer[i], &pkg_sz, sizeof(short));
        i += 2;
        m_tx_buffer[i++] = event_smpfrq;
        m_tx_buffer[i++] = event_cause;
        m_tx_buffer[i++] = fhead.uscale;//电压单位
        m_tx_buffer[i++] = fhead.iscale;//电流单位
        memmove(&m_tx_buffer[i], &event_tail_pkg_num, sizeof(long));
        i += sizeof(long);
        memmove(&m_tx_buffer[i], &event_end_time, sizeof(timeval));
    }
    m_tx_buffer[kDataHdSz] = rbuf[1];
    //printf("event_pkg_num=%d, event_num=%d\n", event_pkg_num, event_num);
    memmove(&m_tx_buffer[kDataHdSz + 1], &event_num, sizeof(short));
    if (event_pkg_num == 0 && event_num) {//一个暂态事件记录发送完毕
        fclose(f_transtu);
        if (f_transti != NULL)  fclose(f_transti);
        f_transtu = NULL;
        f_transti = NULL;
        event_num--;
        event_pos++;
        event_pos %= transt_max;
    }
    return 33 + pkg_sz * 2;
}
//---------------------------------------------------------------------------

int AppPrtclPqB::TxPower01Time(uchar *rbuf)
{
    time_t tm_ary[10];  //Power on&off time
    int num01 = save_func->ReadPower01Time(tm_ary, 10);

    m_tx_buffer[kDataHdSz] = num01;
    memcpy(&m_tx_buffer[kDataHdSz + 1], tm_ary, num01 * sizeof(time_t));
    return num01 * sizeof(time_t) + 1;
}

/*!
Description: 通讯命令处理

    Input:  rbuf -- 收到的数据
    Output: tbuf,sz -- 要发送的应答数据及其大小
    Return: 一次性返回的数据帧总数
    Called by:  CCommHandle::CeiveTrans
*/
int AppPrtclPqB::CmdReceive(unsigned char *rbuf, unsigned char **tbuf, int *sz)
{
    unsigned int ui;
    unsigned long li;
    char syspara_update = 0;
    char tx_buf = 0; //发送缓存，0=m_tx_buffer,1=ptx_bufd;
    int reti, ack;
    unsigned char *puchr;

    memcpy(m_data_hd, rbuf, kDataHdSz);
    if (m_data_hd->compress) {
        li = 2048;
        reti = uncompress(m_rx_buffer, &li, rbuf + kDataHdSz, m_data_hd->body_len);
        if (reti) {
            *sz = 0;
            return 0;
        }
    } else {
        memcpy(m_rx_buffer, rbuf + kDataHdSz, m_data_hd->body_len);
    }
    m_data_hd->prtcl_type = 1;
    m_data_hd->prtcl_ver = 3; //协议版本为3
    m_data_hd->compress = 0;
    m_data_hd->compress_alg = 1;
    //m_data_hd->body_len = 0;

    m_data_hd->equip_type = prmcfg->pqm_type();
    m_data_hd->unit_scale = harmfunc->units_type(0) | (harmfunc->units_type(1) << 2) | (harmfunc->units_type(2) << 4);

    if (m_data_hd->window_sz > MaxWindowsSz) {
        m_data_hd->window_sz = MaxWindowsSz;
    } else if (m_data_hd->window_sz < 1) {
        m_data_hd->window_sz = 1;
    }
    puchr = m_rx_buffer;    //数据体
    bool cyc_1st = true;
    int k;
    if (prmcfg->security_en()==0) logged_ = true;
    if (!logged_) { //not logged
        switch(m_data_hd->cmd) {
            case CMD_SRCH:
            case CMD_RANDKEY:
            case CMD_IDENTIFY:
            case CMD_GETUSERS:
            case CMD_LOGIN:
            case CMD_SYSHIDE_PARAM:
                break;
            default:
                *sz = 0;
                return 0; //无需应答
        }
    }
    for (k = 0; k < m_data_hd->window_sz; k++) {
        ack = 1;
        switch(m_data_hd->cmd) {
            case CMD_SRCH:
                printf("cmd is CMD_SRCH\n");
                break;
            case CMD_ALM:
                //push_debug_info("Received alm cmd!");
                ui = pqmfunc->alarm_word();
                m_tx_buffer[kDataHdSz] = ui >> 8;
                m_tx_buffer[kDataHdSz + 1] = ui & 0xff;
                m_data_hd->body_len = 2;
                break;
            case CMD_REAL:
                //push_debug_info("Received real cmd!");
                m_data_hd->body_len = tx_real(puchr, 0);
                m_data_hd->compress = 1;
                tx_buf = 1;
                break;
            case CMD_TRANST:
                //printf("recieved CMD_TRANST %d\n", m_data_hd->cmd);
                m_data_hd->body_len = tx_trnst_rec(puchr);
                //printf("m_data_hd->body_len=%d\n", m_data_hd->body_len);
                //m_data_hd->compress = 1;//压缩率几乎为0
                break;
            case CMD_SETTM:
                set_time(puchr);
                ack = 0;
                break;
            case CMD_INI:
                m_data_hd->body_len = IniDev(puchr, &syspara_update);
                break;
            case CMD_FHSET:
                m_data_hd->body_len = tx_syspar(puchr);
                m_data_hd->compress = 1;
                break;
            case CMD_WTSET:
                m_data_hd->body_len = set_syspar(puchr, m_data_hd->body_len);
                syspara_update = 1;
                pee_warning->set_para_update(2);
                break;
		    case CMD_CUSTOM_LIMIT:
			    m_data_hd->body_len = custom_limit_handle(puchr);
                syspara_update = 1;
			    break;
            case CMD_SETDEVNUM:
            case CMD_SETBAUDRATE:
            case CMD_SAVE_SPACE:
            case CMD_IP_ADDR:
            case CMD_PORT_NUM:
            case CMD_SET_PROTOCOL:
            case CMD_SAVE_TYPE:
            case CMD_CONNECTION:
            case CMD_SYSHIDE_PARAM:
            case CMD_TIMESYN_PARAM:
                syspara_update = 1;
            case CMD_CAP_RA:
            case CMD_CAP_THR:
            case CMD_RA_PARAM:
            case CMD_TRNSF_PARAM:
            case CMD_DC_COMPONENT:
                m_data_hd->body_len = set_para(puchr, m_data_hd->body_len, m_data_hd->cmd);
                break;
            case CMD_SETDEVSN:
                m_data_hd->body_len = SetDeviceSn(puchr);
                break;
            case CMD_GET_AUDIT_LOG:
                m_data_hd->body_len = TxAuditLog(puchr);
                tx_buf = 1;
                m_data_hd->compress = 1;
                break;
            case CMD_RANDKEY:
                m_data_hd->body_len = TxRandkey();
                break;
            case CMD_IDENTIFY:
                m_data_hd->body_len = Identify(puchr);
                break;
            case CMD_GETUSERS:
                m_data_hd->body_len = GetUsers();
                break;
            case CMD_LOGIN:
                m_data_hd->body_len = Login(puchr);
                break;
            case CMD_CHGPASS:
                m_data_hd->body_len = ChgPasswd(puchr);
                break;
            case CMD_CVT_MODIFY:
                m_data_hd->body_len = cvt_modify_handle(puchr);
                break;
            case CMD_EQUIP_INF:
                m_data_hd->body_len = tx_smp_ver(puchr);
                break;
            case CMD_SMP_UPFILE:
                m_data_hd->body_len = tx_smp_data(puchr);
                break;
            case CMD_SMP_UPSTATE:
                m_data_hd->body_len = tx_smp_suc(puchr);
                break;
            case CMD_TRIG_ENABLE:
                m_data_hd->body_len = trigger_able(puchr);
                break;
            case CMD_ONOFF_TIME:
                m_data_hd->body_len = TxPower01Time(puchr);
                break;
            case CMD_FREQ:
            case CMD_UNBLC:
            case CMD_VOLTDV:
            case CMD_GET_Pst:
            case CMD_GET_Plt:
                m_data_hd->body_len = TxOtherRec(m_data_hd->cmd, puchr);
                //if (!m_data_hd->body_len) ack = 0;
                tx_buf = 1;
                if (m_data_hd->body_len) {
                    m_data_hd->compress = 1;
                } else {
                    m_data_hd->compress = 0;
                }
                break;
            case CMD_GET_HRM:
                m_data_hd->body_len = TxHarmRec(cyc_1st, puchr);
                //printf("m_data_hd->body_len=%d\n", m_data_hd->body_len);
                cyc_1st = false;
                if (m_data_hd->body_len) {
                    m_data_hd->compress = 1;
                    tx_buf = 1;
                } else {
                    m_data_hd->compress = 0;
                    tx_buf = 0;
                }
                break;
            case CMD_MAN_REC:
                m_data_hd->body_len = man_rec(puchr);
                break;
            case CMD_CAP_WARN:
            case CMD_TRNSF_WARN:
                m_data_hd->body_len = TxEewRec(m_data_hd->cmd, puchr);
                m_data_hd->compress = 1;
                tx_buf = 1;
                break;
            case CMD_QUIT:
                usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "Cold reboot", 1);
                ColdReboot();
              	break;
            case CMD_SSHD:
                //printf("CMD_SSHD:%d\n", puchr[0]);
                if (puchr[0]==1) {
                    //printf("start sshd!\n");
                    system("/etc/init.d/S50sshd start");
                    set_net_para(5, NULL);
                    usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "Start sshd", 1);
                } else {
                    //printf("stop sshd!\n");
                    system("/etc/init.d/S50sshd stop");
                    set_net_para(6, NULL);
                    usr_mng().SetAudit("setting", usr_mng().usr_name(user_id_), "Stop sshd", 1);
                }
                m_data_hd->body_len = 0; //can be commentated
              	break;
            default:
                m_data_hd->body_len = 0;
                ack = 0;
                break;
        }

        if (!ack) {
            *sz = 0;
            return 0; //无需应答
        }

        *sz = m_data_hd->body_len + kDataHdSz;
        tbuf[k] = new uchar[*sz];
        if (tx_buf) {
            memcpy(ptx_bufd, m_data_hd, kDataHdSz);
            memcpy(tbuf[k], ptx_bufd, *sz);
            delete [] ptx_bufd;
            ptx_bufd = NULL;
        } else {
            memcpy(m_tx_buffer, m_data_hd, kDataHdSz);
            memcpy(tbuf[k], m_tx_buffer, *sz);
        }
        sz++;
        m_data_hd->frm_sn++;
    }
    if (m_data_hd->cmd!=CMD_ALM) {
        timer_logout_->Start(prmcfg->lcm_dely_time()*60);
    }

    if (syspara_update) {
        if (syspara_update == 2) {
            notice_pthread(kTTDisplay, COMMINFO, INIT_UNIT, NULL);
        } else {
            notice_pthread(kTTSave, SAVEINFO, kSysParam, NULL);
        }
    }
    return k;
}

static const int RCD_WAITS = 10; //取记录结束等待次数
void AppPrtclPqB::PostProcess(void *data)
{
    //暂态记录
    if (transt_cnt++ == RCD_WAITS) { //等待超时
        if (f_transtu != NULL) {
            fclose(f_transtu);
            f_transtu = NULL;
            if (f_transti != NULL) fclose(f_transti);
            f_transti = NULL;
        }
    }

    if (logged_) {
        if (timer_logout_->TimeOut()) logged_ = false;
    }
}

//Description: 触发暂态手动录波命令
int AppPrtclPqB::man_rec(uchar *rbuf)
{
    if (prmcfg->manual_rec_enable()) {
        volt_variation->set_stdy_event(1);  //启动手动录波
    }
    m_tx_buffer[kDataHdSz] = volt_variation->manual_state();
    return 1;
}

/*!
Description:Send Electric Equipment Warning Record

    Input:  cmd -- CMD_CAP_WARN or CMD_TRNSF_WARN
            rbuf -- received buffer
*/
int AppPrtclPqB::TxEewRec(int cmd, uchar *rbuf)
{
    timeval stime;
    time_t etime;

    memcpy(&stime, rbuf, sizeof(timeval));
    memcpy(&etime, &rbuf[sizeof(timeval)], sizeof(time_t));
    char maxnum = rbuf[sizeof(timeval)+sizeof(time_t)];

    ptx_bufd = new uchar[kDataHdSz + 1 + maxnum * sizeof(EEWRecord)];
    int num;
    pthread_mutex_lock(&store_mutex);
    if (cmd == CMD_CAP_WARN) {
        num = pee_warning->ReadRcdFile((char*)&ptx_bufd[kDataHdSz + 1], &stime, etime, maxnum, kEEWRcd);
    } else {
        num = pee_warning->ReadRcdFile((char*)&ptx_bufd[kDataHdSz + 1], &stime, etime, maxnum, kEEWTRcd);
    }
    pthread_mutex_unlock(&store_mutex);
    if (!num) return 0;
    ptx_bufd[kDataHdSz] = num;

    tm tmi; char tm_str[24];
    GmTime(&tmi, &stime.tv_sec);
    strftime(tm_str, 20, "%Y-%m-%d_%H:%M:%S", &tmi);
    //printf("%s\tmaxnum=%d, num=%d\n", tm_str, maxnum, num);

    return (num * sizeof(EEWRecord) + 1);
}

/*!
Description:Send Electric Equipment Warning Record

    Input:  cmd -- CMD_CAP_WARN or CMD_TRNSF_WARN
            rbuf -- received buffer
*/
int AppPrtclPqB::TxOtherRec(int cmd, uchar *rbuf)
{
    time_t stim, etim;

    memcpy(&stim, rbuf, sizeof(time_t));
    memcpy(&etim, &rbuf[sizeof(time_t)], sizeof(time_t));
    char maxnum = rbuf[sizeof(time_t)*2];
#if 0       //for debug
    tm tmi; char tm_str[24]; 
    GmTime(&tmi, &stim);
    strftime(tm_str, 20, "%Y-%m-%d_%H:%M:%S", &tmi);
    printf("%s ~ ", tm_str);
    GmTime(&tmi,  &etim);
    strftime(tm_str, 20, "%Y-%m-%d_%H:%M:%S", &tmi);
    printf("%s\n", tm_str);
#endif

    SaveFileType type;
    switch (cmd) {
        case CMD_GET_Pst:
        case CMD_GET_Plt:
            type = kPstSave;
            break;
        case CMD_FREQ:
            type = kFreqSave;
            break;
        case CMD_UNBLC:
            type = kUnblcSave;
            break;
        case CMD_VOLTDV:
            type = kVoltdvSave;
            break;
        default:
            return 0;
    }
    int recsz = sizeof(time_t) + other_func->get_save_size(type, 1);
    //int sz = kDataHdSz + 4 + (!maxnum) ? recsz * 255 : recsz * maxnum;
    ptx_bufd = new uchar[kDataHdSz + 4 + ((!maxnum) ? recsz * 255 : recsz * maxnum)];
    int num;
    pthread_mutex_lock(&store_mutex);
    if (cmd == CMD_GET_Plt) {
        num = save_func->ReadPltData((char*)&ptx_bufd[kDataHdSz+1], &stim, &etim, maxnum);
    } else {
        num = save_func->ReadRecData((char*)&ptx_bufd[kDataHdSz+1], &stim, &etim, maxnum, type);
    }
    pthread_mutex_unlock(&store_mutex);
    ptx_bufd[kDataHdSz] = num;

#if 0       //for debug
    printf("maxnum=%d, num=%d\n\n", maxnum, num);
#endif

    if (!num) return 0;
    return (num * recsz + 4);
}

/*!
Description:Send Harmonic Record

    Input:  first -- cycle first
            rbuf -- received buffer
    Return: size of data body
*/
int AppPrtclPqB::TxHarmRec(bool first, uchar *rbuf)
{
    //printf("received CMD_GET_HRM cmd\n");
    if (first) {
        time_t stim, etim;
        memcpy(&stim, rbuf, sizeof(time_t));
        memcpy(&etim, &rbuf[sizeof(time_t)], sizeof(time_t));
        pthread_mutex_lock(&store_mutex);
        int tol = save_func->ReadHarms2Buf(stim, etim);
        pthread_mutex_unlock(&store_mutex);
        m_data_hd->window_sz = tol;
        if(m_data_hd->window_sz<=0) return 0;
    }
    //printf("m_data_hd->window_sz=%d\n", m_data_hd->window_sz);
    unsigned char *pch;
    int recsz = save_func->GetHarmFrmBuf(&pch);
    
    ptx_bufd = new uchar[kDataHdSz + recsz];
    memcpy(&ptx_bufd[kDataHdSz], pch, recsz);
    delete [] pch;
    return recsz;
}

