#ifndef VIEWSET_H
#define VIEWSET_H

#include <time.h>
#include "display/display.h"
#include "conversion.h"
#include "component/component.h"

class Charm_func;

class CSetView{
public:
	CSetView(int w, int h);
	~CSetView();

	SFont font;
	int color;
	bool frame_visible; //�Ƿ���ʾ�߿�
	int cursor_left[8]; //������߾�
	int cursor_top[8]; //�����ϱ߾�

	unsigned long rand_num;
	unsigned short order;

	void view_set();
	int view_set_key(int ktype);
    inline void return_from_set(int ktype) {  //�˳����ý���
        curr_cmd_ = cmd_stack_[0];
        view_set_key(ktype);
    };
	void ini_para();
	void show_cursor();
	int harm_refresh();
    bool Locked();
    
    void set_left(int x) { left_ = x; };
    void set_top(int y) { top_ = y; };

protected:
private:
    inline void pushcmd(int cmd) {  //����������ջ
        cmd_stack_[pcmd_stack_] = cmd;
        if (pcmd_stack_<9) pcmd_stack_++; 
    };
    inline int popcmd() {           //���������ջ
        if(pcmd_stack_>0) pcmd_stack_--; 
        return cmd_stack_[pcmd_stack_];
    };
    inline void reset_stack() {      //Reset Stack
        pcmd_stack_ = 0; 
    };
    
	inline void clear() { pqm_dis->clear(left_, top_, left_+width, top_+height); };
    inline void refresh() { pqm_dis->refresh(left_, top_, left_+width, top_+height); };
	inline void show_frame() {
		if(frame_visible){
			pqm_dis->rectangle(left_,top_,left_+width,top_+height,color);
		}
	};

    void view_init(int type);
    
	void view_sxxini(int type, const char* name, int row, int x1, int x2, int y);
	void view_sxxini_key(int type, int ktype);
	void view_sxx(int type, int x, int y, int ext=0);
	void view_sxx_key(int type, int ktype, int ext=0);

	void view_shmcurlmt();
	void Show01Time();

	int edit_num(int ktype,int nst,char *str,int limtnum, bool sign=false);
	int select_lst(int ktype,int nst,int limtnum);
	void IniMdfyTime(tm *ptm);
	void verify_time(tm *ptm);
    void Bit2Str(char *str, long val, int offset);
    int Freq2Indx(int data, int type);
    void ShowAuditEvent(int type, int x0, int y0);
    void RunIniSys(int type);
	void set_pop_box(int x0, int y0, int x1, int y1) {
	    pop_box_.x0 = x0; pop_box_.y0 = y0; pop_box_.x1 = x1; pop_box_.y1 = y1;};

	int left_, top_;
	int width, height;
	//int view_num, view_num_ly1, view_num_ly2, view_num_ly3,*view_lyp;
	struct PopBox {int x0,y0,x1,y1;};  //Pop-up box position&size. (x0,y0)=left&top, (x1,y1)=right&bottom
	PopBox pop_box_;

	char key_buf[6][24];
	int buf_pt[8];
	bool para_update_;  //��������
	int eew_update_;    //eew�����豸Ԥ����ز�������
	bool sys_dbg, comm_update, daram_update,socket_update,netpara_update;
	bool m_serial_prtcl_update, m_socket_prtcl_update, gps_pulse_update, ntppara_update;
	int buf_sel_, buf_sel_sz_;
	bool nd_rfrsh_; //need refresh
	tm time_set_; //��������ʱ��Ļ���
	char **phrm_rcd_buf; //г��������¼��ѯ����ָ��
	int rcdsz[8];
	int hwaddr[6];//Mac ��ַ�ݴ�
	
	int cursorx, cursory;
	bool showcursor;
	
	unsigned short freq;
	int alm_num;
	int alm_dis_num; //ÿ����ʾ�ı�����¼��
	unsigned short *alm_ary;
	int TranstRcdTimeOkCnt; //��̬��¼ʱ������ȷ�ϴ���
	int set_confirm_ok_cnt_; //��ȷ�����������
	int TranstRcdTimeData; 

    long ra_show_hr_;   //harmonic be showed. bit0~23 correspond to 2~25 harmonic
	unsigned int num_onoff; //��ſ��ػ����ݸ����������10

    bool have_title_;       //�����Ƿ��б���
    char* ptitle;           //����
    int tiltle_height_;     //����߶�
    bool force_exit_;       //ǿ���˳�
    int curr_cmd_;          //��ǰ����
    int menu_num_;          //��ǰ����Ĳ˵������Ŀ
    int item_totl_;         //��ǰ�˵���Ĳ���������0=ֻ��1ҳ
    int menu2cmd_[10];      //��ǰ������˵����Ӧ������
    int menu_tag_[10];      //��ǰ������˵����Ӧ��tag����
    int menu_x_;            //��ǰ����˵�����ʼ������
    int Item_y_[10];        //��ǰ������˵����������
    int Item_x_[10];        //��ǰ������˵�������ֵ����ʼ������
    int curr_x_, curr_y_;    //��ǰ������ֵ������
    int curr_sn_;           //��ǰ�˵������

    int cmd_stack_[10];     //�����ջ
    int pcmd_stack_;        //�����ջ��ǰָ��
    int ps_err_cnt_;        //��������������
    time_t lock_t_;         //login locked time
    int trnst_valid_;  //transient record function enable. 0=invalid, 1=valid, 0xc3=uninitialized

};


#endif // VIEWSET_H 
