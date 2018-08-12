#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <math.h>

using namespace std;

#include "chart.h"
#include "display/vga_color.h"

//num:矢量的个数，len:轴长度
CChartVector::CChartVector(int num, int len)
{
	visible = true;
	left = 0;
	top = 0;
	width_ = len*2+56;
	height_ = len*2+20;
	color = vgacolor(kVGA_Default);
	font.asc_size = 8;
	font.cn_size = 12;
	font.space = 0;
	font.color = vgacolor(kVGA_Default);
	border.left_width = 0;
	border.top_width = 0;
	border.right_width = 0;
	border.bottom_width = 0;
	border.color = vgacolor(kVGA_Default);

	series_arrow = true;
	axis_len = len;
	bar_width = 3;
	axis_scale = axis_len;

	for(int i=0;i<4;i++){
		axis_title[i].text = NULL;
		axis_title[i].self_font = false;
		axis_title[i].font.asc_size = 8;
		axis_title[i].font.cn_size = 12;
		axis_title[i].font.space = 0;
		axis_title[i].font.color = vgacolor(kVGA_Default);
	}
	axis_title[0].left = 1*width_/2 + 1*len + 2;
	axis_title[1].left = 1*width_/2 + 0*len;
	axis_title[2].left = 0*width_/2 + 0*len + 2;
	axis_title[3].left = 1*width_/2 + 0*len;
	axis_title[0].top = 1*height_/2 + 0*len + 2;
	axis_title[1].top = 0*height_/2 + 0*len + 11;
	axis_title[2].top = 1*height_/2 + 0*len + 2;
	axis_title[3].top = 1*height_/2 + 1*len + 8;
	
	data_num = num;
	if(num>0){
		data_value = new Vector[num];
		data_color = new int[num];
		data_label = new SLabel[num];
		for(int i=0;i<num;i++){
			data_label[i].left = 0;
			data_label[i].top = 0;
			data_label[i].text = NULL;
			data_label[i].self_font = false;
			data_label[i].font.asc_size = 8;
			data_label[i].font.cn_size = 12;
			data_label[i].font.space = 0;
			data_label[i].font.color = vgacolor(kVGA_Default);
			data_value[i].amp = 0;
		}
	}else{
		data_value = NULL;
		data_color = NULL;
		data_label = NULL;
	}
    
	clear();
	update = true;
}

//-------------------------------------------------------------------------
CChartVector::~CChartVector()
{
	if(data_value!=NULL) delete [] data_value;
	if(data_color!=NULL) delete [] data_color;
	if(data_label!=NULL) delete [] data_label;
}

//-------------------------------------------------------------------------
// 赋值函数
CChartVector & CChartVector::operator =(const CChartVector &other)
{	
	//检查自赋值
	if(this == &other)
		return *this;
	
	visible = other.visible;
	left = other.left;
	top = other.top;
	width_ = other.width_;
	height_ = other.height_;
	color = other.color;
	memcpy(&font, &other.font, sizeof(SFont));
	memcpy(&border, &other.border, sizeof(SEdges));

	series_arrow = other.series_arrow;
	axis_len = other.axis_len;
	bar_width = other.bar_width;
	axis_scale = other.axis_scale;
	
	for(int i=0;i<4;i++){
		if(axis_title[i].text!=NULL){
			delete [] axis_title[i].text;
		}
	}
	for(int i=0;i<4;i++){
		memcpy(&axis_title[i], &other.axis_title[i], sizeof(SLabel));
		if(other.axis_title[i].text!=NULL){
			int j = strlen(other.axis_title[i].text);
			axis_title[i].text = new char[j+2];
			strcpy(axis_title[i].text, other.axis_title[i].text);
		}
	}

	for(int i=0;i<data_num;i++){
		if(data_label[i].text!=NULL){
			delete [] data_label[i].text;
		}
	}
	if(data_value!=NULL) delete [] data_value;
	if(data_color!=NULL) delete [] data_color;
	if(data_label!=NULL) delete [] data_label;
	data_num = other.data_num;
	data_value = new Vector[data_num];
	data_color = new int[data_num];
	data_label = new SLabel[data_num];
	for(int i=0;i<data_num;i++){
		memcpy(&data_label[i], &other.data_label[i], sizeof(SLabel));
		data_label[i].text = NULL;
		memcpy(&data_value[i], &other.data_value[i], sizeof(Vector));
		data_value[i].amp = other.data_value[i].amp;
		data_value[i].phs = other.data_value[i].phs;
	}

	//返回本对象的引用
	return *this;
}


//-------------------------------------------------------------------------
void CChartVector::set_visible(bool yn)
{
	visible = yn;
	if(!visible){
		pqm_dis->clear(left, top, left+width_, top+height_);
	}
	update = true;
} 

//-------------------------------------------------------------------------
void CChartVector::refresh()
{
	
	if(!update) return;
	if(visible){
		pqm_dis->clear(left,top,left+width_,top+height_);
	}
	draw();
	pqm_dis->refresh(left,top,left+width_,top+height_);
}

//-------------------------------------------------------------------------
void CChartVector::draw()
{
	update = false;
	if(!visible) return;
	    
	draw_data();
	pqm_dis->setmode(DISOPXOR);
	draw_frame();
	pqm_dis->setmode(DISOPUN);
}

//-------------------------------------------------------------------------rrr
//绘制图标框架
void CChartVector::draw_frame()
{
	int i, wl, wr;
	
	//画边框
	for(i=0;i<border.left_width;i++) { //左边框, 上-->下
		pqm_dis->line(left-i,top,left-i,top+height_,border.color,0);
	}
	for(i=0;i<border.right_width;i++) { //右边框, 上-->下
		pqm_dis->line(left+width_+i-1,top,left+width_+i-1,top+height_,border.color,0);
	}
	wl = border.left_width>1?border.left_width-1:0;
	wr = border.right_width>1?border.right_width-1:0;
	for(i=0;i<border.top_width;i++) { //上边框, 左-->右
		pqm_dis->line(left-wl,top-i,left+width_+wr,top-i,border.color,0);
	}
	for(i=0;i<border.bottom_width;i++) { //下边框, 左-->右
		pqm_dis->line(left-wl,top+height_+i-1,left+width_+wr,top+height_+i-1,border.color,0);
	}

	//坐标轴
	i = width_/2 - axis_len;
	pqm_dis->line(left+i+8, top+height_/2, left+width_-i-8, top+height_/2, color, 0);
	i = height_/2 - axis_len;
	pqm_dis->line(left+width_/2, top+i+8, left+width_/2, top+height_-i-8, color, 0);
	
	int x, y, cl, spc, asz, csz;
	//坐标轴标题
	SLabel * pslabel;
	for(i=0;i<4;i++){
		pslabel = &axis_title[i];
		if(pslabel->self_font){
			cl = pslabel->font.color;
			spc = pslabel->font.space;
			asz = pslabel->font.asc_size;
			csz = pslabel->font.cn_size;
		}else{
			cl = font.color;
			spc = font.space;
			asz = font.asc_size;
			csz = font.cn_size;
		}
		pqm_dis->set_font(csz, asz);
		x = left + pslabel->left;
		y = top + pslabel->top;
		pqm_dis->puts(pslabel->text,x,y,cl, spc);
	}
	//基准线
	int x0, y0;
	float fi,fj;
	x0 = left + width_/2;
	y0 = top + height_/2;
	fi = axis_len*5;
	fi /= 6;
	for(i=0;i<360;i+=10){
		fj = i;
		fj = fj*PI2/360;
		x = fi*cos(fj)+0.5;
		y = -fi*sin(fj)+0.5;
		pqm_dis->set_pixel(x0+x,y0+y,color);
	}
	for(i=0;i<360;i+=120){
		fj = i;
		fj = fj*PI2/360;
		x = fi*cos(fj) + 0.5;
		y = -fi*sin(fj)+ 0.5;
		pqm_dis->line(x0,y0,x0+x,y0+y,color,3);
	}
}

//-------------------------------------------------------------------------
void CChartVector::clear()
{
	for(int i=0;i<data_num;i++){
		data_value[i].amp = 0;
		data_value[i].phs = 0;
	}
}

//-------------------------------------------------------------------------
//添加单个显示数据
void CChartVector::add_data(int indx, float amp, float phs, int color)
{
	if(indx>=data_num) return;
	data_value[indx].amp = amp;
	data_value[indx].phs = phs;
	data_color[indx] = color;

	update = true;
}

//-------------------------------------------------------------------------
//绘制显示数据
void CChartVector::draw_data()
{
	int i, k, x0, y0, x, y;
	float fi, fj;
	
	fi = 0;
	for (i=0;i<data_num;i++) {
		if (data_value[i].amp>fi) {
			fi = data_value[i].amp;
		}
	}
	if(fi) {
		axis_scale = fi*6/5;
	}
	x0 = left + width_/2;
	y0 = top + height_/2;
	if(series_arrow){ //箭形图
		for(i=0;i<data_num;i++){
			if(data_value[i].amp==0) continue;
			fi = data_value[i].amp*axis_len/axis_scale;
			fj = data_value[i].phs*PI2/360;
			pqm_dis->draw_vector(x0,y0,fi,fj,5,data_color[i]);
			//矢量标注
			x = fi*cos(fj)+0.499;
			y = -fi*sin(fj)+0.499;
			//if(i==1) printf("amp=%4.2f,phs=%4.3f,x=%d,y=%d\n",fi,fj,x,y);
			int cl, spc, asz, csz;
			SLabel * pslabel;
			pslabel = &data_label[i];
			if(pslabel->self_font){
				cl = pslabel->font.color;
				spc = pslabel->font.space;
				asz = pslabel->font.asc_size;
				csz = pslabel->font.cn_size;
			}else{
				cl = font.color;
				spc = font.space;
				asz = font.asc_size;
				csz = font.cn_size;
			}
			pqm_dis->set_font(csz, asz);
			k = abs(x)+abs(y);
			if(k){ //矢量不为零
				if(x>0){
					pslabel->left = x + 2;
				}else{
					pslabel->left = x -8;
				}
				if(y>0){
					pslabel->top = y + 8;
				}else{
					pslabel->top = y - 2;
				}
				pqm_dis->puts(pslabel->text,x0+pslabel->left,
						y0+pslabel->top,cl, spc);
			}
		}
	}
}

//-------------------------------------------------------------------------
//设置矢量标注
void CChartVector::set_data_label(int indx, const char *str)
{
	if(indx>=data_num) return;
	int i = strlen(str);
	if(data_label[indx].text!=NULL) delete [] data_label[indx].text;
	data_label[indx].text = new char[i+2];
	strcpy(data_label[indx].text, str);
}

//-------------------------------------------------------------------------
//设置轴标题
void CChartVector::set_axis_title(int indx, const char *str)
{
	if(indx>=4) return;
	int i = strlen(str);
	if(axis_title[indx].text!=NULL) delete [] axis_title[indx].text;
	axis_title[indx].text = new char[i+2];
	strcpy(axis_title[indx].text, str);
}


