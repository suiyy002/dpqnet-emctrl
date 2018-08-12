#ifndef HARMFORM_H
#define HARMFORM_H
#include "../component/chart.h"
#include "../view.h"

class CPosCursor:public CCartoon{
public:
	CPosCursor(int w, int h);
	~CPosCursor(){};
	void paint();
protected:
private:
};

class CHarmForm{
public:
	CHarmForm();
	~CHarmForm();

	CTabControl *tabctrl_harm; //谐波界面选项标签
	CChart * chart_harm; //谐波频谱图表
	CLabelSet * labelset_harm; //谐波数据
	CLabel * labelset_harm_title; //谐波数据题头
	CLabelSet * labelset_thd; //总谐波畸变率数据
	CLabelSet * labelset_thd_title; //总谐波畸变率数据标题
	CLabelSet *labelset_power; //总功率数据
	CLabel * label_harm; //某次谐波的数据
	CPosCursor * pos_cursor; //频谱图表的指示光标
	
	int view_num; //谐波子界面的当前序号;
	static const int hmnum_per_page = 6; //在综合界面每页显示的谐波次数
	static const int view_totl = HARM_VIEW_TOTL; //子界面总数
	void draw();
	void refresh(int type);
	void set_update(){update = true;}
	void switch_view();
protected:
private:
	int left;
	int top;
	int width;
	int height;
	int color;
	SEdges border;
	bool update;
	void init_component();
};

extern CHarmForm * harm_form;
#endif /* HARMFORM_H */
