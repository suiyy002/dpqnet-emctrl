#ifndef _MCU_DEV_H_
#define _MCU_DEV_H_
//---------------------------------------------------------------------------
#include <stdint.h>
#include <linux/spi/spidev.h>   // /usr/local/arm/cross/am335xt3/devkit/arm-arago-linux-gnueabi/usr/include

class McuDev
{
public:
    McuDev(const char *dev, int num=1);
    ~McuDev();
    
    SendCmd();
    
protected:

private:
    struct CommObject * comm_obj_; //Communication object
};

enum McuModbusCmd {kPower0Time=100};


#endif //_MCU_DEV_H_
