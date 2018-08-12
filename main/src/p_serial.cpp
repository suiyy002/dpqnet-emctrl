#include <cstdio>
//#include <cstdlib>
//#include <sys/ioctl.h>
//#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include "thread/pthread_mng.h"
#include "commu/serial_handle.h"
#include "pqm_func/prmconfig.h"
#include "GUI/view.h"
#include "device/device.h"

#define OPENED_MAX 5 //最多同时打开5个串口
void *thread_serial(void *myarg)
{
    CleanupNode *pthnode;

    printf("serial thread run...\n");
    pthnode = (CleanupNode *) myarg;
    
    SerialHandle *serial_hdl = new SerialHandle(OPENED_MAX);
    int baud = prmcfg->sys_para_sg()->BaudRate[0];
    int prtcl = 0;
    serial_hdl->CreateCommuObj(SRL_PORT_SNUM, baud, prtcl, prtcl);   //PQ data port. protocol Custom defined B
    baud = prmcfg->sys_para_sg()->BaudRate[1];
    serial_hdl->CreateCommuObj(SRL_PORT_SNUM+1, baud, 1, 1);   //GPS timesync port. protocol for GPS IRIG-B

    for ( ; ; ) {
        serial_hdl->Run(2000);
        if (cwq.control.active == QUITCMD) break;
        p_comm_cnt++;//串口接收数据计数累加

        if (prmcfg->get_update(BaudrateUpdate)) {
            baud = prmcfg->sys_para_sg()->BaudRate[0];
            serial_hdl->CreateCommuObj(SRL_PORT_SNUM, baud, prtcl, prtcl);   //PQ data port. protocol Custom defined B
        }
        if(prmcfg->get_update(SerialPrtclUpdate)) {
            int k = prtcl;
            prtcl = prmcfg->comm_protocol(0);
            if (k != prtcl) {
                serial_hdl->CreateCommuObj(SRL_PORT_SNUM, baud, prtcl, prtcl);   //PQ data port. protocol Custom defined B
            }
        }
    }
    delete serial_hdl;
    notice_clrq(pthnode);
    return NULL;
}

