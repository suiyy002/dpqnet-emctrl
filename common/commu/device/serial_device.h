#ifndef _SERIAL_DEVICE_H_
#define _SERIAL_DEVICE_H_

#include "commu_device.h" 

class SerialDevice : public CommuDevice {
public:
    SerialDevice();
    ~SerialDevice();
    
    int Read(void *buf, size_t size);
	int Write(void *buf, size_t size);
	int Restart();

	int Open(int dev_num, unsigned int baud);
	void Close();
	
protected:
private:
	struct termios oldtio_; 
	int dev_num_;   //number of serial port device
	int baud_;      //baudrate 
};

#endif // _SERIAL_DEVICE_H_ 
