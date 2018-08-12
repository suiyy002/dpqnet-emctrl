#ifndef MAINFRAME_H
#define MAINFRAME_H
#include "../component/battery.h"
#include "../view.h"

class CMainFrame{
public:
	CMainFrame();
	~CMainFrame();

	CTabControl *tabctrl_view; //����ѡ���ǩ
	CTabControl *tabctrl_phs; //��λѡ���ǩ
	CLabel * label_help; //������ʾ
	CLabel * label_clamp; //����ǯ���
	CLabel * label_frq; //Ƶ��
	CLabel * label_unitnum; //��Ԫ���
	CLabel * label_clock; //ʱ��
	CBattery * battery_indicator; //��ص���ָʾ
	CLabel * label_sys; //ϵͳ��Ϣ��ʾ
	int dis_height_; //��ʾ����ĸ߶�
	int dis_top_; //��ʾ��������λ��

	void draw();
	int refresh();
	void set_update(){update = true;}
	
	int height() { return height_; };
	int width() { return width_; };
	
protected:
private:

	int left;
	int top;
	int width_;
	int height_;
	int color;
	SEdges border;
	bool update;
	void InitComponent();
};

extern CMainFrame * main_frame;
#endif /* MAINFRAME_H */
