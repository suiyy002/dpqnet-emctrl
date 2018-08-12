#ifndef APPPRTCLCUSTOM_H
#define APPPRTCLCUSTOM_H

#include <stdint.h>
typedef unsigned char uchar;

const int HarmBufNum = 8;

//---------- Applicate layer 自定义 protocol class -------------------
#define MaxWindowsSz 16
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
class AppPrtclPQ{

public:
	AppPrtclPQ(int hdsz);
	~AppPrtclPQ();
	
protected:
	int custom_limit_handle(uchar *rbuf);
    int cvt_modify_handle(uchar *rbuf);
	int set_para(uchar *rbuf, unsigned int rsz, int cmd);
	int set_syspar(uchar *rbuf, unsigned int rsz);
	void set_time(uchar *rbuf);
	int trigger_able(uchar *rbuf);
	int tx_real(uchar *rbuf, int lastharm);
	int tx_syspar(uchar *rbuf);
	int tx_smp_ver(uchar *buf);
	int tx_smp_data(uchar *rbuf);
	int tx_smp_suc(uchar *rbuf);
    int TxRandkey();
    int Identify(uchar *rbuf);
    int Login(uchar *rbuf);
    int GetUsers();
    int ChgPasswd(uchar *rbuf);
    int SetDeviceSn(uchar *rbuf);
    int TxAuditLog(uchar *rbuf);
    int IniDev(uchar *rbuf, char *update);

	const int kDataHdSz;
	struct TrstRcd *trst_rcd;
	struct SysPara *syspara;

	unsigned char *ptx_bufd; //动态发送缓存，用于发送大块数据
	unsigned char m_tx_buffer[2048];  //静态发送缓存，用于发送小块数据
	char **hrmrcdp;  //取谐波记录时的临时缓存
	uchar *p_freq_rcd_buf;//取频率时的临时缓存
	uchar *p_imbal_rcd_buf;//取不平衡度的临时缓存
	uchar *p_warp_rcd_buf;//取电压偏差的临时缓存

	unsigned int rcd_totl, rcd_pos;
	unsigned int harm_nml_cnt, transt_cnt;//取纪录延迟等待计数 //harm_alm_cnt:删除
	unsigned long event_pos, event_num, event_pkg_num, event_pkg_num_i;
	unsigned long event_tail_pkg_num;//未部包数
	struct timeval event_end_time;
	unsigned char event_cause, event_smpfrq;//暂态原因,采样率
	unsigned char event_uscale_, event_iscale_; //暂态电压, 电流单位系数
	unsigned int transt_max;  //暂态事件的最大保存数目
	FILE *f_transtu, *f_transti;  //当前打开的暂态事件记录文件
	int init_unit_cnt; //对于连续发送的单元初始化命令，只处理一次

	int hmrcdsz[HarmBufNum];
	int iwaitcnt;
	FILE *fp;
	bool logged_; //true=logged in
	int user_id_; //current logged user index
    class TimerCstm *timer_logout_;  //timer for logout
	
private:
	class SecurMng *secur_mng_;
    int ps_err_cnt_;        //连续密码错误计数
    class TimerCstm *timer_lock_;  //timer for login locked because passwd continuous error
    class TimerCstm *timer_setime_; //timer for set time command
    class TimerCstm *timer_inidev_; //timer for initialize device command
};

#define CMD_SRCH 5
//#define CMD_HRM  6
#define CMD_ALM  7
//#define CMD_HRMA 8
#define CMD_REAL 9
#define CMD_SETTM 10
#define CMD_FHSET 11
#define CMD_WTSET 12
#define CMD_INI 13
//#define CMD_FLUTRCD 14
//#define CMD_Pst 15
//#define CMD_PstABLE 16 //obsolete
#define CMD_LCMCRL 17
#define CMD_TRANST 18
#define CMD_SETDEVNUM 19
#define CMD_SETBAUDRATE 20
#define CMD_SAVE_SPACE 21
#define CMD_IP_ADDR 22
#define CMD_PORT_NUM 23
#define CMD_CUSTOM_LIMIT 24
#define CMD_SET_PROTOCOL 25	  
///[DSP
#define CMD_EQUIP_INF 26		///获取设备软硬件信息
#define CMD_SMP_UPFILE 27	///发送采样板升级程序
#define CMD_SMP_UPSTATE 28		///获取采样板升级结果

extern int updsp_syn;
///]
#define CMD_SAVE_TYPE   29
//#define CMD_FREQ_NML 	30
//#define CMD_FREQ_ALM	31
//#define CMD_IMBAL_NML	32
//#define CMD_IMBAL_ALM	33
//#define CMD_WARP_NML	34
//#define CMD_WARP_ALM	35
#define CMD_MAN_REC		36
#define CMD_CONNECTION  37
#define CMD_ONOFF_TIME  38		///读取开关机时间
//#define CMD_LATEST_REC  39
//#define CMD_Plt         40
#define CMD_CVT_MODIFY  41
#define CMD_CAP_WARN    42
#define CMD_CAP_RA      43
#define CMD_CAP_THR     44
#define CMD_RA_PARAM    45
#define CMD_TRNSF_WARN    46
#define CMD_TRNSF_PARAM   47

#define CMD_SYSHIDE_PARAM 49
#define CMD_TIMESYN_PARAM 50
#define CMD_TRIG_ENABLE 51
#define CMD_DC_COMPONENT 52

#define CMD_FREQ	64
#define CMD_UNBLC	65
#define CMD_VOLTDV	66
#define CMD_GET_Pst	67
#define CMD_GET_Plt	68
#define CMD_GET_HRM	69
 
const int CMD_GET_AUDIT_LOG = 92;
const int CMD_SETDEVSN = 93;
const int CMD_CHGPASS = 94;
const int CMD_GETUSERS = 95;
const int CMD_RANDKEY = 96;
const int CMD_IDENTIFY = 97;
const int CMD_LOGIN = 98;
const int CMD_SSHD = 99;
const int CMD_QUIT = 100;

#endif // APPPRTCLCUSTOM_H 
