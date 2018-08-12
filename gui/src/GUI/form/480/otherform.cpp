#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#include "../otherform.h"
#include "../mainframe.h"
#include "display/vga_color.h"

COtherForm * other_form;

//-----------------------------------------------------------------------
COtherForm::COtherForm()
{
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
	
	view_num = 0;
	view_num2 = 0;
	
	update = true;
	init_component();
}

//-----------------------------------------------------------------------
COtherForm::~COtherForm()
{
}

//-----------------------------------------------------------------------
void COtherForm::init_component()
{
	CLabelSet *plabelset;
	CLabel *plabel;
	CChartVector *pvctrchart;
	int k;

	//线电压数据初始化
	labelset_lu = new CLabelSet(4);
	plabelset = labelset_lu;
	plabelset->left = left + 4;
	plabelset->top = top + 4;
	plabelset->width = width - plabelset->left - 1;
	plabelset->height = 18;
	//plabelset->transparent = false;
	//plabelset->bgcolor = vgacolor(kVGA_Default);
	//plabelset->font.color = COLOR_BLACK;
	plabelset->font.space = 2;
	plabelset->font.asc_size = 16;
	plabelset->font.cn_size = 14;
	plabelset->font.color = vgacolor(kVGA_LightYellow);
	plabelset->allrefresh = true;
	plabelset->labels_[0].top = 0;
	plabelset->set_txt(0,"线电压(kV):");
	k = 108;
	for(int i=1;i<4;i++){
		plabelset->labels_[i].top = 0;
		plabelset->labels_[i].left = k;
		k += 104;
	}

	//电压不平衡度数据初始化
	labelset_imbalanceu = new CLabelSet(6);
	plabelset = labelset_imbalanceu;
	plabelset->left = labelset_lu->left;
	plabelset->top = labelset_lu->top + 28;
	plabelset->width = 96;
	plabelset->height = 120;
	plabelset->font.space = 1;
	plabelset->font.cn_size = 14;
	plabelset->font.asc_size = 16;
	//plabelset->font.color = vgacolor(kVGA_LightYellow);
	//plabelset->allrefresh = true;
	plabelset->set_txt(0,"电压不平衡:");
	plabelset->set_txt(2,"负序电压:");
	plabelset->set_txt(4,"零序电压:");
	k = 0;
	for(int i=0;i<3;i++){
		plabelset->labels_[2*i].top = k;
		k += 40;
	}
	for(int i=0;i<3;i++){
		plabelset->labels_[2*i+1].top = plabelset->labels_[2*i].top+18;
		plabelset->labels_[2*i+1].left = 8;
	}
	//电流不平衡度数据初始化
	labelset_imbalancei = new CLabelSet(6);
	plabelset = labelset_imbalancei;
	*plabelset = *labelset_imbalanceu;
	plabelset->left += 240;
	plabelset->set_txt(0,"电流不平衡:");
	plabelset->set_txt(2,"负序电流:");
	plabelset->set_txt(4,"零序电流:");

	//基波电压矢量图初始化
	vectorchart_u = new CChartVector(3,50);
	pvctrchart = vectorchart_u;
	pvctrchart->left = left+84;
	pvctrchart->top = labelset_imbalancei->top;
	pvctrchart->font.space = -1;
	pvctrchart->font.asc_size = 12;
	pvctrchart->set_axis_title(0," 0°");
	pvctrchart->set_axis_title(1," 90°");
	pvctrchart->set_axis_title(2,"180°");
	pvctrchart->set_axis_title(3," 270°");
	pvctrchart->set_data_label(0,"A");
	pvctrchart->set_data_label(1,"B");
	pvctrchart->set_data_label(2,"C");
	//基波电流矢量图初始化
	vectorchart_i = new CChartVector();
	pvctrchart = vectorchart_i;
	*pvctrchart = *vectorchart_u;
	pvctrchart->left += width/2;
	pvctrchart->set_data_label(0,"A");
	pvctrchart->set_data_label(1,"B");
	pvctrchart->set_data_label(2,"C");
	

	//电压回路接线不全指示初始化
	label_lack_phsu = new CLabel();
	plabel = label_lack_phsu;
	plabel->left = left + 8;
	plabel->top = labelset_lu->top + 28;
	plabel->left = labelset_imbalanceu->left;
	plabel->font.cn_size = 14;
	plabel->font.space = 2;
	plabel->set_txt("电压回路接线不全！");
	//电流回路接线不全指示初始化
	label_lack_phsi = new CLabel();
	plabel = label_lack_phsi;
	*plabel = *label_lack_phsu;
	plabel->left = labelset_imbalancei->left;
	plabel->set_txt("电流回路接线不全！");

	//电压偏差数据初始化
	labelset_warpu = new CLabelSet(4);
	plabelset = labelset_warpu;
	plabelset->left = labelset_lu->left;
///[PLT
	plabelset->top = top + height - 56;
///]
	plabelset->width = width - 2;
	plabelset->height = 18;
	plabelset->font.space = 3;
	plabelset->font.cn_size = 14;
	plabelset->font.asc_size = 16;
	plabelset->font.color = vgacolor(kVGA_LightYellow);
	//plabelset->allrefresh = true;
	plabelset->labels_[0].top = 0;
	plabelset->set_txt(0,"电压偏差:");
	k = 88;
	for(int i=1;i<4;i++){
		plabelset->labels_[i].top = 0;
		plabelset->labels_[i].left = k;
		k += 112;
	}
	//短时闪变数据初始化
	labelset_Pst = new CLabelSet(4);
	plabelset = labelset_Pst;
	*plabelset = *labelset_warpu;
	plabelset->top += 18;
	plabelset->set_txt(0,"短时闪变:");
	//长时闪变数据初始化
	labelset_Plt = new CLabelSet(4);
	plabelset = labelset_Plt;
	*plabelset = *labelset_Pst;
	plabelset->top += 18;
	plabelset->set_txt(0,"长时闪变:");

	//调试信息数据初始化
	labelset_debug = new CLabelSet(10);
	plabelset = labelset_debug;
	plabelset->left = left + 4;
	plabelset->top = top + 8;
	plabelset->width = width;
	plabelset->height = height-6;
	//plabelset->transparent = false;
	//plabelset->bgcolor = vgacolor(kVGA_Default);
	//plabelset->font.color = COLOR_BLACK;
	plabelset->font.space = 1;
	plabelset->font.asc_size = 12;
	//plabelset->allrefresh = true;
	k = 0;
	for(int i=0;i<10;i++){
		plabelset->labels_[i].top = k;
		plabelset->labels_[i].left = 0;
		k += 16;
		if(i==6) k +=10;
	}
}

//-----------------------------------------------------------------------
void COtherForm::draw()
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
	if(view_num==0){
		labelset_lu->draw();
		label_lack_phsi->draw();
		label_lack_phsu->draw();
		labelset_imbalanceu->draw();
		labelset_imbalancei->draw();
		vectorchart_u->draw();
		vectorchart_i->draw();
		labelset_warpu->draw();
		labelset_Pst->draw();
		labelset_Plt->draw();
	}else if(view_num==1){
		if(view_num2){
			draw_daram();
		}else{
			labelset_debug->draw();
		}
	}else if(view_num==2){
		labelset_debug->draw();
	}
	update = false;
}

//-------------------------------------------------------------------------
//type刷新类型，1=全部刷新
void COtherForm::refresh(int type)
{
	if(type){ //显示器全部刷新
		draw();
	}else if(update){ //窗口全部刷新
		pqm_dis->clear(left,top,left+width,top+height);
		draw();
		pqm_dis->refresh(left,top,left+width,top+height);
	}else{
		if(view_num==0){
			labelset_lu->refresh();
			label_lack_phsu->refresh();
			label_lack_phsi->refresh();
			labelset_imbalanceu->refresh();
			labelset_imbalancei->refresh();
			labelset_Pst->refresh();
			labelset_Plt->refresh();
			labelset_warpu->refresh();
			vectorchart_u->refresh();
			vectorchart_i->refresh();
		}else if(view_num==1){
			if(!view_num2){
				labelset_debug->refresh();
			}
		}else if(view_num==2){
			labelset_debug->refresh();
		}
	}
	update = false;
}

//-------------------------------------------------------------------------
void COtherForm::draw_daram()
{

}

//-------------------------------------------------------------------------
//设置界面序号
//num(I): view_num; num2(I): view_num2;
void COtherForm::set_view_num(int num, int num2)
{
	view_num = num;
	view_num2 = num2;
	
	if (view_num==1) {
		labelset_debug->allrefresh = false;
	} else if(view_num==2) {
		labelset_debug->allrefresh = true;
	}
}

