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
#include "pthread_mng.h"
#include "socket_client.h"

extern SocketClient *g_sock_client;

void *thread_socket(void *myarg)
{
    CleanupNode *pthnode;

    printf("socket thread run...\n");
    pthnode = (CleanupNode *) myarg;

    for ( ; ; ) {
        g_sock_client->Run(100);
        if (cwq.control.active == QUITCMD) break;
    }
    notice_clrq(pthnode);
    return NULL;
}
