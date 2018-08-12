#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
//#include "Version.h"
#include "pthread_mng.h"
//#include "device/device.h"
#include "time_cst.h"
#include "gui_msg_queue.h"

void *thread_timer(void *myarg)
{
    CleanupNode *pthnode = (CleanupNode *) myarg;
    printf("timer thread run...\n");

    int cnt_dot1s = 0;  //0.1s per one
    for (;;) {
        msSleep(100);
        cnt_dot1s++;
        int minor_type = 0;
        if (cnt_dot1s%5 == 0) { //0.5s
            if (cwq.control.active == QUITCMD) break;
        }
        if (cnt_dot1s%10 == 0) { //1s
            messageq_guic().IncreaseCount();
            minor_type |= kOneSecond;
        }
        if (cnt_dot1s%30 == 0) { //3s
            minor_type |= kThreeSecond;
        }
        if (cnt_dot1s%100 == 0) { //10s
            messageq_guic().FetchParam();
            minor_type |= kTenSecond;
        }
        if (cnt_dot1s%600 == 0) { //1minute
            minor_type |= kOneMinute;
            cnt_dot1s = 0;
        }
        notice_pthread(kTTMain, kPTimerInfo, minor_type, NULL);
    }
    notice_clrq(pthnode);
    return NULL;
}

