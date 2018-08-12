#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#include "battery.h"
#include "display/vga_color.h"

CBattery::CBattery(int w, int lv)
{
	visible = true;
	color = vgacolor(kVGA_Default);//COLOR_GREY1;
	left = 0;
	top = 0;
	width = w;
	level = lv;
	height = level+2;
	pole_w = width/PolewRate;
	pole_h = height/PolehRate;
	power_quantity = 50;
	update = true;
	direction = 0;
	level_color_ = vgacolor(kVGA_Green);
	charge_cnt_ = 0;
}

void CBattery::draw()
{
	int tp, lf, hg, wd; //电池尺寸
	int x1, y1, h1, w1; //电极尺寸
	int x2, y2, h2, w2; //电量尺寸
	
	update = false;

	if (!visible) {
	    return;
	}
	if(power_low){
		if(!(++low_note_frq_%2)) return;
		power_low = false;
	}
	//int qnty = level*(power_quantity+100/level/2)/100;
	int qnty = power_quantity;
	if(direction<2){ //电池方向为上下
		lf = left;
		wd = width;
		hg = height;
		x1 = left + (width-pole_w)/2;
		w1 = pole_w;
		h1 = pole_h;
		x2 = left+1;
		h2 = (level*qnty+50)/100;
		w2 = width-2;
		if(direction==0){ //上
			y1 = top;
			tp = top + pole_h;
			y2 = tp + 1 + level - h2;
		}
		if(direction==1){ //下
			tp = top;
			y1 = top + height;
			y2 = tp + 1;
		}
	}else { //电池方向为左右
		tp = top;
		wd = height;
		hg = width;
		y1 = top + (width-pole_w)/2;
		w1 = pole_h;
		h1 = pole_w;
		y2 = top+1;
		h2 = width-2;
		w2 = (level*qnty+50)/100;
		if(direction==2){ //左
			lf = left + pole_h;
			x1 = left;
			x2 = lf + 1 + level - w2;
		}
		if(direction==3){ //右
			lf = left;
			x1 = left+height;
			x2 = lf + 1;
		}
	}
	pqm_dis->rectframe(lf, tp, lf+wd, tp+hg, color); //绘电池框
	pqm_dis->rectangle(x1, y1, x1+w1, y1+h1, color); //绘电极
	pqm_dis->rectangle(x2, y2, x2+w2, y2+h2, level_color_); //绘电量
	pqm_dis->rectframe(lf+1, tp+1, lf+wd-1, tp+hg-1, vgacolor(kVGA_Black)); //绘电池框与电量之间的缝隙
}
 
void CBattery::refresh()
{
	if(!update) return;

	int  wd, hg;
	if(direction<2){ //电池方向为上下
		wd = width;
		hg = height + pole_h;
	}else { //电池方向为左右
		wd = height + pole_h;
		hg = width;
	}
	pqm_dis->clear(left, top, left+wd, top+hg);
	draw();
	pqm_dis->refresh(left, top, left+wd, top+hg);

}

//设置当前剩余电量
void CBattery::set_power(int qnty)
{
	int k = 5; //充电动画分几段
	int i = 100/k;
	if(qnty==BATTRY_CHARGING){
		power_quantity = i*charge_cnt_;
		charge_cnt_ ++;
		charge_cnt_ %= (k+1);
	} else if (qnty==BATTRY_TRICKLE_CHARGE){
		power_quantity = i*charge_cnt_;
		charge_cnt_ ++;
		charge_cnt_ %= (k+1);
		if(charge_cnt_==0) charge_cnt_ = k-2;
	}else{
		power_quantity = qnty;
		charge_cnt_ = 0;
	}
	if (power_quantity<19) {
	    level_color_ = vgacolor(kVGA_Yellow);
	} else {
	    level_color_ = vgacolor(kVGA_Green);
	}
	update = true;
}
	
//通知电量低
void CBattery::note_power_low()
{
	power_low = true;
	update = true;
}

//设置电池显示方向
void CBattery::set_direct(int direct)
{
	direction = direct;
}

//设置电池显示的位置
void CBattery::set_position(int x, int y)
{
	left = x;
	top = y;
}

//设置显示使能
void CBattery::set_visible(bool yn)
{
	visible = yn;
}
