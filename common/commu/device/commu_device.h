#ifndef _COMMU_DEVICE_H_
#define _COMMU_DEVICE_H_

#include <termios.h> 

//--------------- Communication device base class -----------------
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
class CommuDevice {
public:
	CommuDevice(){};
	virtual ~CommuDevice(){};
	
	//读设备数据
	//返回值: <=0,读设备出错; 实际读取的数目
	virtual int Read(void *buf, size_t size)=0;
	virtual int Write(void *buf, size_t size)=0;
	virtual int Restart()=0;
protected:
	int fd_;    //communication device file descriptor
private:
};

#endif // _COMMU_DEVICE_H_ 
