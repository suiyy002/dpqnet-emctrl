#ifndef PQMFUNC_H
#define PQMFUNC_H
#include "daram_usr.h"
#include "prmconfig.h"
#include "../Version.h"
#include "../thread/pthread_mng.h"
#include "../comm/datatype.h"
#include "../GUI/component/battery.h"
#include "save_func.h"

#define OS_HZ 100
//#define TransDataIntSz (sizeof(TRANS_DATABLK_BUF)*INT_BUF_NUM+sizeof(TRANS_TTL_BUF)*3)

//}__attribute__ ((__packed__));

class Cpqm_func{
public:
	Cpqm_func();
	~Cpqm_func();

	short * get_real_pt(int type, int &num);
	short * GetReservPoint(int type, int *num=NULL);
	void harm_hdl();
	void fluct_hdl();
    void HalfSecHandle();
	void indicator_light();
	unsigned int alarm_word(){ return alarm_word_; };
	void read_real(char **bufp, int &sz);
	void start_ato_adj();
	void cancel_ato_adj();
	int get_adj_stat(int *cnt);
	void start_get_dc_val();
	void cancel_dc_val();
	int get_dcval_stat(int *cnt);
	void set_dbg_data(unsigned char isdbg);
	void close_func_transt(void); //关闭暂态功能

	int read_battery_power(int volt=500);
    int get_power_stat() { int ret = power_qnty_==BATTRY_POWER_LOW?1:0; return ret;}; 
	//void set_hrm_time(struct timeval *time) { memcpy(&hrm_time_, time, sizeof(timeval)); };
	void * stdy_z_buf() { return stdy_z_buf_; };

    char *time_str() { return time_str_; };
    bool save_cyc10() { return save_cyc10_; };
    void set_save_cyc10(bool val) { save_cyc10_ = val; save_cyc10_len_=0;};
protected:
private:
	void adjust_phs_zero(unsigned short phs);
	void auto_adjust();
	void get_sample_dc_val();
    void read_time();
    void ScanStdyEvent(int alarm);
    void WriteReal2Shm();
    void WriteQuality2Shm();
    void Harm10cycHdl();
    void SynTimestamp(timeval *time);

	Steady_Z_Data *stdy_z_buf_;
	unsigned int alarm_word_; //超限状态字。1=超限。bit0~2=ABC THDu, bit3~5=HRu, bit6~8=I_hr, bit9~11=Volt_dv,
	                          //bit12=freq, bit13=unbalance, bit14=negative sequence current
	unsigned short *fluct_sv_buf;
	unsigned short harm_valid; //扩展谐波有效标识. bit0=1:26~50有效; bit1=1:间谐波有效
	struct timeval hrm_time_;
	
	int harm_refresh_mon;//用来监测谐波数据是否持续刷新
	SysPara *syspara;
	DebugPara *debug_para;
	
	int auto_adj; //是否自动校准 
	int m_progress_cnt;//进度计数
	int m_get_dc_val; //获取采样电路特征直流分量的状态，0=完成，0xb64b=正在进行，1=超时
	float ato_adj_utol[3], ato_adj_itol[3];
    short base_phs_last[2][3]; //前次的基波相位
    int phs_cnt[2][3]; //基波相位连续错误计数
	char time_str_[24]; //当前时间的字符串格式
	long now_t_;   //当前时间
    int power_qnty_;    //Battery power quantity
    Hrm10CycBuf hrm10_buf_;
    bool save_cyc10_;    //是否开始保存10周波数据
    int save_cyc10_len_; //10周波存储计数
    bool sync_time_en_; //时标 10min 同步
    long offset_us_;    //时标偏移量
    long last_10min_, next_10min_;
    timeval o_time_;
    unsigned short real_quality_;
	
	static void lstner_prd_type(int new_prd_type);
};

const int SteadyZData = 1;
const int TransData = 2;



extern Cpqm_func *pqmfunc;
#endif //PQMFUNC_H

