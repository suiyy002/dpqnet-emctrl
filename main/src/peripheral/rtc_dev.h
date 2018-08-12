#ifndef _RTC_DEV_H_
#define _RTC_DEV_H_
//---------------------------------------------------------------------------
#include "time_cst.h"
#include <stdint.h>
#include <linux/rtc.h>   // /usr/local/arm/cross/am335xt3/devkit/arm-arago-linux-gnueabi/usr/include

class RtcDev
{
public:
    RtcDev(){
        char * rtc_dev = "/dev/rtc";
        fd = open(rtc_dev, O_RDWR);
        if (fd < 0) {
            printf( "Failed to open /dev/rtc!\n" );
            fd = NULL;
        }
    };
    ~RtcDev(){ if (fd) close(fd); };

    time_t Get() { //Get time of rtc
        if (!fd) return 0;
        struct tm tmi;
        int ret = ioctl(fd, RTC_RD_TIME, &tmi);
        if (ret < 0) {
            printf( "Failed to read rtc!\n" );
            return 0;
        } else {
            printf("get hwclock %02d:%02d:%02d\n", tmi.tm_hour, tmi.tm_min, tmi.tm_sec);
            return MakeTime(&tmi, 0); //将tm类型时间转换为time_t类型
        }
    };
    
    /*!
    Return:   <0,调用失败; >=0成功
    */
    int Set(time_t time) { //Set time of rtc
        if (!fd) return -1;
        struct tm tmi;
        GmTime(&tmi, &time); //将time_t类型时间转换为tm类型
        int ret = ioctl(fd, RTC_SET_TIME, &tmi);
        if (ret < 0) {
            printf( "Failed to set rtc!\n" );
        }
        return ret;
    };
    
protected:
private:
	int fd;
};

inline RtcDev &rtc_dev()
{
	static RtcDev rtc;
	return rtc;
};


#endif //_RTC_DEV_H_
