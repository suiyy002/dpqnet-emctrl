#include <stdio.h>
#include <unistd.h>
//#define extern
#include <asm/io.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "../device.h"

void getlocaltime(struct timeval *time)
{
    gettimeofday(time, NULL);
    //time->tv_sec += time_zone;
}

void setlocaltime(struct timeval *time)
{
    //time->tv_sec -= time_zone;
    settimeofday(time, NULL);

    struct tm *ptm1;
    //ptm1 = localtime(&(time->tv_sec));
    rw_rtc_time(0, &(time->tv_sec));
    //set_rtc_time(ptmi);
}
