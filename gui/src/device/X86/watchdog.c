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

int Active_Watchdog; 

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

int CloseWatchdog()
{
	unsigned char status_B;

	if (ioperm(0x70,2,1)==-1)     {
		printf("ioperm  failed\n");
		return(-1);
	}
	if (ioperm(0xa1,1,1)==-1)     {
		printf("ioperm  failed\n");
		return(-1);
	}
	if (ioperm(0x80,1,1)==-1)     {//for inb_p, outb_p...
		printf("ioperm  failed\n");
		return(-1);
	}
	CMOS_READ(RTC_REGC); //clear alarm signal
//	outb(0x0b,0x70);
	status_B=CMOS_READ(RTC_REGB);
	status_B &= 0xdf;//Mask AIE
	CMOS_WRITE(status_B, RTC_REGB);

//    outb(0x0b,0x70);
//    outb(status_B,0x71);

	if (ioperm(0x70,2,0)==-1)
	{
		printf("ioperm failed\n");
		return(-1);
	}
	if (ioperm(0xa1,1,0)==-1)
	{
		printf("ioperm failed\n");
		return(-1);
	}
	if (ioperm(0x80,1,0)==-1)     {//for inb_p, outb_p...
		printf("ioperm  failed\n");
		return(-1);
	}
	return(0);
}

#define DOGTIME 50
int ClearWatchdog()
{
	short int hour,min,sec;
	unsigned int t_sec;
	unsigned char status_B;

	if (ioperm(0x70,2,1)==-1)     {
		printf("ioperm  failed\n");
		return(-1);
	}
	if (ioperm(0xa1,1,1)==-1)     {
		printf("ioperm  failed\n");
		return(-1);
	}
	if (ioperm(0x80,1,1)==-1)     {//for inb_p, outb_p...
		printf("ioperm  failed\n");
		return(-1);
	}

	outb(inb(0xa1)|1,0xa1);   //mask IRQ8?
	CMOS_WRITE_P(0x20,RTC_REGA); //turn the oscillator on
	//Set SET bit of REGB to 1,prevent update
	status_B = CMOS_READ_P(RTC_REGB);
	CMOS_WRITE_P((status_B|0x80),RTC_REGB);

	sec=CMOS_READ(RTC_SECONDS);
	min=CMOS_READ(RTC_MINUTES);
	hour=CMOS_READ(RTC_HOURS);

	BCD_TO_BIN(hour);
	BCD_TO_BIN(min);
	BCD_TO_BIN(sec);

	t_sec=hour*3600+min*60+sec+DOGTIME;

	hour=t_sec/3600%24;
	min =t_sec/60%60;
	sec =t_sec%60;

	BIN_TO_BCD(hour);
	BIN_TO_BCD(min);
	BIN_TO_BCD(sec);

	CMOS_WRITE(hour,RTC_HOURS_ALARM);
	CMOS_WRITE(min,RTC_MINUTES_ALARM);
	CMOS_WRITE(sec,RTC_SECONDS_ALARM);
    
	CMOS_READ(RTC_REGC); //clear alarm signal
//	outb(0x0b,0x70);
	status_B=CMOS_READ(RTC_REGB);
	status_B=status_B|0x20; //Enable Alarm Interrupt
	status_B=status_B&0x7f; //Clear SET bit of REGB to 0,enabe update
	CMOS_WRITE(status_B, RTC_REGB);

//    outb(0x0b,0x70);
//    outb(status_B,0x71);
    
   if (ioperm(0x70,2,0)==-1)
    {
     printf("ioperm failed\n");
     return(-1);
    }
    if (ioperm(0xa1,1,0)==-1)
    {
     printf("ioperm failed\n");
     return(-1);
    }
    if (ioperm(0x80,1,0)==-1)     {//for inb_p, outb_p...
      printf("ioperm  failed\n");
      return(-1);
     }

   return(0);
}

int OpenWatchdog()
{
	ClearWatchdog();
}
