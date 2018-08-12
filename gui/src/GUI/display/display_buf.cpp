#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cmath>

#include<sys/ioctl.h>
#include<linux/fb.h>
#include<unistd.h>
#include<fcntl.h>

#include "display.h"
#include "vga.h"
#include "vga_color.h"

using namespace std;


//三角图标
static const char ArrowIcon[11][11] = {
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			 	{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0},
				{0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0},
				{0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} };
//箭头图标
static const char ArrowIcon1[11][11] = {
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0},
			 	{0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0},
				{0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0},
				{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
				{0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0},
				{0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0},
				{0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0},
				{0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} };

//双向箭头图标
static const char ArrowIconD[11][11] = {
				{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0},
				{0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0},
			 	{0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0},
				{0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0},
				{0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0} };

//#define gb16_index(l,r) ((((l-0xa1)&0x7f)*94+((r-0xa1)&0x7f)) << 5)
#define gb16_index(l,r) (((l-0xa1)*94+(r-0xa1))*32)
#define gb14_index(l,r) (((l-0xa1)*94+(r-0xa1))*28)
#define gb12_index(l,r) (((l-0xa1)*94+(r-0xa1))*24)
#define en16_index(c)    (c * 16+0)
#define en12_index(c)    (c * 12+0)
#define en10_index(c)    (c * 10+1)

unsigned char * DisplayBuf::ascfont_8x16(NULL);
unsigned char * DisplayBuf::ascfont_8x12(NULL);
unsigned char * DisplayBuf::ascfont_8x10(NULL);
unsigned char * DisplayBuf::hzfont_16x16(NULL);
unsigned char * DisplayBuf::hzfont_14x14(NULL);
unsigned char * DisplayBuf::hzfont_12x12(NULL);

//-------------------------------------------------------------------------
//Initialize vga display
DisplayBuf::DisplayBuf()
{
	int bits;   //bits per pixel
	if (vga_init(&disbufw_, &disbufh_, &bits)<0) exit(1);
	vga_color().SetColorSys(bits);
	dis_buf_ = new uint32_t[disbufw_*disbufh_+disbufw_*16];   //防止在最后一行超出显示宽度范围后，造成内存溢出
	op_mode_ = DISOPUN;
}
//-------------------------------------------------------------------------
//close the framebuffer description
DisplayBuf::~DisplayBuf()
{
	vga_close();
	delete [] dis_buf_;
}


/* -----------------------------------------------------------------------------
Description:Clear rectangle
Input:      x1,y1 -- top left corner (include)
            x2,y2 -- bottom right corner (exclude)
                     width=x2-x1; height=y2-y1;
----------------------------------------------------------------------------- */
void DisplayBuf::clear(int x1, int y1, int x2, int y2, int color)
{
	rectangle(x1, y1, x2, y2, color);
}
void DisplayBuf::clear(int color)
{
	memset(dis_buf_, color, disbufw_*disbufh_*sizeof(uint32_t));
}

/* -----------------------------------------------------------------------------
Description:Refresh rectangle area
Input:      x1,y1 -- top left corner (include)
            x2,y2 -- bottom right corner (exclude)
                     width=x2-x1; height=y2-y1;
----------------------------------------------------------------------------- */
void DisplayBuf::refresh(int x1, int y1, int x2, int y2)
{
	vga_update(dis_buf_, x1, y1, x2, y2);
	//printf("refresh part!!\n");
}
void DisplayBuf::refresh()
{
	vga_update(dis_buf_, 0, 0, disbufw_, disbufh_);
	//printf("refresh all!!\n");
}

//-------------------------------------------------------------------------
void DisplayBuf::drawchar(unsigned char *pmode, int w, int h,
		int x, int y, int fg)
{
	int i,xi, yi, wi, hi, posi, posj;

	i = (w+7)/8;
	for (hi = 0, yi = y; hi < h; hi++, yi++){
		if(yi<0) continue;
		if(yi>disbufh_) break;
		posi = hi*i; //字模当前点的位置
		posj = yi*disbufw_; //显示缓存当前点的位置
		for (wi = 0, xi = x; wi < w; wi++, xi++){
			if (((pmode[posi + wi/8] >> (7-wi%8)) & 0x01) != 0){
				if(xi<disbufw_) dis_buf_[posj+xi] = fg;
			}
		}
	}
}

//draw rectangle frame
// its top left corner at (x1,y1). 
// include (x1,y1), exclude (x2,y2).
void DisplayBuf::rectframe(int x1, int y1, int x2, int y2, int color)
{
	line(x1,y1,x1,y2,color,0);// draw left line. 
	line(x1,y1,x2,y1,color,0);// draw top line. 
	line(x2-1,y1,x2-1,y2,color,0);// draw right line. 
	line(x1,y2-1,x2,y2-1,color,0);// draw bottom line. 
}

//read data from rect to tobuf
void DisplayBuf::read_rect(int x1, int y1, int x2, int y2, int *tobuf)
{
	int i, j;

	for(i=y1; i<y2; i++){
		for(j=x1;j<x2;j++){
			*tobuf = dis_buf_[i*disbufw_+j];
			tobuf ++;
		}
	}
}

//write data from frombuf to rect
void DisplayBuf::write_rect(int x1, int y1, int x2, int y2, int *frombuf)
{
	int i, j;

	for(i=y1; i<y2; i++){
		for(j=x1;j<x2;j++){
			dis_buf_[i*disbufw_+j] = *frombuf;
			frombuf ++;
		}
	}
}

/* -----------------------------------------------------------------------------
Description:draw rectangle
Input:      x1,y1 -- top left corner (include)
            x2,y2 -- bottom right corner (exclude)
                     width=x2-x1; height=y2-y1;
----------------------------------------------------------------------------- */
void DisplayBuf::rectangle(int x1, int y1, int x2, int y2, int color)
{
	int i, j;
	for(i=y1; i<y2; i++){
		for(j=x1;j<x2;j++){
			dis_buf_[i*disbufw_+j] = color;
		}
	}
}

/* -----------------------------------------------------------------------------
Description:Same as rectangle, is xor
----------------------------------------------------------------------------- */
void DisplayBuf::rectangle_xor(int x1, int y1, int x2, int y2, int color)
{
	int i, j;
	for(i=y1; i<y2; i++){
		for(j=x1;j<x2;j++){
			dis_buf_[i*disbufw_+j] ^= color;
		}
	}
}

/*//-------------------------------------------------------------------------
static inline void vga_set_pixelin(int x, int y)
{

	switch(logic_op){
		case LOGUN:
			dis_buf_[y*disbufw_+x] = color;
			break;
		case LOGAND:
			dis_buf_[y*disbufw_+x] = color;
			break;
		case LOGOR:
			dis_buf_[y*disbufw_+x] = color;
			break;
		case LOGXOR:
			dis_buf_[y*disbufw_+x] = color;
			break;
		default:
			break;

}*/

//-------------------------------------------------------------------------
void DisplayBuf::setmode(int type)
{
	op_mode_ = type;
}
//-------------------------------------------------------------------------
// Drawing a icon, its top left corner at (x,y)
// type:icon type
// direct:0=up,1=down,2=left,3=right
void DisplayBuf::draw_icon(int x, int y, int type, int color ,int direct)
{
	const char * pch;
	int i,j;
	
	switch(type){
		case ICON_ARROW:
			pch = &ArrowIcon[0][0];
			break;
		case ICON_ARROW1:
			pch = &ArrowIcon1[0][0];
			break;
		case ICON_ARROWD:
			pch = &ArrowIconD[0][0];
			break;
		default:
			return;
			break;
	
	}
	switch(direct){
		case 0: //UP
			for(i=y;i<y+11;i++){
				for(j=x;j<x+11;j++){
					if(*pch){
						dis_buf_[i*disbufw_+j] = color;
					}
					pch ++;	
				}
			}
			break;
		case 1: //DOWN
			pch += 120;
			for(i=y;i<y+11;i++){
				for(j=x;j<x+11;j++){
					if(*pch){
						dis_buf_[i*disbufw_+j] = color;
					}
					pch --;
				}
			}
			break;
		default:
			break;
	}
}

/*!
Drawing a picture in any angle

    Input:  x,y -- origin
            w,h -- picture width & height
            picbuf -- picture buffer, 0=none 
            rad -- angle of picture. unit:radian
            type -- 0=x,y is left&center, 1=x,y is left&top; 2=x,y is left&bottom
*/
/* void DisplayBuf::draw_picture(int x, int y, int w, int h, const DISBUF_TYPE* picbuf, double rad, int type)
{
	int i, j, k;
	float fi, fj;
	const DISBUF_TYPE* pbuf;
	int x0,y0, x1,y1;
	float fx,fy;
	float kx1,ky1, kx2,ky2;
	float *pbufx = new float[w];
	float *pbufy = new float[w];
	
	//rad = 0.785398;
	kx2 = sin(-rad);
	ky2 = cos(-rad);
	kx1 = cos(rad);
	ky1 = sin(rad);
	//printf("kx2=%5.4f,ky2=%5.4f,kx1=%5.4f,ky1=%5.4f\n",kx2,ky2,kx1,ky1);
	if(type==0){
		x0 = 0; y0 = h/2;
	} else if (type==1) {
		x0 = 0; y0 = 0;
	} else {
		x0 = 0; y0 = h;
	}
	for(j=0;j<w;j++){
		pbufx[j] = (j-x0)*kx1;
		pbufy[j] = (j-x0)*ky1;
		//printf("pbufx[%d]=%5.4f, pbufy[%d]=%5.4f\n",j,pbufx[j],j,pbufy[j]);
	}
	for(i=0;i<h;i++){
		for(k=0;k<2;k++){
			int wi = w - k;
			int ki = k*2 - 1;
			fj = y0 - i;
			if(2*i+k>0&&2*i+k<2*h-1){
				fj -= ki*0.25;
			}
			if(2*i+k==0){ //第一条线
				fx = kx1/4;
				fy = ky1/4;
			}else{
				fx = k*kx1/2;
				fy = k*ky1/2;
			}
			fx += fj*kx2;
			fy += fj*ky2;
			pbuf = picbuf + i*w;
			for(j=0;j<wi;j++){
				fi = pbufx[j] + fx;
				x1 = fi>=0?fi+0.49:fi-0.49;
				fi = pbufy[j] + fy;
				y1 = fi>=0?fi+0.49:fi-0.49;
				if(*pbuf){
					dis_buf_[(y-y1)*disbufw_+x+x1] = COLOR_DEFAULT;
				}
				pbuf++;
			}
		}
		//printf("\n");
	}
	delete [] pbufx;
	delete [] pbufy;
}*/

//-------------------------------------------------------------------------
// Drawing a line in any angle
// type=0,left&centre=x,y; =1,left&top=x,y; =2,left&bottom=x,y
// rad:unit is radian 弧度
void DisplayBuf::draw_vector(int x, int y, float val, double rad, int width, int color)
{
	int i, j, k;
	float fi,fj;
	
	//初始化起始点坐标存放数组
	int *pbufx = new int[width*2];
	int *pbufy = new int[width*2];
	
	//把弧度换算为0～2PI范围内
	while(rad>2*M_PI){
		rad -= 2*M_PI;
	}
	//计算矢量终点的相对坐标
	int x1, y1;
	fi = val*cos(rad);
	x1 = fi>=0?fi+0.499:fi-0.499;
	fi = val*sin(rad);
	y1 = fi>=0?fi+0.499:fi-0.499;
	
	int a1;
	double di = M_PI/2;
	if(rad<=di||rad>2*di&&rad<=3*di){ //在第一、三象限
		a1 = 0;
	}else{	//在第二、四象限
		a1 = 1;
	}

	//根据宽度、斜率计算起始点坐标组
	float kx2 = sin(-rad);
	float ky2 = cos(-rad);
	int xk,yk, xj,yj;
	int num = 0; //初始化起始点的数目
	for(i=0;i<width;i++){
		fi = i*kx2;
		xj = fi>=0?fi+0.499:fi-0.499;
		fi = i*ky2;
		yj = fi>=0?fi+0.499:fi-0.499;
		if(i==0||xk!=xj||yk!=yj){//是第一点||与前一点坐标不同
			pbufx[num] = xj;
			pbufy[num] = yj;
			num ++;
			if(xk!=xj&&yk!=yj&&i!=0){//与前一点不在一条直线上
				pbufx[num] = a1==0?xj:xk;
				pbufy[num] = a1==0?yk:yj;
				num ++;
			}
			xk = xj; yk = yj;
		}
	}
	//计算起始中点的坐标
	k = width/2;
	fi = k*kx2;
	xj = fi>=0?fi+0.499:fi-0.499;
	fi = k*ky2;
	yj = fi>=0?fi+0.499:fi-0.499;
	//画线
	for(i=0;i<num;i++){
		xk = pbufx[i] - xj;
		yk = pbufy[i] - yj;
		line(x+xk, y-yk, x+x1+xk, y-y1-yk, color, 0);
	}
	delete [] pbufx;
	delete [] pbufy;
}

//-------------------------------------------------------------------------
// Drawing a line from start point to end point in graphic buffer 
// type: 0 solid; 1 short dash; 2 dash; 3 dot 
//include (x1,y1), exclude (x2,y2);
void DisplayBuf::line(int x1, int y1, int x2, int y2,	int color ,int type)
{
	int k,l,kc,lc;
	int kx,ky,x,y, hlfkx, hlfky;
	int akx,aky,sx,sy;

	kx=x2-x1;
	ky=y2-y1;
	if(kx<0){
		akx = -kx;
		sx = -1;
	}else{
		akx = kx;
		sx = 1;
	}
	if(ky<0){
		aky = -ky;
		sy = -1;
	}else{
		aky = ky;
		sy = 1;
	}

	kc = 3;
	switch(type){
		case 0: //实线
			lc = 6000;
			break;
		case 1: //短短划线
			lc = 4;
			break;
		case 2: //短划线
			lc = 9;
			break;
		case 3: //点线
			lc = 1;
			kc = 3;
			break;
		default:
			break;
	}
	l = lc;
	k = kc;

	ky *=8;
	kx *=8;
	hlfky = sx*ky/2;
	hlfkx = sy*kx/2;
	if(akx > aky){
		if(op_mode_==DISOPUN){
			for(x=0;x<akx;x++){
				y = (x*sx*ky + hlfkx)/kx;
				if(l-->=1){
					dis_buf_[(y1+y)*disbufw_+x1+x*sx] = color;
					k = kc;
				}else{
					if(k--<1){
						l=lc;
					}
				}
			}
		}else if(op_mode_==DISOPXOR){
			for(x=0;x<akx;x++){
				y = (x*sx*ky + hlfkx)/kx;
				if(l-->=1){
					dis_buf_[(y1+y)*disbufw_+x1+x*sx] ^= color;
					k = kc;
				}else{
					if(k--<1){
						l=lc;
					}
				}
			}
		}
	}else{
		if(op_mode_==DISOPUN){
			for(y=0;y<aky;y++){
				x = (y*sy*kx + hlfky)/ky;
				if(l-->=1){
					dis_buf_[(y1+y*sy)*disbufw_+x1+x] = color;
					k = kc;
				}else{
					if(k--<1){
						l=lc;
					}
				}
			}
		}else if(op_mode_==DISOPXOR){
			for(y=0;y<aky;y++){
				x = (y*sy*kx + hlfky)/ky;
				if(l-->=1){
					dis_buf_[(y1+y*sy)*disbufw_+x1+x] ^= color;
					k = kc;
				}else{
					if(k--<1){
						l=lc;
					}
				}
			}
		}
	}
	//dis_buf_[y2*disbufw_+x2] = color;
}

// Drawing a wide pixel in graphic buffer 
void DisplayBuf::set_wpixel(int x, int y, int color )
{
	dis_buf_[(y-1)*disbufw_+x] = color;
	dis_buf_[y*disbufw_+x] = color;
	dis_buf_[(y+1)*disbufw_+x] = color;
}
// Drawing a pixel in graphic buffer 
void DisplayBuf::set_pixel(int x, int y, int color )
{
	dis_buf_[y*disbufw_+x] = color;
}

/* Drawing a wide line from start point to end point in graphic buffer */
/* mode: constant to xor;  */
// width = 2n+1;
void DisplayBuf::wline(int x1, int y1, int x2, int y2, int color, int width)
{
	int kx,ky,x,y,w;
	int akx,aky,sx,sy, hlfkx, hlfky;

	
	kx=x2-x1;
	ky=y2-y1;
	if(kx<0){
		akx = -kx;
		sx = -1;
	}else{
		akx = kx;
		sx = 1;
	}
	if(ky<0){
		aky = -ky;
		sy = -1;
	}else{
		aky = ky;
		sy = 1;
	}

	ky *=8;
	kx *=8;
	hlfky = sx*ky/2;
	hlfkx = sy*kx/2;
	while(width>3){
		w = width/2;
		if(akx > aky){
			for(x=0;x<akx;x++){
				y = (x*sx*ky + hlfkx)/kx;
				dis_buf_[(y1+y-w)*disbufw_+x1+x*sx] = color;
				dis_buf_[(y1+y+w)*disbufw_+x1+x*sx] = color;
			}
			dis_buf_[(y2-w)*disbufw_+x2] = color;
			dis_buf_[(y2+w)*disbufw_+x2] = color;
		}else{
			for(y=0;y<aky;y++){
				x = (y*sy*kx + hlfky)/ky;
				dis_buf_[(y1+y*sy)*disbufw_+x1+x-w] = color;
				dis_buf_[(y1+y*sy)*disbufw_+x1+x+w] = color;
			}
			dis_buf_[y2*disbufw_+x2-w] = color;
			dis_buf_[y2*disbufw_+x2+w] = color;
		}
		width -= 2;
	}
	if(akx>aky){
		for(x=0;x<akx;x++){
			y = (x*sx*ky + hlfkx)/kx;
			dis_buf_[(y1+y-1)*disbufw_+x1+x*sx] = color;
			dis_buf_[(y1+y)*disbufw_+x1+x*sx] = color;
			dis_buf_[(y1+y+1)*disbufw_+x1+x*sx] = color;
		}
		dis_buf_[(y2-1)*disbufw_+x2] = color;
		dis_buf_[y2*disbufw_+x2] = color;
		dis_buf_[(y2+1)*disbufw_+x2] = color;
	}else{
		for(y=0;y<aky;y++){
			x = (y*sy*kx + hlfky)/ky;
			dis_buf_[(y1+y*sy)*disbufw_+x1+x-1] = color;
			dis_buf_[(y1+y*sy)*disbufw_+x1+x] = color;
			dis_buf_[(y1+y*sy)*disbufw_+x1+x+1] = color;
		}
		dis_buf_[y2*disbufw_+x2-1] = color;
		dis_buf_[y2*disbufw_+x2] = color;
		dis_buf_[y2*disbufw_+x2+1] = color;
	}
}

//-------------------------------------------------------------------------
//Open the specific library of char
void DisplayBuf::input_font_lib(int type)
{
    FILE *f_strm;
    char *filename;

    switch(type){
    	case CHINESE_16X16:
            if(hzfont_16x16!=NULL) return;
            filename = "./chs16.fon";
            break;
    	case CHINESE_14X14:
            if(hzfont_14x14!=NULL) return;
            filename = "./chs14.fon";
            break;
    	case CHINESE_12X12:
            if(hzfont_12x12!=NULL) return;
            filename = "./chs12.fon";
            break;
        case ASC_8X16:
            if(ascfont_8x16!=NULL) return;
            filename = "./asc16.fon";
            break;
        case ASC_8X12:
            if(ascfont_8x12!=NULL) return;
            filename = "./asc12.fon";
            break;
        case ASC_8X10:
            if(ascfont_8x10!=NULL) return;
            filename = "./asc10.fon";
            break;
       default: 
           return;
           break;
    }
    f_strm = fopen(filename, "rb");
    if(f_strm==NULL){
	printf("file %s open error!\n", filename);	
    	return;
    }
    unsigned char * chp;
    fseek(f_strm, 0, SEEK_END);
    unsigned long flen = ftell(f_strm);
    chp = new unsigned char[flen+1];
    fseek(f_strm, 0, SEEK_SET);
    size_t sz = fread((char*)chp, 1, flen, f_strm);
    if(sz<flen){
        printf("Read %s error!", filename);
        delete [] chp;
    	return;
    }
    switch(type){
    	case CHINESE_16X16:
            hzfont_16x16 = chp;
            break;
    	case CHINESE_14X14:
            hzfont_14x14 = chp;
            break;
    	case CHINESE_12X12:
            hzfont_12x12 = chp;
            break;
        case ASC_8X10:
            ascfont_8x10 = chp;
            break;
        case ASC_8X12:
            ascfont_8x12 = chp;
            break;
        case ASC_8X16:
            ascfont_8x16 = chp;
            break;
        default: break;
    }
    fclose(f_strm);
}

//-------------------------------------------------------------------------
//Set font size of chinese and ASCII char
void DisplayBuf::set_font(int chs, int asc)
{
	chinese_font = chs;
	ascii_font = asc;
}

//-------------------------------------------------------------------------
//Draw a ASCII char at size of 8x16.
//its bottem left corner at (x0,y0); its ISN is 0xc
void DisplayBuf::ascii_char_8x16(unsigned char c, int x0, int y0, int color)
{
	if(ascfont_8x16!=NULL){
		unsigned char *p= ascfont_8x16 + en16_index(c);
	
		drawchar(p,8,16,x0,y0-16,color);
	}
}

//Draw a ASCII char at size of 8x12.
//its bottem left corner at (x0,y0); its ISN is 0xc
void DisplayBuf::ascii_char_8x12(unsigned char c, int x0, int y0, int color)
{
	if(ascfont_8x12!=NULL){
		unsigned char *p= ascfont_8x12 + en12_index(c);
	
		drawchar(p,8,12,x0,y0-10,color);
	}
}

//Draw a ASCII char at size of 8x8.
//its bottem left corner at (x0,y0); its ISN is 0xc
void DisplayBuf::ascii_char_8x10(unsigned char c, int x0, int y0, int color)
{
	if(ascfont_8x10!=NULL){
		unsigned char *p= ascfont_8x10 + en10_index(c);
	
		drawchar(p,8,8,x0,y0-8,color);
	}
} 

//Draw a chinese char at size of 16x16.
//its bottem left corner at (x0,y0); its ISN is 0xc1c2
void DisplayBuf::chinese_char_16x16(unsigned char c1, unsigned char c2,
				int x0, int y0,int color)
{
	if(hzfont_16x16!=NULL){
		unsigned char *p = hzfont_16x16 + gb16_index(c1, c2);

		drawchar(p,16,16,x0,y0-16,color);
	}
}

//Draw a chinese char at size of 14x14.
//its bottem left corner at (x0,y0); its ISN is 0xc1c2
void DisplayBuf::chinese_char_14x14(unsigned char c1, unsigned char c2,
				int x0, int y0,int color)
{
	if(hzfont_14x14!=NULL){
		unsigned char *p = hzfont_14x14 + gb14_index(c1, c2);

		drawchar(p,14,14,x0,y0-14,color);
	}
}

//Draw a chinese char at size of 12x12.
//its bottem left corner at (x0,y0); its ISN is 0xc1c2
void DisplayBuf::chinese_char_12x12(unsigned char c1, unsigned char c2,
				int x0, int y0,int color)
{
	if(hzfont_12x12!=NULL){
		unsigned char *p = hzfont_12x12 + gb12_index(c1, c2);

		drawchar(p,12,12,x0,y0-12,color);
	}
}

// 
// Draw Chinese and Ascii string int vertical, its top left corner at (x,y). 
// spc:字符间的间距调整，默认为0
void DisplayBuf::vputs(const char *pnt,int x, int y, int color, int spc)
{
	int i, k, n;
	char stri[3];
	const unsigned char * pci;
	
	k = strlen(pnt);
	//printf("strlen=%d\n",k);
	pci = (unsigned char*)pnt;
	n = 0; i = 0;
	while(n<k){
		if(*pci>=0xa1){
			stri[0] = *pci;
			pci ++;
			stri[1] = *pci;
			pci ++;
			stri[2] = 0;
			n += 2;
			i += chinese_font;
		}else{
			stri[0] = *pci;
			pci ++;
			stri[1] = 0;
			n ++;
			i += ascii_font;
		}
		pqm_dis->puts(stri, x, y+i, color, spc);
		i += spc;
	}

}
//Description:  Draw Chinese and Ascii string, its bottem left corner at (x,y). 
//Input:        fg - frontgroud color, 字体颜色
//              spc - 字符间的间距调整，默认为0
void DisplayBuf::puts(const char *pnt,int x, int y, int fg, int spc)
{
	unsigned char tmp[2];
	int i;
	i = 0;
	while(pnt[i]){
		if((unsigned char)pnt[i]>=0xa1){
			tmp[0]=(unsigned char)pnt[i];
			tmp[1]=(unsigned char)pnt[i+1];
			switch(chinese_font){
				case 16:
					chinese_char_16x16(tmp[0],tmp[1],x,y,fg);
					x +=16;
					break;
				case 14:
					chinese_char_14x14(tmp[0],tmp[1],x,y,fg);
					x +=14;
					break;
				default:
					chinese_char_12x12(tmp[0],tmp[1],x,y+1,fg);
					x +=12;
					break;
			}
			i+=2;
		}else{
			tmp[0]=(unsigned char)pnt[i];
			switch(ascii_font){
				case 16:
					ascii_char_8x16(tmp[0],x,y,fg);
					x +=7 ;
					break;
				case 12:
					ascii_char_8x12(tmp[0],x,y,fg);
					x +=7 ;
					break;
				default:
					ascii_char_8x10(tmp[0],x,y,fg);
					x +=7 ;
					break;
				}
			i++;
		}
		x += spc;
	}
}
