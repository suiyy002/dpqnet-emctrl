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

	CTabControl *tabctrl_harm; //г������ѡ���ǩ
	CChart * chart_harm; //г��Ƶ��ͼ��
	CLabelSet * labelset_harm; //г������
	CLabel * labelset_harm_title; //г��������ͷ
	CLabelSet * labelset_thd; //��г������������
	CLabelSet * labelset_thd_title; //��г�����������ݱ���
	CLabelSet *labelset_power; //�ܹ�������
	CLabel * label_harm; //ĳ��г��������
	CPosCursor * pos_cursor; //Ƶ��ͼ���ָʾ���
	
	int view_num; //г���ӽ���ĵ�ǰ���;
	static const int hmnum_per_page = 6; //���ۺϽ���ÿҳ��ʾ��г������
	static const int view_totl = HARM_VIEW_TOTL; //�ӽ�������
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
