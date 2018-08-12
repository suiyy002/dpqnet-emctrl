/*! \file messageq_guic.h
    \brief GUI client message queue.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _MESSAGEQ_GUIC_H_
#define _MESSAGEQ_GUIC_H_

#include <stdint.h>
#include "pthread_mng.h"
#include "message_queue.h"

static const int kCmdBufNum = 64;

class MessageQGuiC:public MessageQueue {
  public:
	MessageQGuiC(){};
	~MessageQGuiC();
    void InitQGui(int max);

    int Push(uint8_t qidx, int type, uint32_t size, void *data, uint8_t *idx=NULL);
    void PushCtrlSig(uint8_t qidx, uint16_t type, uint16_t val1, uint16_t val2=0);
    void FetchParam();
    void IncreaseCount();
    void ClearWaitCnt(int idx, int sn);
	
    //Accessors
    uint8_t wait_cnt(int idx, int sn) { return wait_cnt_[idx][sn]; };
  protected:
    
  private:
    uint8_t **wait_cnt_;    //waiting counter
};

MessageQGuiC & messageq_gui();

#endif // _MESSAGEQ_GUIC_H_ 
