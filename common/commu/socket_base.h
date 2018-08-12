#ifndef _SOCKET_BASE_H_
#define _SOCKET_BASE_H_

#include <stdint.h>

class SocketBase {
public:
    SocketBase(int max);
    virtual ~SocketBase();
    
protected:
    bool IsInt(const char *str);
    int FindIdleFD();
    int CreateCommuObj(int idx, int phy_t, int app_t);
    void DeleteCommuObj(int idx);
    
    int max_connect_;       //maximum connection be established
    struct pollfd *pl_fd_;  //use for poll()
    class CommuDisptchr **commu_dispch_;
    
    //following be only for socket server
    int sock_type_;         //socket type. 0=internet, 1=local.
    char addr_name_[128];   //port number for internet or path/filename for local.

private:
};

#endif	//_SOCKET_BASE_H_
