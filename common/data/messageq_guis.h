/*! \file gui_msg_queue.h
    \brief GUI message queue.
    Copyright (c) 2017  Xi'an Boyuu Electric, Inc.
*/
#ifndef _MESSAGEQ_GUIS_H_
#define _MESSAGEQ_GUIS_H_

#include <stdint.h>
#include "pthread_mng.h"
#include "message_queue.h"

class MessageQGuiS:public MessageQueue {
  public:
	MessageQGuiS(){};
	~MessageQGuiS();
    void InitQGui(int max);

    int Push(int type, uint32_t size=0, void *data=NULL, uint32_t *ld_idx=NULL);
	
  protected:

  private:
};

MessageQGuiS & messageq_guis();

#endif // _MESSAGEQ_GUIS_H_ 
