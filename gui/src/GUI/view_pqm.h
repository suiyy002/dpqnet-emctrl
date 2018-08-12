#ifndef _VIEW_PQM_H_
#define _VIEW_PQM_H_
#include "display/display.h"
#include "utility.h"
#include "conversion.h"
#include "time_cst.h"
#include <cstring>

#define HARM_VIEW_TOTL 5	//г��������ӽ�������
#define MAIN_VIEW_TOTL 4	//��������ӽ�������

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
	int msg_type;   //��Ϣ����
	
	int view_num;
	int power_phs_,harm_nums;
	int timex, timey;
	unsigned short int test1,test2,test3;
	int phs_color[3];
	int debug_show, tstcnt ; //for debug
	int bar_scale; //Ƶ��ͼ��ʾ����
	int iwav_scale; //����������ʾ����
	int full_scale; //����������ʱ�ļ���
	int frq_chart_page; //Ƶ��ͼ��ǰҳ
	int battery_qnty; //��ǰ��ص���
	int sys_prompt_valid_; //ϵͳ��Ϣ��Чλ
	int phs_errcnt[6]; //������λ�������
	unsigned continue_run_time; //ϵͳ��������ʱ�����
    unsigned char man_rec_;	///�ֶ�¼����־. 1=start, 0=end
    
	void msgkey_treat(int ktype);
	void HandleMsgTimer(int tmtype);
	void msgdata_treat(int dftype);
	void dis_treat();
	void refresh();
	void ini_variable(); //��ʼ������

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

	void battery_treat(); //��ص�����⴦��
	void sys_prompt_treat(); //ϵͳ��Ϣ��ʾ����

enum refresh_modes {ALL_FRSH=1, REALWV_FRSH, SELF_FRSH,
			SHORT_MEMORY};

enum SysPromptID {  //ϵͳ��Ϣ��ʾID
    kBtryPowerLow=1,      //������
    kManualRecWvStr,    //�ֶ�¼�������ɹ�
    kManualRecWvF,      //�ֶ�¼������ʧ��
    kSaveRec10Str,      //��ʼ�洢10�ܲ�
    kSaveRec10End,      //�����洢10�ܲ�
    kSettingLocked,      //���ý��汻����
    };
};

static const int HARM_NUM_PERPG = 25; //ÿҳ��ʾ��г������

//������ʾ��Ӧ����Ϣ����
const int MSG_SWITCH_VIEW = 1;  //�л�����
const int MSG_SWITCH_PHS = 2; //�л���λ
const int MSG_WAVE_DATA = 3; //�������ݸ���
const int MSG_HARM_DATA = 4; //г�����ݸ���
const int MSG_SWITCH_VIEW2 = 5;  //�л�2������

const int MSG_CHG_HARMNUMS = 20; //�ı�г������
const int MSG_SWITCH_DBG = 21; //�л�����״̬
const int MSG_CLOCK_UPDATE = 22; //ʱ�Ӹ���
const int MSG_PRESS_KEY = 23; //�м�����
const int MSG_SWITCH_CCLAMP_RATIO = 24; //�л�����ǯ���


#endif  //_VIEW_PQM_H_
