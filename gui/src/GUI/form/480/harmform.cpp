#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#include "../harmform.h"
#include "../mainframe.h"
#include "display/vga_color.h"

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//		CPosCursor
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
CPosCursor::CPosCursor(int w, int h):CCartoon(w,h)
{
}

void CPosCursor::paint()
{
	pqm_dis->draw_icon(left, top-1, ICON_ARROW1, color, 1);
	pqm_dis->line(left+5,top+11,left+5,top+height,color,3);
}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//		CHarmForm
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
CHarmForm * harm_form;

//-----------------------------------------------------------------------
CHarmForm::CHarmForm()
{
	view_num = 0;
	
	left = 0;
	top = main_frame->dis_top_;
	width = main_frame->width();
	height = main_frame->dis_height_;
	color = vgacolor(kVGA_Default);
	border.left_width = 0;
	border.top_width = 1;
	border.right_width = 0;
	border.bottom_width = 1;
	border.color = vgacolor(kVGA_Default);
	
	update = true;
	init_component();
}

//-----------------------------------------------------------------------
CHarmForm::~CHarmForm()
{
}

//-----------------------------------------------------------------------
void CHarmForm::init_component()
{
	CLabelSet *plabelset;
	CLabel *plabel;
	int k;

	//谐波界面选项标签类初始化
	tabctrl_harm = new CTabControl(5);
	tabctrl_harm->top = top;
	tabctrl_harm->left = 23;
	//tabctrl_harm->width = width;
	tabctrl_harm->height = height;
	tabctrl_harm->tab_position = tpLEFT;
	tabctrl_harm->tab_height = tabctrl_harm->left-3;
	tabctrl_harm->set_tab_start(8);
	tabctrl_harm->border.left_width = 1;
	tabctrl_harm->font.cn_size = 14;
	tabctrl_harm->font.space = 2;
    tabctrl_harm->tab_vofst = (tabctrl_harm->tab_height-tabctrl_harm->font.cn_size)/2;
	tabctrl_harm->set_tabs_title(0, "综合");
	tabctrl_harm->set_tabs_title(1, "电压");
	tabctrl_harm->set_tabs_title(2, "电流");
	tabctrl_harm->set_tabs_title(3, "有功");
	tabctrl_harm->set_tabs_title(4, "无功");

	int w = 9; //柱子宽度
	int intv = 4; //柱子间距
	int cht_wd = (w+intv)*(HARM_NUM_PERPG+1)-1; //频谱图宽度
	//某次谐波数据初始化
	label_harm = new CLabel("");
	plabel = label_harm;
	plabel->width = cht_wd;
	plabel->height = 16;
	plabel->top = 24;
	plabel->left = tabctrl_harm->left +34;
	plabel->font.asc_size = 16;
	plabel->auto_size = false;
	plabel->transparent = false;
	plabel->bgcolor = vgacolor(kVGA_HiGrey);
	plabel->font.color = vgacolor(kVGA_Default);
	plabel->font.space = 1;
	plabel->top_space = 2;
	plabel->left_space = 8;

	//谐波频谱图表初始化
	chart_harm = new CChart(cht_wd, height-58);
	chart_harm->left = label_harm->left;
	chart_harm->top = label_harm->top+32;
	chart_harm->border.left_width = 0;
	chart_harm->border.top_width = 1;
	chart_harm->border.right_width = 1;
	//chart_harm->border.bottom_width = 0;
	chart_harm->set_series_type(kSeriesBar);
	chart_harm->bar_width = w;
	chart_harm->start_point = w + intv;
	chart_harm->draw_range = cht_wd+1 - chart_harm->start_point;
	chart_harm->set_last_tick(true);

	chart_harm->axis_x.position = 0;
	chart_harm->axis_x.axis_border = 0; //坐标轴线型为dash
	chart_harm->axis_x.ticks_grid = false; //不显示刻度栅格
	//chart_harm->axis_x.ticks_grid_border = 3; //刻度栅格的线型dot
	chart_harm->axis_x.ticks_origin = false; //不显示原点刻度
	chart_harm->axis_x.ticks_spc = (1000*(w+intv)*5+cht_wd/2)/cht_wd; //刻度间距
	chart_harm->set_scale_val(0, 5); //刻度单位数值
	chart_harm->axis_x.labels_x = -5; //刻度标注位置
	chart_harm->axis_x.labels_y = 2;
	chart_harm->axis_x.minor_num = 4; //辅助刻度的数目
	//chart_harm->axis_x.minor_grid = true; //显示辅助刻度栅格
	chart_harm->axis_x.minor_len = 3; //辅助刻度长度
	chart_harm->axis_x.ticks_len = 5; //刻度长度
	chart_harm->axis_x.right_margin = 1;
	chart_harm->axis_x.labels_font.asc_size = 12;
	chart_harm->axis_x.labels_font.space = 0;
	
	chart_harm->axis_y.position = 0;
	chart_harm->axis_y.ticks_len = 3; //刻度长度
	chart_harm->axis_y.minor_len = 0; //不显示辅助刻度
	chart_harm->axis_y.labels_x = -4; //刻度标注位置
	chart_harm->axis_y.labels_y = 4; 
	chart_harm->axis_y.labels_font.space = -1;
	//chart_harm->axis_y.val_unit = "%"; //刻度值的显示单位 
	chart_harm->axis_y.ticks_spc = 188; //刻度间距
	chart_harm->axis_y.val_prec = 2; //刻度标注的精度
	chart_harm->axis_y.val_decimal = 0;
	//chart_harm->axis_y.ticks_origin = false; //不显示原点刻度
	//chart_u->axis_y.labels_align = 1; //左对齐
	chart_harm->axis_y.title = "( ％)"; 
	chart_harm->axis_y.title_x = -30;
	chart_harm->axis_y.title_y = -2; //标题的位置
	chart_harm->axis_y.title_font.asc_size = 12;

	chart_harm->axis_y.labels_x = 2; //刻度标注位置
	chart_harm->axis_y.labels_font.asc_size = 12;
	chart_harm->axis_y.bottom_margin = 6;
	chart_harm->set_visible(false);

	//频谱图表的指示光标初始化
	k = chart_harm->top - label_harm->top - label_harm->height;
	k += chart_harm->height();
	pos_cursor = new CPosCursor(11,k);
	pos_cursor->color = vgacolor(kVGA_HiCyan);
	pos_cursor->move(chart_harm->left,label_harm->top+label_harm->height);

	//总谐波畸变率数据标题初始化
	labelset_thd_title = new CLabelSet(2);
	plabelset = labelset_thd_title;
	plabelset->set_txt(0,"总谐波");
	plabelset->set_txt(1,"畸变率");
	plabelset->left = left + width - 72;
	plabelset->top = chart_harm->top;
	plabelset->width = 70;
	plabelset->height = 44;
	plabelset->transparent = false;
	plabelset->bgcolor = vgacolor(kVGA_Blue1);
	plabelset->font.color = vgacolor(kVGA_Default);
	plabelset->font.space = 2;
	plabelset->font.cn_size = 14;
	plabelset->font.asc_size = 16;
	k = 6;
	for(int i=0;i<2;i++){
		plabelset->labels_[i].top = k;
		plabelset->labels_[i].left = 8;
		k += 18;
	}
	//总谐波畸变率数据初始化
	labelset_thd = new CLabelSet(6);
	plabelset = labelset_thd;
	plabelset->left = labelset_thd_title->left;
	plabelset->top = labelset_thd_title->top + labelset_thd_title->height + 1;
	plabelset->width = labelset_thd_title->width;
	plabelset->height = 112;
	plabelset->allrefresh = true;
	plabelset->transparent = false;
	plabelset->bgcolor = vgacolor(kVGA_Default);
	plabelset->font.color = vgacolor(kVGA_Black);
	plabelset->font.asc_size = 16;
	plabelset->font.space = 1;
	k = 6;
	for(int i=0;i<3;i++){
		plabelset->labels_[2*i].top = k;
		plabelset->labels_[2*i].left = 2;
		k += 15;
		plabelset->labels_[2*i+1].top = k;
		plabelset->labels_[2*i+1].left = 2;
		k +=19;
	}

	//总功率率数据初始化
	labelset_power = new CLabelSet(12);
	plabelset = labelset_power;
	plabelset->width = labelset_thd_title->width;
	plabelset->height = labelset_thd_title->height;
	plabelset->left = labelset_thd_title->left;
	plabelset->top = chart_harm->top;
	plabelset->allrefresh = true;
	//plabelset->transparent = false;
	//plabelset->bgcolor = vgacolor(kVGA_Default);
	plabelset->font.color = vgacolor(kVGA_Default);
	plabelset->font.cn_size = 14;
	plabelset->font.asc_size = 12;
	plabelset->font.space = 0;
	k = 22;
	for(int i=1;i<6;i++){
		plabelset->labels_[2*i].top = k;
		plabelset->labels_[2*i].left = 2;
		k +=12;
		plabelset->labels_[2*i+1].top = k;
		plabelset->labels_[2*i+1].left = 10;
		k +=16;
	}
	//plabelset->labels_[0]_.self_font = true;
	plabelset->labels_[0].left = 8;
	plabelset->labels_[0].font.space = 3;

	//plabelset->labels_[1].self_font = true;
	plabelset->labels_[1].left = 0;
	plabelset->labels_[1].top = 12;
	//plabelset->labels_[1].font.space = -1;
	//plabelset->labels_[1].font.asc_size = 12;
	
	plabelset->set_txt(0,"总功率");
	plabelset->set_txt(1,"---------");
	plabelset->set_txt(8,"PF:");
	plabelset->set_txt(10,"DPF:");

	//谐波数据初始化
	labelset_harm = new CLabelSet(12);
	labelset_harm->border.top_width = 1;
	labelset_harm->left = tabctrl_harm->left + 16;
	labelset_harm->top = top + 24;
	labelset_harm->width = 272;
	labelset_harm->height = 180;
	labelset_harm->allrefresh = true;
	labelset_harm->font.space = 1;
	labelset_harm->font.asc_size = 12;
	k = 4;
	int cnt = labelset_harm->get_count();
	for(int i=0;i<cnt/2;i++){
		labelset_harm->labels_[2*i].top = k;
		k += 13;
		labelset_harm->labels_[2*i+1].top = k;
		k += 16;
	}
	//谐波数据题头初始化
	labelset_harm_title = new CLabel("幅值     含有率   相位");
	labelset_harm_title->left = labelset_harm->left + 64;
	labelset_harm_title->top = labelset_harm->top - 17;
	labelset_harm_title->font.space = 2;
	labelset_harm_title->font.cn_size = 14;
}

//-----------------------------------------------------------------------
void CHarmForm::draw()
{
	int i, wl, wr;


	//画边框
	for(i=0;i<border.left_width;i++) { //左边框, 上-->下
		pqm_dis->line(left-i,top,left-i,top+height,border.color,0);
	}
	for(i=0;i<border.right_width;i++) { //右边框, 上-->下
		pqm_dis->line(left+width+i-1,top,left+width+i-1,top+height,border.color,0);
	}
	wl = border.left_width>1?border.left_width-1:0;
	wr = border.right_width>1?border.right_width-1:0;
	for(i=0;i<border.top_width;i++) { //上边框, 左-->右
		pqm_dis->line(left-wl,top-i,left+width+wr,top-i,border.color,0);
	}
	for(i=0;i<border.bottom_width;i++) { //下边框, 左-->右
		pqm_dis->line(left-wl,top+height+i-1,left+width+wr,top+height+i-1,border.color,0);
	}
	
	tabctrl_harm->draw();
	chart_harm->draw();
	labelset_harm->draw();
	labelset_harm_title->draw();
	labelset_thd_title->draw();
	labelset_thd->draw();
	label_harm->draw();
	pos_cursor->draw();
	labelset_power->draw();
	update = false;
}

//-------------------------------------------------------------------------
//type刷新类型，=1:全部刷新
void CHarmForm::refresh(int type)
{
	if(type){ //显示器全部刷新
		draw();
	}else if(update){ //窗口全部刷新
		pqm_dis->clear(left,top,left+width,top+height);
		draw();
		pqm_dis->refresh(left,top,left+width,top+height);
	}else{
		tabctrl_harm->refresh();
		chart_harm->refresh();
		labelset_harm->refresh();
		labelset_harm_title->refresh();
		labelset_thd->refresh();
		label_harm->refresh();
		pos_cursor->refresh();
		labelset_power->refresh();
	}
	update = false;
}
//-------------------------------------------------------------------------
//切换谐波子界面
void CHarmForm::switch_view()
{
	//设置界面选项标签
	tabctrl_harm->set_tab_index(view_num);
	labelset_thd_title->set_visible(true);
	labelset_thd->set_visible(true);
	labelset_power->set_visible(false);
	if(view_num){ //频谱图界面
		chart_harm->set_visible(true);
		label_harm->set_visible(true);
		pos_cursor->set_visible(true);
		labelset_harm->set_visible(false);
		labelset_harm_title->set_visible(false);
		if(view_num>2){ //功率频谱
			labelset_thd_title->set_visible(false);
			labelset_thd->set_visible(false);
			labelset_power->set_visible(true);
		}
	}else{
		chart_harm->set_visible(false);
		label_harm->set_visible(false);
		pos_cursor->set_visible(false);
		labelset_harm->set_visible(true);
		labelset_harm_title->set_visible(true);
	}
}


