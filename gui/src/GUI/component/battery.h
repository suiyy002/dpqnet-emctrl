#ifndef BATTERY_H
#define BATTERY_H
#include "component.h"

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//		CBattery  实现电池电量显示
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

class CBattery{
public:
	CBattery(int w, int lv);
	~CBattery();

	int color;
	
	void draw(); //绘制电池图形
	void refresh(); //刷新电池显示
	void set_power(int qnty); //设置当前剩余电量
	void set_direct(int direct); //设置电池显示方向
	void set_position(int x, int y); //设置电池显示的位置
	void set_visible(bool yn); //设置显示使能
	void note_power_low(); //通知电量低
protected:
	bool visible;
	int left;
	int top;
	int width;
	int height;
private:
	int power_quantity; //剩余电量，单位％
	bool update;  //是否刷新显示区域
	int direction; //正极的方向, 0:上, 1:下, 2:左, 3:右
	int pole_w; //电极宽度
	int pole_h; //电极高度
	int level; //电量级数
	int level_color_; //电量显示颜色
	bool power_low; //电池电量低
	int low_note_frq_; //电量低提示闪烁频率
	int charge_cnt_; //充电状态计数
	
	static const int PolehRate = 8; //电池整体高度与电极的高度之比
	static const int PolewRate = 2; //电池整体宽度与电极的宽度之比
};

//电池电量定义
#define BATTRY_POWER_FULL 100
#define BATTRY_CHARGING 110
#define BATTRY_TRICKLE_CHARGE 120
#define BATTRY_POWER_LOW 10

#endif /* BATTERY_H */
