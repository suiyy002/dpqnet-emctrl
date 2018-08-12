#ifndef _SOCKET_SERVER_H_
#define _SOCKET_SERVER_H_

#include <stdint.h>
#include "socket_base.h"

class SocketServer:public SocketBase {
public:
    SocketServer(int max, const char *name);
    ~SocketServer();
    
    int Run(int timeout);
    
    void set_phy_prtcl( int val ) { phy_prtcl_ = val; };
    void set_app_prtcl( int val ) { app_prtcl_ = val; };
protected:
private:
    int Accept(int socket, int type);
    int MakeSocket(uint16_t port);
    int MakeSocket(const char *filename);
    int Start (const char *name);

    class TimerCstm *timer_idle_;  //idle timer
    int phy_prtcl_;     //physical layer protocol
    int app_prtcl_;     //application layer protocol
    
    static const int kIdleWTime;  //idle waiting time. unit:s
};

#endif	//_SOCKET_SERVER_H_
