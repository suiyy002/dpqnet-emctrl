#ifndef MAINFORM_H
#define MAINFORM_H
#include "../component/chart.h"
#include "../view.h"

const int kMaxShowNum = 7;  //ͬʱ��ʾг�����������
const int kHRuBufNum = 10;  //��ѹ�����ʻ�������
const int kTHeatCurveNum = 3;  //��ѹ������Ԥ�����ߵ�����

struct HrWithIdx {
    short a; //alpha value
    char idx;   //Numbers of harmonic, 2~25  
};

struct raViewData {
    short thr[24][3]; //r-a value
    char list[24][2];   //[0],[1] correspond to last and next of list  
    int show_harm[kMaxShowNum+2];   //harmonic be showed, 0=no show,[kMaxShowNum] is min, [kMaxShowNum+1] is max
                                    //e.g. 3,5,7,9,0...,2,10
    int show_num;   //ͬʱ��ʾг��������
    int hr_buf[24][kHRuBufNum+1]; //Harmonic ratio buffer, last one is max, unit:0.01%
    int head; //Harmonic ratio buffer pointer
};

class CMainForm{
public:
	CMainForm();
	~CMainForm();

	CTabControl *tabctrl_main; //������ѡ���ǩ
	//���ν���Ŀؼ�
	CChart * chart_u;           //��ѹͼ��
	CChart * chart_i;           //����ͼ��
	CLabel * label_urms;        //��ѹ��Чֵ
	CLabel * label_THDu;        //��ѹ��г��������
	CLabel * label_irms;        //������Чֵ
	CLabel * label_THDi;        //������г��������

	//����Ԥ������Ŀؼ�
	CLabelSet * labelset_cap_title; //����Ԥ�����ݱ���
	CLabelSet * labelset_cap[4];   	//����Ԥ������,0~3�ֱ��Ӧ������ʱ�䡢A�ࡢB�ࡢC��
	//r-a����Ŀؼ�
	CChart * chart_ra;              //��-��ͼ��
	//��ѹ��Ԥ������Ŀؼ�
	CLabelSet * labelset_trnsfmr[3];//��ѹ��Ԥ������,0~2�ֱ��Ӧ������ʱ�䡢���ֵ������ʱ��
	CChart * chart_t_heat;          //��ѹ����������ͼ��
	CLabelSet * labelset_theat;     //��ѹ�����µķ������ݡ�

	CLabel * label_debug1; //������Ϣ1
	CLabel * label_debug2; //������Ϣ2
	CLabel * label_rcd_wave;  //�ֶ�¼����Ϣ��ʾ
	
	void refresh(int type);
	void set_update(){update = true;}
	void SwitchView();
	void PushHRu(void *val);
	void set_view_totl(int num);
	int view_num() {return view_num_; };
	void set_prdct_type(int type){ 
        if (type>=kEEWNet200&&type<kEEWNet300) { //Transformer
            tabctrl_main->set_tabs_title(2, "��ѹ��");
        } else if(type>=kEEWNet300){
            tabctrl_main->set_tabs_title(2, "����");
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
	int view_num_;  //��������ӽ���ĵ�ǰ���;
	int curr_view_; //��ǰ����;
	int prdct_type_;    //product type
	
	CLabel * label_uw_title;    //��ѹ���α���
	CLabel * label_iw_title;    //�������α���
	CLabel * label_urms_title;  //��ѹ��Чֵ����
	CLabel * label_THDu_title;  //��ѹ��г�������ʱ���
	CLabel * label_irms_title;  //������Чֵ����
	CLabel * label_THDi_title;  //������г�������ʱ���

	CLabelSet * labelset_event; //�¼��б�
	CLabel * label_event_title; //�¼��б���ͷ
	raViewData ra_data;
    static const int kDisHarmNum = 7;  //��ʾг��������
    
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
