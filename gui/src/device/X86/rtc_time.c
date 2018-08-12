#include <stdio.h>
#include <unistd.h>
#define extern
#include <asm/io.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <linux/kd.h>
#include <sys/ioctl.h>

#include "../device.h"


#define CMOS_READ_P(addr) ({ \
outb_p((addr),0x70); \
inb_p(0x71); \
})
#define CMOS_WRITE_P(val, addr) ({ \
outb_p((addr),0x70); \
outb_p((val),0x71); \
})
#define CMOS_READ(addr) ({ \
outb((addr),0x70); \
inb(0x71); \
})
#define CMOS_WRITE(val, addr) ({ \
outb((addr),0x70); \
outb((val),0x71); \
})


int set_rtc_time(struct tm *ptm)
{
	unsigned char save_rega, save_regb;

    //struct timeval tv;
	struct tm stm;
	//time_t newtime;
			    
	iopl(3);
	
	//Set SET bit of REGB to 1,prevent update 
	save_regb = CMOS_READ_P(RTC_REGB);
	CMOS_WRITE_P((save_regb|0x80),RTC_REGB);
	//Enable the oscillator but holds the countdown chain in reset.
	save_rega = CMOS_READ_P(RTC_REGA);
	CMOS_WRITE_P((save_rega|0x70), RTC_REGA);
	
	stm.tm_sec = ptm->tm_sec;
	stm.tm_min = ptm->tm_min;
	stm.tm_hour = ptm->tm_hour;
	stm.tm_wday = ptm->tm_wday+1;
	stm.tm_mday = ptm->tm_mday;
	stm.tm_mon = ptm->tm_mon+1;
	stm.tm_year = (ptm->tm_year+1900)%100;
	
        BIN_TO_BCD(stm.tm_sec);
	BIN_TO_BCD(stm.tm_min);
	BIN_TO_BCD(stm.tm_hour);
	BIN_TO_BCD(stm.tm_wday);
	BIN_TO_BCD(stm.tm_mday);
	BIN_TO_BCD(stm.tm_mon);
	BIN_TO_BCD(stm.tm_year);

        CMOS_WRITE_P (stm.tm_sec, RTC_SECONDS);
        CMOS_WRITE_P (stm.tm_min, RTC_MINUTES);
        CMOS_WRITE_P (stm.tm_hour, RTC_HOURS);
        //CMOS_WRITE_P (stm.tm_wday, );
        CMOS_WRITE_P (stm.tm_mday, RTC_DAY);
	CMOS_WRITE_P (stm.tm_mon, RTC_MONTH);
	CMOS_WRITE_P (stm.tm_year, RTC_YEAR);
	
	CMOS_WRITE_P (save_rega, RTC_REGA);
	CMOS_WRITE_P (save_regb, RTC_REGB);
	printf("set rtc time!\n");
	return 0;
}

void getlocaltime(struct timeval *time)
{
	gettimeofday(time, NULL);
}

void setlocaltime(struct timeval *time)
{
	time_t time_ti;
	struct tm *ptmi;

	settimeofday(time,NULL);
	
	ptmi = gmtime(&time_ti);
	set_rtc_time(ptmi);
}

