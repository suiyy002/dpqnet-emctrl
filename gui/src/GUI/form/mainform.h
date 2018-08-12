#ifndef MAINFORM_H
#define MAINFORM_H
#include "../component/chart.h"
#include "../view.h"

const int kMaxShowNum = 7;  //同时显示谐波的最大数量
const int kHRuBufNum = 10;  //电压含有率缓存数量
const int kTHeatCurveNum = 3;  //变压器发热预警曲线的数量

struct HrWithIdx {
    short a; //alpha value
    char idx;   //Numbers of harmonic, 2~25  
};

struct raViewData {
    short thr[24][3]; //r-a value
    char list[24][2];   //[0],[1] correspond to last and next of list  
    int show_harm[kMaxShowNum+2];   //harmonic be showed, 0=no show,[kMaxShowNum] is min, [kMaxShowNum+1] is max
                                    //e.g. 3,5,7,9,0...,2,10
    int show_num;   //同时显示谐波的数量
    int hr_buf[24][kHRuBufNum+1]; //Harmonic ratio buffer, last one is max, unit:0.01%
    int head; //Harmonic ratio buffer pointer
};

class CMainForm{
public:
	CMainForm();
	~CMainForm();

	CTabControl *tabctrl_main; //主界面选项标签
	//波形界面的控件
	CChart * chart_u;           //电压图表
	CChart * chart_i;           //电流图表
	CLabel * label_urms;        //电压有效值
	CLabel * label_THDu;        //电压总谐波畸变率
	CLabel * label_irms;        //电流有效值
	CLabel * label_THDi;        //电流总谐波畸变率

	//电容预警界面的控件
	CLabelSet * labelset_cap_title; //电容预警数据标题
	CLabelSet * labelset_cap[4];   	//电容预警数据,0~3分别对应：更新时间、A相、B相、C相
	//r-a界面的控件
	CChart * chart_ra;              //γ-α图表
	//变压器预警界面的控件
	CLabelSet * labelset_trnsfmr[3];//变压器预警数据,0~2分别对应：更新时间、最大值、持续时间
	CChart * chart_t_heat;          //变压器发热曲线图表
	CLabelSet * labelset_theat;     //变压器最新的发热数据。

	CLabel * label_debug1; //调试信息1
	CLabel * label_debug2; //调试信息2
	CLabel * label_rcd_wave;  //手动录波信息提示
	
	void refresh(int type);
	void set_update(){update = true;}
	void SwitchView();
	void PushHRu(void *val);
	void set_view_totl(int num);
	int view_num() {return view_num_; };
	void set_prdct_type(int type){ 
        if (type>=kEEWNet200&&type<kEEWNet300) { //Transformer
            tabctrl_main->set_tabs_title(2, "变压器");
        } else if(type>=kEEWNet300){
            tabctrl_main->set_tabs_title(2, "电容");
        }
	    prdct_type_=type; 
	};
	void switch_view_num (int val);
protected:
private:
enum CurrMainView { kWave, kMVEvent, kEEWCap, kEEWCap_ra, kEEWTrnsfmr };

	int left;
	int top;
	int width;
	int height;
	int color;
	SEdges border;
	bool update;
	int view_totl_;
	int view_num_;  //主界面的子界面的当前序号;
	int curr_view_; //当前界面;
	int prdct_type_;    //product type
	
	CLabel * label_uw_title;    //电压波形标题
	CLabel * label_iw_title;    //电流波形标题
	CLabel * label_urms_title;  //电压有效值标题
	CLabel * label_THDu_title;  //电压总谐波畸变率标题
	CLabel * label_irms_title;  //电流有效值标题
	CLabel * label_THDi_title;  //电流总谐波畸变率标题

	CLabelSet * labelset_event; //事件列表
	CLabel * label_event_title; //事件列表题头
	raViewData ra_data;
    static const int kDisHarmNum = 7;  //显示谐波的数量
    
    int tw_curve_w_;  //Transformer warning chart width

	void draw();
	void InitComponent();
	void InitWaveView();
	void InitEventView();
	void InitCapwView();
	void Init_raView();
	void InitTwView();
	void Draw_raCurve();
	void DrawLegend();
	void SetLegendVal();
    void DrawHRu();
    void ShowEEWCap(int type);
    void Get_raBorder(int &min_idx, int &max_idx, int cnt);
    
    void ShowWave(int type);
    void ShowEventList(int type);
    void ShowEEWT(int type);
    void DrawTwCurve(int type);
};

extern CMainForm * main_form;
#endif /* MAINFORM_H */
