/*
 * sioport.h -- definitions for the standard ioport module
 * Copyright (C) 20011-2012 Seapex Wang and Boyuu Electric
 * The source code in this file can be used only by Boyuu
 */

#ifndef _SIOPORT_H_
#define _SIOPORT_H_

#ifdef ARM
	#define IO_BASE    0x29100000
#elif X86
	#define IO_BASE    0x0000
#else //Other architecture
#endif

#define IO_SIZE    0xFFFF  /* for general IO access */
#define PARALLEL_ADDR    0x378  /* 并口的地址 */
#define PARALLEL_IRQ    46  // 并口对应的中断号。2215为46，2200为41
//------------------------------------------------------------------------------

enum PROOF_TIME_TYPE{
	SECOND_PULSE,	//秒脉冲
	MINUTE_PULSE	//分脉冲
};
//------------------------------------------------------------------------------

// Ioctl definitions
extern int time_intr_num;
extern int intr_unable;
extern int watchdog_intv;
extern int lcm_power;
/* Use 'o' as magic number */
#define SIOPORT_IOC_MAGIC  'o'

#define SIOPORT_IOCSECOND   _IOW(SIOPORT_IOC_MAGIC, 0xe0, time_intr_num)
#define SIOPORT_IOCMINUTE   _IOW(SIOPORT_IOC_MAGIC, 0xe1, time_intr_num)
#define SIOPORT_IOCUNABLE   _IOW(SIOPORT_IOC_MAGIC, 0xe2, intr_unable)
#define SIOPORT_IOCDOGINTV  _IOW(SIOPORT_IOC_MAGIC, 0xe3, watchdog_intv)
#define SIOPORT_IOCDOGOPEN  _IOW(SIOPORT_IOC_MAGIC, 0xe4, watchdog_intv)
#define SIOPORT_IOCDOGCLOSE _IOW(SIOPORT_IOC_MAGIC, 0xe5, watchdog_intv)
#define SIOPORT_IOCLCMPOWER _IOW(SIOPORT_IOC_MAGIC, 0xe6, lcm_power)
#define REGISTER_SIZE 0x4

#define SIOPORT_IOC_MINNR 0xe0

#define SIOPORT_IOC_MAXNR 0xe6
#define WD_GPHCON	0x56000070
#define WD_GPHDAT	0x56000074
#define LCM_PWCON	0x4D000010
#define LCM_PWDAT	0x4D000000
//------------------------------------------------------------------------------

#endif  //_SIOPORT_H_
