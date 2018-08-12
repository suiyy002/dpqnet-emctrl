#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#include "component.h"
#include "display/vga_color.h"

CLabelSet::CLabelSet(int cnt)
{
	visible = true;
	allrefresh = false;
	left = 0;
	top = 0;
	width = 80;
	height = 60;
	font.asc_size = 8;
	font.cn_size = 12;
	font.space = 0;
	font.color = vgacolor(kVGA_Default);
	transparent = true;
	border.left_width = 0;
	border.top_width = 0;
	border.right_width = 0;
	border.bottom_width = 0;
	border.color = vgacolor(kVGA_Default);

	count = cnt;
	labels_ = new SLabel[count];
	for(int i=0;i<count;i++){
		labels_[i].left = 0;
		labels_[i].top = i*font.cn_size +2;
		labels_[i].rfrs_width = 0;
		labels_[i].text = NULL;
		labels_[i].auto_size = true;
		labels_[i].self_font = false;
		labels_[i].font.asc_size = 8;
		labels_[i].font.cn_size = 12;
		labels_[i].font.space = 0;
		labels_[i].font.color = vgacolor(kVGA_Default);
		labels_[i].update = true;
		calc_vwsz(i);
	}
	update = true;
}

//-------------------------------------------------------------------------
CLabelSet::~CLabelSet()
{
	for(int i=0;i<count;i++){
		if(labels_[i].text!=NULL){
			delete [] labels_[i].text;
		}
	}
	delete [] labels_;
}

//-------------------------------------------------------------------------
// 赋值函数
CLabelSet & CLabelSet::operator =(const CLabelSet &other)
{	
	//检查自赋值
	if(this == &other)
		return *this;
	
	visible = other.visible;
	allrefresh = other.allrefresh;

	left = other.left;
	top = other.top;
	width = other.width;
	height = other.height;
	memcpy(&font, &other.font, sizeof(SFont));
	transparent = other.transparent;
	bgcolor = other.bgcolor;
	memcpy(&border, &other.border, sizeof(SEdges));
	
	for(int i=0;i<count;i++){
		if(labels_[i].text!=NULL){
			delete [] labels_[i].text;
		}
	}
	delete [] labels_;
	count = other.count;
	labels_ = new SLabel[count];
	for(int i=0;i<count;i++){
		memcpy(&labels_[i], &other.labels_[i], sizeof(SLabel));
		labels_[i].text = NULL;
		calc_vwsz(i);
	}

	//返回本对象的引用
	return *this;
}	

//-------------------------------------------------------------------------
//计算显示区域的尺寸
void CLabelSet::calc_vwsz(int indx)
{
	int k, n, i;
	char *strp;
	
	strp = labels_[indx].text;
	if(strp==NULL){
		i = 0;
	}else{
		i = strlen(strp);
	}

	int cn_sz, spc;
	if(labels_[indx].self_font){ //使用私有字体
		cn_sz = labels_[indx].font.cn_size;
		spc = labels_[indx].font.space;
	}else{	//使用公共字体
		cn_sz = font.cn_size;
		spc = font.space;
	}
	
	k = 0; n = 0;
	while(k<i){
		if((unsigned char)strp[k]>=0xa1){
			n += (cn_sz+spc);
			k += 2;
		}else{
			n += (7+spc);
			k ++;
		}
	}
	if(labels_[indx].width<n){
		labels_[indx].rfrs_width = n;
	}
	labels_[indx].width = n;
	labels_[indx].height = cn_sz;
}

//-------------------------------------------------------------------------
void CLabelSet::clear()
{
	for(int i=0;i<count;i++){
		if(labels_[i].text!=NULL){
			delete [] labels_[i].text;
			labels_[i].text = NULL;
		}
	}
}

//-------------------------------------------------------------------------
void CLabelSet::set_txt(int indx, const char *str)
{
	
	if(indx>=count) return;
	int i = strlen(str);
	if(labels_[indx].text!=NULL) delete [] labels_[indx].text;
	labels_[indx].text = new char[i+2];
	strcpy(labels_[indx].text, str);

	if(labels_[indx].auto_size){
		calc_vwsz(indx);
	}
	labels_[indx].update = true;
	if(allrefresh) update = true;
}
//-------------------------------------------------------------------------
void CLabelSet::drawone(int indx)
{
	int cn_sz, asc_sz, cl, spc;
	char *strp;
	
	strp = labels_[indx].text;
	if(strp==NULL) return;
	if(labels_[indx].self_font){ //使用自己的字体
		cn_sz = labels_[indx].font.cn_size;
		asc_sz = labels_[indx].font.asc_size;
		cl = labels_[indx].font.color;
		spc = labels_[indx].font.space;
	}else{
		cn_sz = font.cn_size;
		asc_sz = font.asc_size;
		cl = font.color;
		spc = font.space;
	}
	pqm_dis->set_font(cn_sz, asc_sz);
	pqm_dis->puts(strp, left+labels_[indx].left,
			top+labels_[indx].top+cn_sz, cl, spc);
	labels_[indx].update = false;
}

//-------------------------------------------------------------------------
void CLabelSet::draw()
{
	update = false;
	if(!visible) return;
	int i, wl, wr;

	if(!transparent){
		pqm_dis->rectangle(left, top, left+width, top+height, bgcolor);
	}
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

	for(i=0;i<count;i++){
		drawone(i);
	}
}

//-------------------------------------------------------------------------
void CLabelSet::set_visible(bool yn)
{
	visible = yn;
	if(!visible){
		pqm_dis->clear(left, top, left+width, top+height);
	}
	update = true;
}
//-------------------------------------------------------------------------
void CLabelSet::refresh()
{
	if(!visible) return;
	if(update){
		pqm_dis->clear(left, top, left+width, top+height);
		draw();
		pqm_dis->refresh(left, top, left+width, top+height);
		return;
	}
	int i, lft, tp, wd, ht;
	for(i=0;i<count;i++){
		if(!labels_[i].update) continue;
		lft = left + labels_[i].left;
		tp = top +labels_[i].top;
		wd = labels_[i].rfrs_width;
		ht = labels_[i].height;
		
		pqm_dis->clear(lft, tp, lft+wd, tp+ht);
		drawone(i);
		pqm_dis->refresh(lft, tp, lft+wd, tp+ht);
		labels_[i].rfrs_width = labels_[i].width;
	}
}

//-------------------------------------------------------------------------
int CLabelSet::get_count()
{
	return count;
}

