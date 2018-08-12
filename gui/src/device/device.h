#ifndef _DEVICE_H_
#define _DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif
///[DSP
#define HAND_FLAG 0	///握手标志
#define SE_FLAG 1	///开始结束标志
#define RW_FLAG 2	///读写标志
#define SUC_FLAG 3	///成功标志
#define DSP_DATA 4	///数据
#define DSP_CLEAR 5	///清双口RAM

#define SEND_DSP 0x5a5a	
#define DSP_ACK 0xa5a5

#define DSP_READABLE 0xa5
#define DSP_START 0xa5
#define DSP_UNSUC 0xa5

#define DSP_WRITABLE 0x5a
#define DSP_END 0x5a
#define DSP_SUC 0x5a

#define DSP_DATADOWN 0xAA
#define DSP_TYPENOMATCH 0x55
#define DSP_ERASERFAIL 0xCC
#define DSP_CHECKDATA 0x33
#define DSP_UPDATING 0xC3
#define DSP_RESTART 0x3C
static const char * UpdspFile = "save/updsp.bin"; //升级存储文件

extern int updsp_syn;

#define DSP_RET_SUC 1 
#define DSP_RET_UPDATING 2
#define DSP_RET_TYPENOMATCH 3
#define DSP_RET_ERASER 4
#define DSP_RET_CHECKDATA 5
#define DSP_RET_RESTART 6
#define DSP_RET_RECVDATA 7 
#define DSP_RET_OPENFILE 8 
#define DSP_RET_WAITHAND 9 
#define DSP_RET_READFILE 10 
#define DSP_RET_WAITWRITABLE 11 

///]

//-------------------drv_device ----------------------------------------------
#ifdef X86
	#define STDIN_DEV "/dev/tty0"  //标准输入设备
	#define ALARM_LED_BIT 0x01 //报警LED控制位, 高电平关
	#define ALARM_LED_BITL 0xFF //为了与arm兼容
	#define RUN_LED_BIT 0x02 //运行LED控制位
	#define LCM_PW_CTRL_BIT 0x40 //lCD电源控制位
	
	#define ALARM_RELAY_BIT 0x04 //报警继电器控制位
	#define EXCEPTION_RELAY_BIT 0x010 //异常继电器控制位
	#define FAILURE_RELAY_BIT 0x08 //故障继电器控制位
	#define PST_RELAY_BIT 0x20 //闪变报警继电器控制位
	#define SKIP_SWITCH_BIT 0x80 //跳闸继电器控制位

#elif defined ARM
	#define STDIN_DEV "/dev/tty0"  //标准输入设备
	#define ALARM_LED_BIT 0x02 //为了与x86兼容
	#define ALARM_LED_BITL 0xFD //报警LED关闭位，低电平关
	#define RUN_LED_BIT 0x04 //运行LED控制位
	#define LCM_PW_CTRL_BIT 0x08 //lCD电源控制位
	
	#define ALARM_RELAY_BIT 0x20 //超限报警继电器控制位
	#define EXCEPTION_RELAY_BIT 0x40 //异常继电器控制位
	#define FAILURE_RELAY_BIT 0x80 //故障继电器控制位
	#define PST_RELAY_BIT 0x01 //闪变报警继电器控制位
	#define SKIP_SWITCH_BIT 0x10 //跳闸继电器控制位
	#define SWITCH_IN1 0x40 //开关量输入1
	#define SWITCH_IN2 0x80 //开关量输入2

#else //Other architecture
#endif

#define DEV_DARAMPOLL "/dev/darampoll"
#define DEV_DARAMINT "/dev/daramint"
#define DEV_SIOPORT "/dev/sioport"

void open_device();
void close_device();
unsigned char read_paral_port(char num);
void set_paral_port(char bit, int val);
void read_switch_in();
void set_alarm(int type, unsigned int alarm);
void refresh_daram(unsigned short *buf, int num, int type);

//--- DSP -----
void save_dsp(unsigned short *buf,int num,int type);
void read_dsp(unsigned short *buf,int num,int type);
int updsp(void);
void get_dsp_update_state(void);
void set_updsp_flag(unsigned char val);
unsigned char updsp_flag();



void save_01_time(long *buf);
void read_01_time(long *buf,int type);
///[DSP
void read_dsp_ver(unsigned short *ver,unsigned short *type);
///]
void lcm_poweroff();
void lcm_poweron();
void set_gps_time(int type, int num); //设置GPS对时的参数
void set_wdog_param(int type, int num);//设置看门狗参数
int rw_rtc_time(int type, time_t *ptime); //读写RTC的时间
void set_transt_pkg_num(int type, int num);//设置暂态数据包数量

#define RTC_SECONDS 0
#define RTC_SECONDS_ALARM 1
#define RTC_MINUTES 2
#define RTC_MINUTES_ALARM 3
#define RTC_HOURS   4
#define RTC_HOURS_ALARM   5
#define RTC_DAY	    7
#define RTC_MONTH   8
#define RTC_YEAR    9
#define RTC_REGA    10
#define RTC_REGB    11
#define RTC_REGC    12
#define CENTURY     50


#define BCD_TO_BIN(val) ((val)=((val)&15)+((val)>>4)*10)
#define BIN_TO_BCD(val) ((val)=(((val)/10)<<4) + (val)%10)

//------------------ Watch dog -----------------------------------------------
/*
#define WD_GPHCON	0x56000070
#define WD_GPHDAT	0x56000074
*/
int OpenWatchdog();
int CloseWatchdog();
int ClearWatchdog();

//-------------- rtc_time ---------------------------------------------------
void getlocaltime(struct timeval *time);
void setlocaltime(struct timeval *time);
void setlocaltime_gps(struct timeval *time);

//-------------- net ---------------------------------------------------
void get_net_para(int type, char *str);
void set_net_para(int type, char *str);
void apply_net_para();
void chk_net_file(void);
void get_ntp_para(int type, char *str);
void set_ntp_para(int type, char *str);
void apply_ntp_para();
void UpNtpParaFile();

//中断DSP
void interrupt_dsp(unsigned short inttype, unsigned short *data, int dlen);
int read_daram (void *buf, int num, int ofst);
int write_daram (void *buf, int num, int ofst);

void ColdReboot();

extern int Active_Watchdog; 
extern int max_head_num;
extern int max_tail_num;


#ifdef __cplusplus
}
#endif

#endif 

