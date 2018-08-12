#ifndef _VIEW_PQM_H_
#define _VIEW_PQM_H_
#include "display/display.h"
#include "utility.h"
#include "conversion.h"
#include "time_cst.h"
#include <cstring>

#define HARM_VIEW_TOTL 5	//谐波界面的子界面总数
#define MAIN_VIEW_TOTL 4	//主界面的子界面总数

typedef enum {
    VW_MAIN, VW_HARM, VW_OTHER, VW_SET
} ViewForm;

class ViewPqm{
public:
	ViewPqm(SocketClient * sock);
	~ViewPqm();

	int HandleMsg(int majortype, int minortype);

	static int SHOW_MAX_HARM_NUM;
	
	//Mutators
	void set_data_idx(int val) { data_idx_ = val; };
protected:
private:
    SocketClient *sock_clnt_;
    int data_idx_;  //index of corresponding data buffer
    ClientInfo clnt_inf_;
    DisplayBuf * dis_buf_;
	int msg_type;   //消息归类
	
	int view_num;
	int power_phs_,harm_nums;
	int timex, timey;
	unsigned short int test1,test2,test3;
	int phs_color[3];
	int debug_show, tstcnt ; //for debug
	int bar_scale; //频谱图显示比例
	int iwav_scale; //电流波形显示比例
	int full_scale; //电流满量程时的级数
	int frq_chart_page; //频谱图当前页
	int battery_qnty; //当前电池电量
	int sys_prompt_valid_; //系统信息有效位
	int phs_errcnt[6]; //基波相位错误计数
	unsigned continue_run_time; //系统连续运行时间计数
    unsigned char man_rec_;	///手动录波标志. 1=start, 0=end
    
	void msgkey_treat(int ktype);
	void HandleMsgTimer(int tmtype);
	void msgdata_treat(int dftype);
	void dis_treat();
	void refresh();
	void ini_variable(); //初始化变量

	void view_main_key(int ktype);
	void view_harm_key(int ktype);
	void view_other_key(int ktype);
	void view_set_key(int ktype);

	void view_frame();
	void view_main();
	void view_harm();
	void view_other();
	void view_set();

	void read_rms_hr(int phs, int vc, int hms, char *str, int all_hr=1);
	
	int chk_zero(short *buf, int numi);
	void show_debug();
	void show_wave(int type, int st);
	void show_thd();
	void show_frq_chart(int phs, int type);
	void show_imbalance(int vc);
	void show_vector_chart();
	void show_mlabel();
	bool judge_frq_chart_page();
	void set_iwav_scale();
	void show_power_param();
	void debug_info_handle(int rfrs=0);

	void battery_treat(); //电池电量监测处理
	void sys_prompt_treat(); //系统信息显示处理

enum refresh_modes {ALL_FRSH=1, REALWV_FRSH, SELF_FRSH,
			SHORT_MEMORY};

enum SysPromptID {  //系统信息提示ID
    kBtryPowerLow=1,      //电量低
    kManualRecWvStr,    //手动录波启动成功
    kManualRecWvF,      //手动录波启动失败
    kSaveRec10Str,      //开始存储10周波
    kSaveRec10End,      //结束存储10周波
    kSettingLocked,      //设置界面被锁定
    };
};

static const int HARM_NUM_PERPG = 25; //每页显示的谐波次数

//界面显示响应的消息类型
const int MSG_SWITCH_VIEW = 1;  //切换界面
const int MSG_SWITCH_PHS = 2; //切换相位
const int MSG_WAVE_DATA = 3; //波形数据更新
const int MSG_HARM_DATA = 4; //谐波数据更新
const int MSG_SWITCH_VIEW2 = 5;  //切换2级界面

const int MSG_CHG_HARMNUMS = 20; //改变谐波次数
const int MSG_SWITCH_DBG = 21; //切换调试状态
const int MSG_CLOCK_UPDATE = 22; //时钟更新
const int MSG_PRESS_KEY = 23; //有键按下
const int MSG_SWITCH_CCLAMP_RATIO = 24; //切换电流钳变比


#endif  //_VIEW_PQM_H_
