#ifndef MAINFRAME_H
#define MAINFRAME_H
#include "../component/battery.h"
#include "../view.h"

class CMainFrame{
public:
	CMainFrame();
	~CMainFrame();

	CTabControl *tabctrl_view; //界面选项标签
	CTabControl *tabctrl_phs; //相位选项标签
	CLabel * label_help; //命令提示
	CLabel * label_clamp; //电流钳变比
	CLabel * label_frq; //频率
	CLabel * label_unitnum; //单元编号
	CLabel * label_clock; //时钟
	CBattery * battery_indicator; //电池电量指示
	CLabel * label_sys; //系统信息提示
	int dis_height_; //显示区域的高度
	int dis_top_; //显示区域纵向位置

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
