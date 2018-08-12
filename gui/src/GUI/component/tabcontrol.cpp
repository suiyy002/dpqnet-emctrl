#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#include "component.h"
#include "display/vga_color.h"

CTabControl::CTabControl(int num)
{
	font.asc_size = 8;
	font.cn_size = 12;
	font.space = 0;
	font.color = vgacolor(kVGA_White);
	color = vgacolor(kVGA_Blue1);
	border.left_width = 0;
	border.top_width = 0;
	border.right_width = 0;
	border.bottom_width = 0;
	border.color = vgacolor(kVGA_Default);
	tab_position = tpTOP;
	tab_height = 15;
	tab_index = 0;
	tab_start_ = 0;
	tab_idx_dis_ = 0;
	tab_max_ = tab_num_ = num;
	tabs_title_ = new char*[tab_num_];
	tabs_fcolor = new int[tab_num_];
	tabs_indent_ = new int[tab_num_];
	tabs_width_ = new int[tab_num_];
	//memset(tabs_width_, 28, tab_num_);
	tabs_tol_width_ = 28*tab_num_;
	for(int i=0;i<tab_num_;i++){
		tabs_title_[i] = NULL;
		tabs_fcolor[i] = vgacolor(kVGA_Default);
		tabs_indent_[i] = 2;
		tabs_width_[i] = 28;
	}
	tab_vofst = (tab_height-font.cn_size+1)/2;
	tab_enable = true;

	left = 0;
	top = tab_height;
	width = 128;
	height = 64;

	update = true;
}

//-------------------------------------------------------------------------
CTabControl::~CTabControl()
{
	delete [] tabs_indent_;
	delete [] tabs_fcolor;
	for(int i=0;i<tab_max_;i++){
		if(tabs_title_[i]!=NULL){
			delete [] tabs_title_[i];
		}
	}
	delete [] tabs_title_;
}

//-------------------------------------------------------------------------
//使能控件，type=false:禁止；type=true:使能
void CTabControl::enable_tab(bool type)
{
	tab_enable = type;
	update = true;
}

//-------------------------------------------------------------------------
//设置当前选中的标签
bool CTabControl::set_tab_index(int num)
{
	if(num<tab_num_){
		tab_index = num;
		update = true;
		return true;
	}else{
		return false;
	}
	
}
//-------------------------------------------------------------------------
//设置标签的标题
bool CTabControl::set_tabs_title(int num, char *str)
{
	if(num<tab_num_){
		int i = strlen(str);
		
		if(tabs_title_[num]!=NULL)
			delete [] tabs_title_[num];
		tabs_title_[num] = new char[i+2];
		strcpy(tabs_title_[num], str);
		//计算标题的宽度及缩进量
		int k = 0;
		int n = 0;
		while(k<i){
			if((unsigned char)str[k]>=0xa1){
				n += (font.cn_size+font.space);
				k += 2;
			}else{
				n += (7+font.space);
				k ++;
			}
		}
		//n -= font.space;
		tabs_width_[num] = n + font.space + 4;
		update = true;
		return true;
	}else{
	    printf("num>=tab_num_; %s num=%d,tab_num=%d\n", str, num, tab_num_);
		return false;
	}
}

//-------------------------------------------------------------------------
void CTabControl::draw()
{
	if (!tab_num_) return;
	update = false;
	int i, j, x1, x2, y1, y2, wl, wr, vh;

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
	
    tabs_tol_width_ = tabs_width_[0];
	for (i=1;i<tab_num_;i++) tabs_tol_width_ += tabs_width_[i];
	tab_idx_dis_ = 0;
	for (i=0;i<tab_index;i++) tab_idx_dis_ += tabs_width_[i];

	if(tab_position==tpTOP||tab_position==tpBOTTOM){
		x1 = left+tab_start_;
		x2 = x1+tabs_tol_width_;
		vh = 1;
		if(tab_position==tpTOP){
			y1 = top-tab_height;
			y2 = top;
		}else{ //tab_position==tpBOTTOM
			y1 = top+height-1;
			y2 = y1+tab_height;
		}
	}else{ //tab_position==tpLEFT||tab_position==tpRIGHT
		y1 = top + tab_start_;
		y2 = y1 + tabs_tol_width_;
		vh = 0;
		if(tab_position==tpLEFT){
			x1 = left-tab_height;
			x2 = left;
		}else{ //tab_position==tpRIGHT
			x1 = left+width;
			x2 = x1+tab_height;
		}
	}
	
	int cl = vgacolor(kVGA_Black);
	pqm_dis->rectangle(x1,y1,x2+1,y2+1,cl);
	pqm_dis->rectframe(x1,y1,x2+1,y2+1,border.color);//绘标签标题外框
	pqm_dis->set_font(font.cn_size, font.asc_size);
	if(vh){ //标签在上下位置
		int x = x1;
		for(i=0;i<tab_num_;i++){ 
			pqm_dis->puts(tabs_title_[i], x+tabs_indent_[i],
					y2-tab_vofst, tabs_fcolor[i], font.space);  //绘字符
			x += tabs_width_[i];
			pqm_dis->line(x,y1,x,y2,border.color,0);            //绘标签标题分隔线
		}
		if(tab_enable){ //绘选中标题
			i = x1+tab_idx_dis_;
			pqm_dis->rectangle(i+1,y1+1,i+tabs_width_[tab_index],y2,color);
			pqm_dis->puts(tabs_title_[tab_index], i+tabs_indent_[tab_index],
					y2-tab_vofst, font.color, font.space);
		}
	}else{ //标签在左右位置
		int y = y1;
		for (i=0;i<tab_num_;i++) { 
			pqm_dis->vputs(tabs_title_[i], x1+tab_vofst,    
					       y+tabs_indent_[i], tabs_fcolor[i], font.space);  //绘字符
			y += tabs_width_[i];
			pqm_dis->line(x1,y,x2,y,border.color,0);        //绘标签标题分隔线
		}
		if (tab_enable) { //绘选中标签
			i = y1+tab_idx_dis_;
			pqm_dis->rectangle(x1+1,i+1,x2, i+tabs_width_[tab_index],color);
			pqm_dis->vputs(tabs_title_[tab_index], x1+tab_vofst, 
					i+tabs_indent_[tab_index], font.color, font.space);
		}
	}
}

//-------------------------------------------------------------------------
void CTabControl::refresh()
{
	if(!update||!tab_num_) return;

	int  x1, x2, y1, y2;

	if(tab_position==tpTOP||tab_position==tpBOTTOM){
		x1 = left+tab_start_;
		x2 = x1+tabs_tol_width_;
		if(tab_position==tpTOP){
			y1 = top-tab_height;
			y2 = top;
		}else{ //tab_position==tpBOTTOM
			y1 = top+height;
			y2 = y1+tab_height;
		}
	}else{ //tab_position==tpLEFT||tab_position==tpRIGHT
		y1 = top + tab_start_;
		y2 = y1 + tabs_tol_width_;
		if(tab_position==tpLEFT){
			x1 = left-tab_height;
			x2 = left;
		}else{ //tab_position==tpRIGHT
			x1 = left+width;
			x2 = x1+tab_height;
		}
	}
	
	pqm_dis->clear(x1,y1,x2,y2);
	draw();
	pqm_dis->refresh(x1,y1,x2,y2);
}
