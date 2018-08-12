#include <cstdio>
#include <cstdlib>
#include <cstring>
//#include <sys/ioctl.h>
//#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "thread/pthread_mng.h"
#include "commu/socket_server.h"
#include "pqm_func/prmconfig.h"
#include "GUI/view.h"

#define CNNCT_MAX 5 //最多建立5个socket连接
void *thread_socket(void *myarg)
{
    CleanupNode *pthnode;

    printf("socket thread run...\n");
    pthnode = (CleanupNode *) myarg;

    char stri[48];
    sprintf(stri, "%d", prmcfg->socket_server_port());
    SocketServer *sock_server = new SocketServer(CNNCT_MAX, stri);
    int prtcl = prmcfg->comm_protocol(1);
    sock_server->set_phy_prtcl(prtcl);
    sock_server->set_app_prtcl(prtcl);
    
    for ( ; ; ) {
        if (sock_server->Run(2000)<0) {
            printf("start socket server failure!\n");
            break;
        }
        if (cwq.control.active == QUITCMD) break;
        p_socket_cnt++;

        if (prmcfg->get_update(SocketPrtclUpdate)) {
            int k = prtcl;
            prtcl = prmcfg->comm_protocol(1);
            if (k != prtcl) {
                sock_server->set_phy_prtcl(prtcl);
                sock_server->set_app_prtcl(prtcl);
            }
        }
        if (prmcfg->get_update(SocketPortIPUpdate)) {
            delete sock_server;
            sscanf(stri, "%d", prmcfg->socket_server_port());
            sock_server = new SocketServer(CNNCT_MAX, stri);
            sock_server->set_phy_prtcl(prtcl);
            sock_server->set_app_prtcl(prtcl);
        }
    }
    delete sock_server;
    notice_clrq(pthnode);
    return NULL;
}

/*!
socket for display data communication
*/
void *thread_socket_dis(void *myarg)
{
    CleanupNode *pthnode;

    printf("display socket thread run...\n");
    pthnode = (CleanupNode *) myarg;

    messageq_guis().InitQGui(CNNCT_MAX);
    char *name = "/tmp/sockgui";
    SocketServer *sock_server = new SocketServer(CNNCT_MAX, name);
    sock_server->set_phy_prtcl(2);
    sock_server->set_app_prtcl(2);
    
    for ( ; ; ) {
        if (sock_server->Run(2000)<0) {
            printf("start socket server failure!\n");
            break;
        }
        if (cwq.control.active == QUITCMD) break;
        p_sockdis_cnt++;

    }
    delete sock_server;
    notice_clrq(pthnode);
    return NULL;
}
