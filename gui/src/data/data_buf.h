/*! \file buffer_pool.h
    \brief display data buffer pool.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _DATA_BUF_H_
#define _DATA_BUF_H_

#include "data_buffer.h"

class DataBuf
{
public:
	DataBuf(int num);
	~DataBuf();

    void SetData(uint8_t *buf, uint8_t type);
    
    //Accasors
    int chl_match() { return chl_match_; };
protected:

private:
    void SetParm(uint8_t cmd, uint8_t *buf);
    void SetFreq(uint8_t *buf);

    DataBufLd * data_ld_;
    DataBufChnl * data_chnl_;
    ParamLD * para_ld_;
    ParamChnl * para_chnl_;
    ParamPHD para_phd_;
    
    pthread_mutex_t mutex_;
    int chl_match_; //channel type match error. i.e. the type be detected is not identical with the type be set.
                    //0=matched. 1=not match
};

extern DataBuf **g_data_buf;

#endif // _DATA_BUF_H_
