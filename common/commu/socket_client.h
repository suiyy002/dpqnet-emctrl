#ifndef _SOCKET_CLIENT_H_
#define _SOCKET_CLIENT_H_

#include <stdint.h>
#include "socket_base.h"

struct ClientInfo {
    int phy_prtcl;     //physical layer protocol
    int app_prtcl;     //application layer protocol
    char name[64];
    char hostname[128];
    int recnct_intvl;    //auto-reconnect interval time. unit:s, 0 = not auto-reconnect 
    
    class TimerCstm *tmr_rcnct_;  //customize timer for auto-reconnect
    int sock_sn;   //sn for this socket
};

class SocketClient:public SocketBase {
public:
    SocketClient(int max);
    ~SocketClient();
    
    int Run(int timeout);
    int Start(const char *name, const char *hostname, int phy_t, int app_t, int intvl);
    void End(int idx);

protected:

private:
    int Connect (const char *name, const char *hostname);
    int InitSocket (struct sockaddr_in *name, const char *hostname, uint16_t port);
    int InitSocket (struct sockaddr_un *name, const char *filename);
    int FindMinSn();
    void Resume(int idx);

    ClientInfo * clnt_inf_;
    int *recnct_intvl_;  //auto-reconnect interval time. unit:s, 0 = not auto-reconnect
    int *sock_sn_;   //sn for per sock
    int sn_acc_;    //sn accumulater
    int data_idx_;  //index of data buffer
};

#endif	//_SOCKET_CLIENT_H_
