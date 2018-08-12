/*! \file app_prtcl_guis.h
    \brief Applicate layer type GUI server protocol class.
    Copyright (c) 2017  Xi'an Boyuu Electric, Inc.
*/
#ifndef _APP_PRTCL_GUIC_H_
#define _APP_PRTCL_GUIC_H_

#include <stdint.h>
#include <pthread.h>
#include "app_prtcl.h"
#include "app_prtcl_gui.h"
#include "message_gui.h"

class AppPrtclGuiC:public AppPrtclBase, public AppPrtclGui
{
public:
	AppPrtclGuiC(int idx);
	~AppPrtclGuiC();
	
	int CmdReceive(unsigned char *rbuf, unsigned char **tbuf, int *sz);
	void PostProcess(void *data) {};
    int CmdSend(unsigned char **tbuf, int *sz, int mark);
protected:

private:
    void ClearSendbuf(int id);
    uint8_t GetFrameID();
    
    int co_idx_;   //index of communication object
    
	uint8_t tx_buffer_[1024];  //transmit buffer
	uint8_t rx_buffer_[1024];  //receive buffer 
	
    pthread_mutex_t mutex_;
    struct SendCmdBuf { //send command buffer. be used resend when command send failure
        uint8_t cmd;
        uint8_t *data;
        uint8_t cnt;    //resend count.
    } send_cmdbuf_[kCmdBufNum];
};

#endif // _APP_PRTCL_GUIC_H_ 
