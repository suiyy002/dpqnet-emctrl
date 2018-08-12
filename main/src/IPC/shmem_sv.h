#ifndef _SHMEM_SV_H_
#define _SHMEM_SV_H_
#include "share_mem.h"
#include "../pqm_func/prmconfig.h"
#include <pthread.h>
#include<cstdio>
#include<cstdlib>
#include<cstring>

enum ShmemDataType {
	kGetSVbuf
};

class ShmemSV
{
public:
    ShmemSV();
    ~ShmemSV();
    
    viod *ReadSv1sBlock();
    set_rdata_cnt() { pshm_data_->rdata_cnt = pshm_data_->wdata_cnt; };
    
protected:

private:
    ShmemSV61850 *pshmem_;  //pointer to share memory
    int lost_packets_;      //total lost packets from power on.

};

ShmemSV &shmem_sv();

#endif //_SHMEM_SV_H_


