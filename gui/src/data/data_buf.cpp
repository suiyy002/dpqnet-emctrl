#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "data_buffer.h"
#include "pthread_mng.h"
#include "gui_msg_queue.h"
#include "time_cst.h"
#include "md5.h"
#include "../Version.h"

DataBuf **g_data_buf = NULL;

/*!
Input:  num -- number of channel
*/
DataBuf::DataBuf(int num)
{
    pthread_mutex_init (&mutex_, NULL);

    data_ld_ = new DataBufLd[num];
    memset(data_ld_, 0, sizeof(DataBufLd)*num);
    data_chnl_ = new DataBufChnl[num];
    memset(data_chnl_, 0, sizeof(DataBufChnl)*num);

    memset(&para_phd_, 0, sizeof(para_phd_));
    para_ld_ = new ParamLD[num];
    memset(para_ld_, 0, sizeof(ParamLD)*num);
    para_chnl_ = new ParamChnl[num];
    memset(para_chnl_, 0, sizeof(ParamChnl)*num);
}

DataBuf::~DataBuf()
{
    delete [] para_chnl_;
    delete [] para_ld_;
    delete [] data_chnl_;
    delete [] data_ld_;

    pthread_mutex_destroy(&mutex_);
}

/*!
    Input:  idx -- connect index
            buf
            type -- kGuiCmd
*/
void DataBuf::SetData(uint8_t *buf, uint8_t type)
{
    pthread_mutex_lock(&mutex_);
    switch (type) {
        case kCmdPrmPhd:
        case kCmdPrmLd:
        case kCmdPrmChnl:
            SetParm(type, buf);
            break;
        case kCmdWarnChlMatch:
            SetChlWarn(buf);
            break;
        case kCmdFrequency:
            SetFreq(idx, buf);
            break;
        default:
            break;
    }
    pthread_mutex_unlock(&mutex_);
}

/*!
    Input:  buf -- 
*/
void DataBuf::SetFreq(uint8_t *buf)
{
    uint8_t ld = *buf;
    memcpy(&data_ld_[ld].frequency_, &buf[1], sizeof(frequency_));
}

/*!
    Input:  buf -- 
*/
void DataBuf::SetChlWarn(uint8_t *buf)
{
    for (int i=0; i<kChannelTol; i++) {
        data_chnl_[i].type = buf[i];
    }
    chl_match_ = 1;
}

/*!
Set parameter

    Input:  cmd -- refer to kGuiCommand
            buf --
            
*/
void DataBuf::SetParm(uint8_t cmd, uint8_t *buf)
{
    uint8_t hv = *buf;
    if (hv==0) return;
    uint8_t i;
    switch(cmd) {
        case kCmdPrmPhd:
            memcpy(&para_phd_, buf, sizeof(ParamPHD));
            break;
        case kCmdPrmLd:
            memcpy(&para_ld_[buf[1]], &buf[1], sizeof(ParamLD));
            break;
        case kCmdPrmChnl:
            memcpy(&para_chnl_[buf[1]], &buf[1], sizeof(ParamChnl));
        default:    break;
    }
}

/*!
Request all parameter

    Input:  idx -- connect index
            buf --
            
*/
void DataBuf::RequestParm(uint8_t cmd, uint8_t *buf)
{
    uint8_t hv = *buf;
    if (hv==0) return;
    uint8_t i;
    switch(cmd) {
        case kCmdPrmPhd:
            memcpy(&para_phd_[idx], buf, sizeof(ParamPHD));
            break;
        case kCmdPrmLd:
            memcpy(&para_ld_[idx][buf[1]], &buf[1], sizeof(ParamLD));
            break;
        case kCmdPrmChnl:
            memcpy(&para_chnl_[idx][buf[1]], &buf[1], sizeof(ParamChnl));
        default:    break;
    }
}
