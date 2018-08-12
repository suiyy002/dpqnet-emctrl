#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#include "setform.h"
#include "form/mainframe.h"
//#include "form/form.h"
#include "viewset.h"
#include "display/vga_color.h"

CSetForm * set_form;

//-----------------------------------------------------------------------
CSetForm::CSetForm()
{
	left = 0;
	top = main_frame->dis_top_;
	width = main_frame->width();
	height = main_frame->dis_height_;
	color = vgacolor(kVGA_Default);
    int bv = 0; //vertical border width
    int bh = 1; //horizontal border width
	border.left_width = bv;
	border.top_width = bh;
	border.right_width = bv;
	border.bottom_width = bh;
	border.color = vgacolor(kVGA_Default);
    int vw = 0; //ViewSet width
    int vh = 1; //ViewSet height
	if (pqm_dis->disbufw()==480) {  //480*272
	    vw = 360;
	    vh = 192;
	} else {    //320*240
	    vw = 300;
	    vh = 168;
	}
	set_view = new CSetView(vw,vh);
	set_view->set_left(left + (width-vw)/2);
	set_view->set_top(top + (height-vh)/2);
	
	update = true;
}

//-----------------------------------------------------------------------
CSetForm::~CSetForm()
{
	delete set_view;
}

//-----------------------------------------------------------------------
void CSetForm::draw()
{
	int i, wl, wr;

	//���߿�
	for(i=0;i<border.left_width;i++) { //��߿�, ��-->��
		pqm_dis->line(left-i,top,left-i,top+height,border.color,0);
	}
	for(i=0;i<border.right_width;i++) { //�ұ߿�, ��-->��
		pqm_dis->line(left+width+i-1,top,left+width+i-1,top+height,border.color,0);
	}
	wl = border.left_width>1?border.left_width-1:0;
	wr = border.right_width>1?border.right_width-1:0;
	for(i=0;i<border.top_width;i++) { //�ϱ߿�, ��-->��
		pqm_dis->line(left-wl,top-i,left+width+wr,top-i,border.color,0);
	}
	for(i=0;i<border.bottom_width;i++) { //�±߿�, ��-->��
		pqm_dis->line(left-wl,top+height+i-1,left+width+wr,top+height+i-1,border.color,0);
	}
	
	set_view->view_set();
	update = false;
}

//-------------------------------------------------------------------------
//typeˢ�����ͣ�=1:ȫ��ˢ��
void CSetForm::refresh(int type)
{
	if(type){ //��ʾ��ȫ��ˢ��
		draw();
	}else if(update){ //����ȫ��ˢ��
		set_view->view_set();
	}
	update = false;
}
