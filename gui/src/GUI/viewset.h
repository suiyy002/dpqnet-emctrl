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
	bool frame_visible; //是否显示边框
	int cursor_left[8]; //光标的左边距
	int cursor_top[8]; //光标的上边距

	unsigned long rand_num;
	unsigned short order;

	void view_set();
	int view_set_key(int ktype);
    inline void return_from_set(int ktype) {  //退出设置界面
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
    inline void pushcmd(int cmd) {  //设置命令入栈
        cmd_stack_[pcmd_stack_] = cmd;
        if (pcmd_stack_<9) pcmd_stack_++; 
    };
    inline int popcmd() {           //设置命令出栈
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
	bool para_update_;  //参数更新
	int eew_update_;    //eew电力设备预警相关参数更新
	bool sys_dbg, comm_update, daram_update,socket_update,netpara_update;
	bool m_serial_prtcl_update, m_socket_prtcl_update, gps_pulse_update, ntppara_update;
	int buf_sel_, buf_sel_sz_;
	bool nd_rfrsh_; //need refresh
	tm time_set_; //本地设置时间的缓存
	char **phrm_rcd_buf; //谐波报警纪录查询缓存指针
	int rcdsz[8];
	int hwaddr[6];//Mac 地址暂存
	
	int cursorx, cursory;
	bool showcursor;
	
	unsigned short freq;
	int alm_num;
	int alm_dis_num; //每屏显示的报警记录数
	unsigned short *alm_ary;
	int TranstRcdTimeOkCnt; //暂态记录时间设置确认次数
	int set_confirm_ok_cnt_; //需确认设置项计数
	int TranstRcdTimeData; 

    long ra_show_hr_;   //harmonic be showed. bit0~23 correspond to 2~25 harmonic
	unsigned int num_onoff; //存放开关机数据个数，最多是10

    bool have_title_;       //界面是否有标题
    char* ptitle;           //标题
    int tiltle_height_;     //标题高度
    bool force_exit_;       //强制退出
    int curr_cmd_;          //当前命令
    int menu_num_;          //当前界面的菜单项的数目
    int item_totl_;         //当前菜单项的参数总数，0=只有1页
    int menu2cmd_[10];      //当前界面各菜单项对应的命令
    int menu_tag_[10];      //当前界面各菜单项对应的tag变量
    int menu_x_;            //当前界面菜单的起始横坐标
    int Item_y_[10];        //当前界面各菜单项的纵坐标
    int Item_x_[10];        //当前界面各菜单项设置值的起始横坐标
    int curr_x_, curr_y_;    //当前设置项值的坐标
    int curr_sn_;           //当前菜单项序号

    int cmd_stack_[10];     //命令堆栈
    int pcmd_stack_;        //命令堆栈当前指针
    int ps_err_cnt_;        //连续密码错误计数
    time_t lock_t_;         //login locked time
    int trnst_valid_;  //transient record function enable. 0=invalid, 1=valid, 0xc3=uninitialized

};


#endif // VIEWSET_H 
