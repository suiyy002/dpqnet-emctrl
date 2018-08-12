#ifndef OTHERFORM_H
#define OTHERFORM_H
#include "../component/chart.h"

class COtherForm{
public:
	COtherForm();
	~COtherForm();

	CLabelSet * labelset_lu; //�ߵ�ѹ����
	CLabelSet * labelset_imbalanceu; //��ѹ��ƽ��
	CLabelSet * labelset_imbalancei; //������ƽ��
	CChartVector *vectorchart_u; //������ѹʸ��ͼ
	CChartVector *vectorchart_i; //��������ʸ��ͼ
	CLabelSet * labelset_Pst; //��ʱ����
	CLabelSet * labelset_Plt; //��ʱ����

	CLabelSet * labelset_warpu; //��ѹƫ��
	CLabel * label_lack_phsu; //��ѹ��·���߲�ȫָʾ
	CLabel * label_lack_phsi; //������·���߲�ȫָʾ

	CLabelSet * labelset_debug; //������Ϣ
	
	void get_view_totl(int &tol, int &tol2){ //��ȡ�ӽ�������
		tol = view_totl; 
		tol2 = view_totl2; 
	};
	void get_view_num(int &num, int &num2){ //��ȡ�������
		num = view_num;
		num2 = view_num2;
	};
	void set_view_num(int num, int num2); //���ý������

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
	int view_num; //�����ӽ���ĵ�ǰ���;
	int view_num2; //���������ӽ���ĵ�ǰ���;

	void init_component();
	void draw_daram();

static const int view_totl = 3; //�ӽ�������
static const int view_totl2 = 9; //�����ӽ�������
};

extern COtherForm * other_form;
#endif /* OTHERFORM_H */
