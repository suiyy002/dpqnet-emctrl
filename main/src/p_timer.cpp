#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "Version.h"
#include "base_func/conversion.h"
#include "thread/pthread_mng.h"
#include "device/device.h"
#include "pqm_func/pqmfunc.h"
#include "pqm_func/save_func.h"
#include "IPC/shmemfunc.h"
#include "pqm_func/volt_variation.h"
#include <time.h>

void *thread_timer(void *myarg)
{
    CleanupNode *pthnode = (CleanupNode *) myarg;
    printf("timer thread run...\n");
    sleep(3);
    if (Active_Watchdog) {
        if (OpenWatchdog() < 0) {
            printf("Can't open watchdog!!\n");
            exit(-1);
        }
        printf("Open watchdog\n");
        ClearWatchdog();
    } else {
        CloseWatchdog();
    }
    
    int cnt_dot1s = 0;  //0.1s per one
    for (;;) {
        msSleep(100);
        shmem_func().TreatShareCmd();
        cnt_dot1s++;
        int minor_type = 0;
        if (cnt_dot1s%5 == 0) { //0.5s
            if (cwq.control.active == QUITCMD) break;
        }
        if (cnt_dot1s%10 == 0) { //1s
            pqnet_ied().Soe2Shm();
            minor_type |= kOneSecond;
        }
        if (cnt_dot1s%30 == 0) { //3s
            minor_type |= kThreeSecond;
        }
        if (cnt_dot1s%100 == 0) { //10s
            minor_type |= kTenSecond;
        }
        if (cnt_dot1s%600 == 0) { //1minute
            cnt_dot1s = 0;
        }
    }
    notice_clrq(pthnode);
    return NULL;
}

