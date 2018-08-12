#ifndef _SOCKET_DEVICE_H_
#define _SOCKET_DEVICE_H_

#include "commu_device.h"
#include <stdint.h>

class SocketDevice : public CommuDevice {
public:
    SocketDevice(int fd);
    ~SocketDevice();
	int Restart() { return fd_; };
    
    int Read(void *buf, size_t size);
	int Write(void *buf, size_t size);

protected:
private:
};

#endif	//_SOCKET_DEVICE_H_
