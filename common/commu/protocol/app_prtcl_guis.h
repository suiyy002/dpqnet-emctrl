/*! \file app_prtcl_guis.h
    \brief Applicate layer type GUI server protocol class.
    Copyright (c) 2017  Xi'an Boyuu Electric, Inc.
*/
#ifndef _APP_PRTCL_GUIS_H_
#define _APP_PRTCL_GUIS_H_

#include "app_prtcl.h"
#include "app_prtcl_gui.h"

class AppPrtclGuiS:public AppPrtclBase, public AppPrtclGui
{
public:
	AppPrtclGuiS();
	~AppPrtclGuiS();
	
	int CmdReceive(unsigned char *rbuf, unsigned char **tbuf, int *sz);
	void PostProcess(void *data) {};
    int CmdSend(unsigned char **tbuf, int *sz, int mark);
protected:

private:
    int HandleParm(uint8_t *tx_buf, uint8_t *rx_buf, uint8_t cmd);

    int co_idx_;   //index of communication object
    int phdprm_chg_;    //physical device parameter changed count
    int *ldprm_chg_;    //physical device parameter changed count
    int *chnlprm_chg_;    //physical device parameter changed count
    
	uint8_t tx_buffer_[1024];  //transmit buffer
	uint8_t rx_buffer_[1024];  //receive buffer 
};


#endif // _APP_PRTCL_GUIS_H_ 
