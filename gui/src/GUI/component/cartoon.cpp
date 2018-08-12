#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#include "component.h"
#include "display/vga_color.h"

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//		CCartoon  实现指定图形的移动
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

CCartoon::CCartoon(int w, int h)
{
	visible = true;
	left = 0;
	top = 0;
	width = w;
	height = h;
	color = vgacolor(kVGA_Default);

	dis_data_bak = new int[width*height];	
	pqm_dis->read_rect(left,top,left+width,top+height,dis_data_bak);
	update = true;
}

//-------------------------------------------------------------------------
CCartoon::~CCartoon()
{
	delete dis_data_bak;
}

//-------------------------------------------------------------------------
void CCartoon::paint()
{
	pqm_dis->line(left,top,left+width,top+height,vgacolor(kVGA_Default),3);
	//printf("x0=%d,y0=%d,x1=%d,y1=%d\n",left,top,left+width,top+height);
}

//-------------------------------------------------------------------------
void CCartoon::draw()
{
	if(update){
		pqm_dis->read_rect(left,top,left+width,top+height,dis_data_bak);
	}
	update = false;
	if(!visible) return;
	paint();
}

//-------------------------------------------------------------------------
void CCartoon::move(int x, int y)
{
	pqm_dis->write_rect(left,top,left+width,top+height,dis_data_bak);
	if(x!=left||y!=top){
		pqm_dis->refresh(left, top, left+width, top+height);
		left = x; top = y;
	}
	update = true;
}

//-------------------------------------------------------------------------
void CCartoon::set_visible(bool yn)
{
	visible = yn;
	if(!visible){
		pqm_dis->write_rect(left,top,left+width,top+height,dis_data_bak);
	}
	update = true;
}

//-------------------------------------------------------------------------
void CCartoon::refresh()
{
	if(!update) return;
	draw();
	pqm_dis->refresh(left, top, left+width, top+height);
}

