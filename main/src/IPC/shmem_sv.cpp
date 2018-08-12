#include<cmath>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include "shmemfunc.h"
#include "../pqm_func/pqmfunc.h"
#include "../pqm_func/volt_variation.h"
#include "../GUI/view.h"
#include "../comm/protocol/AppPrtclB.h"

using namespace std;

#define SQRT30 sqrt(3.0)
#define NORMAL_RMS_U (syspara->line_para.loopPar[0].IT[1]/SQRT30)
#define NORMAL_RMS_I (syspara->line_para.loopPar[1].IT[1]/SQRT30)

ShmemSV &shmem_sv()
{
	static ShmemSV psv;
	return psv;
};

ShmemSV::ShmemSV()
{
    pshmem_ = InitShmSV61850(); //Create share memory and initialize.
    lost_packets_ = 0;
}

ShmemSV::~ShmemSV()
{
    FreeShareMemmory(pshmem_);
    pthread_mutex_destroy(&shm_mutex_); 
}

/*!
Read 1s block sv data from sharemem

    Return: point to 1s blcok sv in sharemem or NULL
*/
viod *ShmemSV::ReadSv1sBlock()
{
    if (pshmem_->wdata_cnt!=pshmem_->rdata_cnt) {
        int i = pshmem_->wdata_cnt-pshmem_->rdata_cnt-1; 
        if (i>0) lost_packets_ += i;
        return &pshmem_->sv_1s_block;
    } else {
        return NULL;
    }
}


