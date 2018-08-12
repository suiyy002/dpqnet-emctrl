#include <stdio.h>
#include <unistd.h>
#include <asm/io.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "../device.h"

int Active_Watchdog; 

int OpenWatchdog()
{
	set_wdog_param(1,1);
	return 0;
}

int CloseWatchdog()
{
	set_wdog_param(2,1);
	return 1;
}

int ClearWatchdog()
{
	set_wdog_param(0,2000);
	return 1;
}

