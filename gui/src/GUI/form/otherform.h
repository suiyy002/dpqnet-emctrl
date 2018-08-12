#ifndef OTHERFORM_H
#define OTHERFORM_H
#include "../component/chart.h"

class COtherForm{
public:
	COtherForm();
	~COtherForm();

	CLabelSet * labelset_lu; //线电压数据
	CLabelSet * labelset_imbalanceu; //电压不平衡
	CLabelSet * labelset_imbalancei; //电流不平衡
	CChartVector *vectorchart_u; //基波电压矢量图
	CChartVector *vectorchart_i; //基波电流矢量图
	CLabelSet * labelset_Pst; //短时闪变
	CLabelSet * labelset_Plt; //长时闪变

	CLabelSet * labelset_warpu; //电压偏差
	CLabel * label_lack_phsu; //电压回路接线不全指示
	CLabel * label_lack_phsi; //电流回路接线不全指示

	CLabelSet * labelset_debug; //调试信息
	
	void get_view_totl(int &tol, int &tol2){ //获取子界面总数
		tol = view_totl; 
		tol2 = view_totl2; 
	};
	void get_view_num(int &num, int &num2){ //获取界面序号
		num = view_num;
		num2 = view_num2;
	};
	void set_view_num(int num, int num2); //设置界面序号

	void draw();
	void refresh(int type);
	void set_update(){update = true;}
protected:
private:
	int left;
	int top;
	int width;
	int height;
	int color;
	SEdges border;
	bool update;
	int view_num; //其它子界面的当前序号;
	int view_num2; //其它二级子界面的当前序号;

	void init_component();
	void draw_daram();

static const int view_totl = 3; //子界面总数
static const int view_totl2 = 9; //二级子界面总数
};

extern COtherForm * other_form;
#endif /* OTHERFORM_H */
