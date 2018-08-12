#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#include "component.h"
#include "display/vga_color.h"

CLabel::CLabel(char *str)
{
	visible = true;
	left = 0;
	top = 0;
	width = 0;
	
	transparent = true;
	bgcolor = vgacolor(kVGA_Default);
	font.asc_size = 8;
	font.cn_size = 12;
	font.space = 0;
	font.color = vgacolor(kVGA_Default);
	left_space = 0;
	top_space = 0;
	auto_size = true;
	int i = strlen(str);
	caption = new char[i+1];
	strcpy(caption,str);
	text = NULL;
	update = true;

	calc_vwsz();
}

//-------------------------------------------------------------------------
CLabel::~CLabel()
{
	delete [] caption;
}

//-------------------------------------------------------------------------
// 赋值函数
CLabel & CLabel::operator =(const CLabel &other)
{	
	//检查自赋值
	if(this == &other)
		return *this;
	
	left = other.left;
	top = other.top;
	width = other.width;
	height = other.height;
	memcpy(&font, &other.font, sizeof(SFont));
	left_space = other.left_space;
	top_space = other.top_space;
	transparent = other.transparent;
	bgcolor = other.bgcolor;
	auto_size = other.auto_size;
	visible = other.visible;

	//返回本对象的引用
	return *this;
}	

//-------------------------------------------------------------------------
//计算显示区域的尺寸
void CLabel::calc_vwsz()
{
	int k, n, i;
	char *strp;
	
	k = 0;
	if(text!=NULL) k = strlen(text);
	i = strlen(caption) + k;
	strp = new char[i+2];
	strcpy(strp, caption);
	if(text!=NULL)strcat(strp, text);

	k = 0; n = 0;
	while(k<i){
		if((unsigned char)strp[k]>=0xa1){
			n += (font.cn_size+font.space);
			k += 2;
		}else{
			n += (7+font.space);
			k ++;
		}
	}
	if(width<n){
		width = n;
	}
	now_width = n;
	height = font.cn_size;
	delete [] strp;
}
//-------------------------------------------------------------------------
void CLabel::set_txt(const char *str)
{
	int i = strlen(str);
	if(text!=NULL) delete [] text;
	text = new char[i+2];
	strcpy(text, str);

	if(auto_size){
		pqm_dis->clear(left, top, left+width, top+height);
		calc_vwsz();
	}
	update = true;
}
//-------------------------------------------------------------------------
void CLabel::draw()
{
	//update = false;
	if(!visible) return;
	int i, k;
	char *strp;
	
	if(!transparent){
		pqm_dis->rectangle(left, top, left+width, top+height, bgcolor);
	}

	k = 0;
	if(text!=NULL) k = strlen(text);
	i = strlen(caption) + k;
	strp = new char[i+2];
	strcpy(strp, caption);
	if(text!=NULL) strcat(strp, text);
	pqm_dis->set_font(font.cn_size, font.asc_size);
	pqm_dis->puts(strp, left+left_space, top+font.cn_size+top_space, font.color, font.space);
	
	delete [] strp;
}
//-------------------------------------------------------------------------
void CLabel::set_visible(bool yn)
{
	visible = yn;
	if(!visible){
		pqm_dis->clear(left, top, left+width, top+height);
	}
	update = true;
}
//-------------------------------------------------------------------------
bool CLabel::get_visible()
{
	return visible;
}
void CLabel::set_update(bool yn)
{
	update= yn;
}
bool CLabel::get_update()
{
	return update;
}

void CLabel::refresh()
{
	if(!update || !visible) return;
	draw();
	pqm_dis->refresh(left, top, left+width, top+height);
	if(auto_size) width = now_width;
}


