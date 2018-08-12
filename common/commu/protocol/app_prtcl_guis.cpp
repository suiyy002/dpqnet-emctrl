#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "app_prtcl_guis.h"
#include "../../pqm_func/prmconfig.h"
#include "../../device/device.h"
#include "../../base_func/conversion.h"
#include "../../base_func/time_cst.h"

/*!
    Input:  idx -- index of communication object
*/
AppPrtclGuiS::AppPrtclGuiS(int idx)
{
    co_idx_ = messageq_guis().BindQueue(idx);
    memset(send_cmdbuf_, 0, sizeof(send_cmdbuf_));

    ldprm_chg_ = new int[kChannelTol];
    chnlprm_chg_ = new int[kChannelTol];
    phdprm_chg_ = 0;
    memset(ldprm_chg_, 0, sizeof(int)*kChannelTol);
    memset(chnlprm_chg_, 0, sizeof(int)*kChannelTol);
    
}

AppPrtclGuiS::~AppPrtclGuiS()
{
    delete [] chnlprm_chg_;
    delete [] ldprm_chg_;
    
    if (co_idx_<0) return;
    messageq_guis().FreeQueue(co_idx_);
}

/*!
Communication command handle

    Input:  rbuf -- received data
    Output: tbuf -- response data
            sz -- size of reponse data
    Return: quantity of packages be send
    Called by:  CommuDisptchr::CeiveTrans
*/
int AppPrtclGuiS::CmdReceive(unsigned char *rbuf, unsigned char **tbuf, int *sz)
{
    CommDataHead head;
    int hdsz = sizeof(head);
    memcpy(&head, rbuf, hdsz);
    if (head.compress) {
        int li = 2048;
        reti = uncompress(rx_buffer_, &li, rbuf + hdsz, head.body_len);
        if (reti) {
            *sz = 0;
            return 0;
        }
    } else {
        memcpy(rx_buffer_, rbuf + hdsz, head.body_len);
    }
    IniHead(&head);

    puchr = rx_buffer_; //data body

    int ack = 1;
    switch(head.cmd) {
        case kCmdPrmPhd:
        case kCmdPrmLd:
        case kCmdPrmChnl:
            m_data_hd->body_len = HandleParm(tx_buffer_, rx_buffer_, head.cmd);
            break;
        default:
            m_data_hd->body_len = 0;
            ack = 0;
            break;
    }
    
    if (!ack) {
        *sz = 0;
        return 0; //don't need response
    }
    
    *sz = head.body_len + hdsz;
    tbuf[0] = new uchar[*sz];
    memcpy(tbuf[0], &head, hdsz));
    memcpy(&tbuf[0][hdsz], tx_buffer_, head.body_len);
    return 1;
}


/*!
Transmit command handle. should be called per 0.1s

    Output: tbuf -- response data
            size -- size of reponse data
            mark -- if data be send in last CmdReceive(). no use
    Return: quantity of packages be send
    Called by:  CommuDisptchr::Transmit
*/
int AppPrtclGuiS::CmdSend(unsigned char **tbuf, int *size, int mark)
{
    if(co_idx_<0) return 0;
    CommDataHead head;
    memset(head, 0, sizeof(head));
    IniHead(&head);
    
    WorkNode *cmd_node;
    int k = 0;
    
    do {
        cmd_node = messageq_guis().Pop(co_idx_);
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
            delete [] point;
        }
        *size = head.body_len + sizeof(head);
        tbuf[k] = new uchar[*size];
        memcpy(tbuf[k], &head, sizeof(head));
        memcpy(&tbuf[k][sizeof(head)], tx_buffer_, head.body_len);
        size++;
    } while(++k<2);
    return k;
}

/*!
handle parameter

    Input:  rx_buf
    Output: tx_buf
    Return: size of tx_buf
*/
int AppPrtclGuiS::HandleParm(uint8_t *tx_buf, uint8_t *rx_buf, uint8_t cmd)
{
    uint8_t gs = *rx_buf;
    uint8_t idx;
    if (gs) {   //set
        switch (cmd) {
            case kCmdPrmPhd:
                phy_dev().SetParm(&phdprm_chg_, &rx_buf[1]);
                break;
            case kCmdPrmLd:
                idx = rx_buf[1];
                pqnet_ied().set_prm_ld(idx, &ldprm_chg_[idx], &rx_buf[2]);
                break;
            case kCmdPrmChnl:
                idx = rx_buf[1];
                pqnet_ied().set_prm_chnl(idx, &chnlprm_chg_[idx], &rx_buf[2]);
            default: break;
        }
    }
    uint8_t *parm = NULL;
    int sz;
    switch (cmd) {
        case kCmdPrmPhd:
            parm = phy_dev().GetParm(&phdprm_chg_);
            sz = sizeof(ParamPHD);
            break;
        case kCmdPrmLd:
            parm = pqnet_ied().prm_ld(idx, &lddprm_chg_[idx]);
            sz = sizeof(ParamLD);
            break;
        case kCmdPrmChnl:
            parm = pqnet_ied().prm_chnl(idx, &chnlprm_chg_[idx]);
            sz = sizeof(ParamChnl);
        default: break;
    }
    if (parm) {
        tx_buf[0] = 1;
        memcpy(&tx_buf[1], parm, sz);
        return sz+1;
    } else {
        rx_buf[0] = 0;
        return 1;
    }
}


