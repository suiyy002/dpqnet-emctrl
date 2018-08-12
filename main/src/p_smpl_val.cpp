#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <cmath>
#include "smpl_val.h"
#include "thread/pthread_mng.h"
#include "pqm_func/pqmfunc.h"
#include "pqm_func/prmconfig.h"
#include "pqm_func/volt_variation.h"
#include "EEW/ee_warning.h"

//-----------------------------------------------------------------------------
void *thread_smpl_val(void *myarg)
{
    WorkNode *tomain;
    WorkNode *todis;
    CleanupNode *pthnode;
    unsigned int flag;
    unsigned short buf[100];

    pthnode = (CleanupNode *) myarg;

    printf("smpl_val thread run...\n");
    for (;;) {
        p_smplv_cnt++; //增加本线程监视计数
        pqnet_ied().ReadSV(200);

        if (g_mainq.control.active == QUITCMD) break;
    }

    notice_clrq(pthnode);
    return NULL;
}



