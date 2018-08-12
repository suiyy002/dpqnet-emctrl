/*! \file display_buf.h
    \brief Display buffer.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _DISPLAY_BUF_H_
#define _DISPLAY_BUF_H_

#include <stdint.h>
//#include "vga.h"

class DisplayBuf{
public:
	DisplayBuf();
	~DisplayBuf();
	
	void set_font(int chs, int asc);
	// Draw Chinese and Ascii string, its bottem left corner at (x,y). 
	void puts(const char *pnt,int x, int y, int fg, int spc=0);
	void vputs(const char *pnt,int x, int y, int fg, int spc=0);
	void clear(int color=0);
	void clear(int x1, int y1, int x2, int y2, int color=0);
	void line(int x1, int y1, int x2, int y2, int color ,int type);
	void wline(int x1, int y1, int x2, int y2, int color, int width=3);
 	void refresh(int x1, int y1, int x2, int y2);
 	void refresh();
	void rectframe(int x1, int y1, int x2, int y2, int color);
	void rectangle(int x1, int y1, int x2, int y2, int color);
	void rectangle_xor(int x1, int y1, int x2, int y2, int color);
	// Drawing a wide pixel in graphic buffer 
	void set_wpixel(int x, int y, int color );
	void set_pixel(int x, int y, int color );
	void setmode(int type = 0);
	void draw_icon(int x, int y, int type, int color ,int direct);
        void input_font_lib(int type);
	void read_rect(int x1, int y1, int x2, int y2, int *tobuf);
	void write_rect(int x1, int y1, int x2, int y2, int *frombuf);
	// void draw_picture(int x, int y, int w, int h, const DISBUF_TYPE* picbuf, double rad, int type=0);
	void draw_vector(int x, int y, float val, double rad, int width, int color);
	
	int disbufh() { return disbufh_; };
	int disbufw() { return disbufw_; };
protected:
private:
	int chinese_font;
	int ascii_font;
	int bgcolor;    //backgroud color
	uint32_t *dis_buf_;     //display buffer
	int disbufw_, disbufh_;   //display buffer width&height in pixel

	int op_mode_;
	static unsigned char * ascfont_8x16;
	static unsigned char * ascfont_8x12;
	static unsigned char * ascfont_8x10;
	static unsigned char * hzfont_16x16;
	static unsigned char * hzfont_14x14;
	static unsigned char * hzfont_12x12;

	void drawchar(unsigned char *pmode, int w, int h,
		int x, int y, int fg);
	void ascii_char_8x16(unsigned char c, int x0, int y0, int color);
	void ascii_char_8x12(unsigned char c, int x0, int y0, int color);
	void ascii_char_8x10(unsigned char c, int x0, int y0, int color);
	void chinese_char_16x16(unsigned char c1, unsigned char c2,
				int x0, int y0,int color);
	void chinese_char_14x14(unsigned char c1, unsigned char c2,
				int x0, int y0,int color);
	void chinese_char_12x12(unsigned char c1, unsigned char c2,
				int x0, int y0,int color);
};

//-------------------------------------------------------------------------
const int CHINESE_16X16 = 0;
const int ASC_8X16 = 1;
const int CHINESE_14X14 = 2;
const int ASC_8X10 = 3;
const int CHINESE_12X12 = 4;
const int ASC_8X12 = 5;

const int DISOPUN = 0; 
const int DISOPAND = 1; 
const int DISOPOR = 2; 
const int DISOPXOR = 3; 

const int ICON_ARROW = 1; 
const int ICON_ARROW1 = 2; 
const int ICON_ARROWD = 3; 

extern DisplayBuf *pqm_dis;

#endif  // _DISPLAY_BUF_H_


