#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "messageq_guic.h"
#include "app_prtcl_guic.h"
#include "phy_prtcl_pqb.h"
#include "data_buffer.h"

/*!
    Input:  idx -- index of communication object
*/
AppPrtclGuiC::AppPrtclGuiC(int idx)
{
    co_idx_ = messageq_guic().BindQueue(idx);
    if (co_idx_==idx) {
        if (g_data_buf[idx]) {
            messageq_guic().FreeQueue(idx);
            co_idx_ = -1;
        } else {
            g_data_buf[idx] = new DataBuf(kChannelTol);
        }
    }
    memset(send_cmdbuf_, 0, sizeof(send_cmdbuf_));

    pthread_mutex_init(&mutex_, NULL);
}

AppPrtclGuiC::~AppPrtclGuiC()
{
    pthread_mutex_destroy(&mutex_);
    
    if (co_idx_<0) return;
    messageq_guic().FreeQueue(co_idx_);
    
    delete g_data_buf[co_idx_];
    g_data_buf[co_idx_] = NULL;
}

/*!
Communication command handle

    Input:  rbuf -- received data
    Output: tbuf -- response data
            sz -- size of reponse data
    Return: quantity of packages be send
    Called by:  CommuDisptchr::CeiveTrans
*/
int AppPrtclGuiC::CmdReceive(uint8_t *rbuf, unsigned char **tbuf, int *sz)
{
    if (co_idx_<0) return 0;
    CommDataHead head;
    int hdsz = sizeof(head);
    memcpy(&head, rbuf, hdsz);
    if (head.compress) {
        uLongf li = 2048;
        int k = uncompress(rx_buffer_, &li, rbuf + hdsz, head.body_len);
        if (k) {
            *sz = 0;
            return 0;
        }
    } else {
        memcpy(rx_buffer_, rbuf + hdsz, head.body_len);
    }
    IniHead(&head);

    int rspn = 0;   //response
    g_data_buf[co_idx_].SetData(co_idx_, rx_buffer_, head.cmd);
    switch(head.cmd) {
        case kCmdPhdPrm:
        case kCmdPrmLd:
        case kCmdPrmChnl:
            break;
        default:
            notice_pthread(kTTMain, kGUICommuCmd, head.cmd, NULL);
            break;
    }
    if (head.cmd<kGuiCmdC2SEnd) ClearSendbuf(head.frm_sn);
    
    int retv = 0;
    if (!rspn) {
        *sz = 0;
    } else {
        *sz = head.body_len + hdsz;
        tbuf[0] = new uint8_t[*sz];
        memcpy(tbuf[0], &head, hdsz);
        memcpy(&tbuf[0][hdsz], tx_buffer_, head.body_len);
        retv = 1;
    }
    return retv;
}

/*!
Transmit command handle. should be called per 0.1s

    Output: tbuf -- response data
            size -- size of reponse data
            mark -- if data be send in last CmdReceive(). no use
    Return: quantity of packages be send
    Called by:  CommuDisptchr::Transmit
*/
int AppPrtclGuiC::CmdSend(unsigned char **tbuf, int *size, int mark)
{
    if(co_idx_<0) return 0;
    CommDataHead head;
    memset(&head, 0, sizeof(head));
    IniHead(&head);
    
    WorkNode *cmd_node;
    int k = 0;
    uint8_t *buf = tx_buffer_;
    do {
        cmd_node = messageq_guic().Pop(co_idx_);
        if (!cmd_node) break;

        int major_t = cmd_node->major_type;
        int minor_t = cmd_node->minor_type;
        void *point = cmd_node->point;
        free(cmd_node);

        head.cmd = minor_t;
        if (point) {
            int sz = *(uint32_t*)point;
            head.body_len = sz;
            memcpy(tx_buffer_, &((char*)point)[4], sz);
        }
        int id = GetFrameID();
        send_cmdbuf_[id].cmd = minor_t;
        send_cmdbuf_[id].data = point;
        send_cmdbuf_[id].cnt = 3;
        messageq_guic().ClearWaitCnt(co_idx_, id);
        head.frm_sn = id;

        *size = head.body_len + sizeof(head);
        tbuf[k] = new uchar[*size];
        memcpy(tbuf[k], &head, sizeof(head));
        memcpy(&tbuf[k][sizeof(head)], tx_buffer_, head.body_len);
        size++;
    } while(++k<2);
    
    for (int i=0; i<kCmdBufNum; i++) { //resend cmd if response not be received
        if (send_cmdbuf_[i].cmd) {
            if (messageq_guic().wait_cnt(co_idx_, i)>3) {
                if (send_cmdbuf_[i].cnt) {
                    messageq_guic().PushQueue(co_idx_, kGUICommuCmd, send_cmdbuf_[i].cmd, send_cmdbuf_[i].data);
                    messageq_guic().ClearWaitCnt(co_idx_, i);
                    send_cmdbuf_[i].cnt--;
                } else {
                    ClearSendbuf(i);
                }
            }
        }
    }
    return k;
}

/*!
Clear send_cmdbuf_.

    Input:  sn -- frame sn
*/
void AppPrtclGuiC::ClearSendbuf(int sn)
{
    if (send_cmdbuf_[sn].data) delete [] send_cmdbuf_[sn].data;
    memset(&send_cmdbuf_[sn], 0, sizeof(SendCmdBuf));
}

/*!
Get idle frame id.

    Return:  frame id
*/
uint8_t AppPrtclGuiC::GetFrameID()
{
    uint8_t i;
    for (i=0; i<kCmdBufNum; i++) {
        if (send_cmdbuf_[i].cmd==0) break;
    }
    if(i>=kCmdBufNum) i = 0;
    return i;
}