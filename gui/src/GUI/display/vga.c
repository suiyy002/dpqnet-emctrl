
/* File: vga.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
//#include <sys/io.h>
#include <linux/fb.h>
#include <stdint.h>

//#include <asm/io.h>
//#include <asm/system.h>

#include "vga.h"

//globals

static size_t map_len_;     //map memory size in bytes
static uint8_t *fb_mem_;    //framebuffer memory
static int fb_mem_sz_;      //size of framebuffer memory in bytes
static int line_len_b_;     //length of a line in bytes
static int line_len_p_;     //length of a line in pixels
static int bits_per_pixel_;
static int bytes_per_pixel_;

const char FB_DEV[] = "/dev/fb0";//Can't be:(const char *FB_DEV="/dev/fb0")

/*!
initialize VGA/(Video Graphics Array)

    Output: dis_w -- width of vga in pixel
            dis_h -- height of vga in pixel
            bits -- bits per pixel
    Return: 0=success, -1=failure
*/
int vga_init(int *dis_w, int *dis_h, int *bits)
{
    /* This call is mostly for the i386 architecture
    if (ioperm(0x3c0, 0x10, 1) == -1) {
        printf("Cannot init vga mode.\n");
        exit(1);
    } */
	
	int fd; // file descriptor of framebuffer
	fd = open(FB_DEV, O_RDWR);
	if(!fd){
		printf("Cannot open framebuffer device %s.\n", FB_DEV);
		return -1;
	}

    int retval = 0;
    do {
    	struct fb_fix_screeninfo finfo;
    	if(ioctl(fd, FBIOGET_FSCREENINFO, &finfo)){
    		printf("Error reading fixed information.\n");
    		retval = -1; break;
    	}
    	printf("fixinfo-cardid		:%s\n",finfo.id);
    	printf("fixinfo-smemstart	:0x%x\n",finfo.smem_start);
    	printf("fixinfo-smemlen		:0x%x\n",finfo.smem_len);
    	printf("fixinfo-type		:%d\n",finfo.type);     //0=Packed pixels
    	printf("fixinfo-visual		:%d\n",finfo.visual);   //2=true color
    	printf("fixinfo-linelength 	:%d\n\n",finfo.line_length);    //length of a line in bytes
    
    	struct fb_var_screeninfo vinfo;
    	if(ioctl(fd,FBIOGET_VSCREENINFO, &vinfo)){
    		printf("Error reading variable information.\n");
    		retval = -1; break;
    	}
    	printf("varinfo-xres        :%d\n", vinfo.xres);
    	printf("varinfo-yres        :%d\n", vinfo.yres);
    	printf("varinfo-xresvirtual :%d\n", vinfo.xres_virtual);
    	printf("varinfo-yresvirtual :%d\n", vinfo.yres_virtual);
    	printf("varinfo-xoffset     :%d\n", vinfo.xoffset);
    	printf("varinfo-yoffset     :%d\n", vinfo.yoffset);
    	printf("varinfo-bits_per_pixel	:%d\n", vinfo.bits_per_pixel);
    	//printf("varinfo-width     :%d\n", vinfo.width);   // width of picture in mm
    	//printf("varinfo-height    :%d\n\n", vinfo.height);// height of picture in mm
    
    	bytes_per_pixel_ = vinfo.bits_per_pixel/8;
        line_len_b_ = finfo.line_length;
        line_len_p_ = vinfo.xres;
        bits_per_pixel_ = vinfo.bits_per_pixel;
        bytes_per_pixel_ = (bits_per_pixel_ - 1) / 8 + 1; //Bytes per pixel
        if(bits_per_pixel_ < 8) {
            if(8 % bits_per_pixel_) {
                printf("bits_per_pixel=%d, must be 1,2 or 4!\n", bits_per_pixel_);
        		retval = -1; break;
            }
            fb_mem_sz_ = finfo.line_length * vinfo.yres * 8 / bits_per_pixel_;
        } else {
            fb_mem_sz_ = vinfo.xres * vinfo.yres * bytes_per_pixel_;
        }
        map_len_ = finfo.smem_len;
        
        fb_mem_ = (uint8_t*)mmap(NULL, map_len_, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        if(!fb_mem_ || fb_mem_ == (unsigned char *) - 1) {
            perror("Error mmap");
    		retval = -1; break;
        }
    	vga_clear();
    	*dis_w = vinfo.xres;
    	*dis_h = vinfo.yres;
    	*bits = bits_per_pixel_;
    } while(0);

	close(fd);
    return retval;
}

void vga_close()
{
    munmap(fb_mem_, map_len_);
}

void vga_clear()
{
    memset(fb_mem_, 0, fb_mem_sz_);
}

void vga_bit_memcpy(void *to, int bx, void * from, int from_tp, int num)
{
    char * pfrom = (char*)from;
    char * pto = (char*)to;

    char lumask, rmask;

    if(bits_per_pixel_ == 1) { //1bit表示一个pixel
        lumask = 0x80;
        rmask = 0x01;
    } else if(bits_per_pixel_ == 2) { //2bit表示一个pixel
        lumask = 0xc0;
        rmask = 0x03;
    } else { //4bit表示一个pixel
        lumask = 0xf0;
        rmask = 0x0f;
    }

    int pixel_per_byte = 8 / bits_per_pixel_;
    int b_num, m_num, e_num; //pixel num in 1st byte, byte num in the middle, pixel num in last byte
    m_num = 0;
    e_num = 0;
    b_num = (pixel_per_byte - bx) % pixel_per_byte;
    if(b_num < num) {
        m_num = (num - b_num) / pixel_per_byte;
        e_num = (num - b_num) % pixel_per_byte;
    } else {
        b_num = num;
    }

    int i, j, k;
    char chi, chj;
    int pos = 0; //当前字节的位置

    if(b_num > 0) { //第一个字节
        chi = *pto;
        for(i = 0; i < b_num; i++) {
            k = (bx + i) * bits_per_pixel_; //位偏移量，从左边起
            chi &= ~(lumask >> k);
            chj = *(pfrom + i * from_tp)&rmask;
            chi |= chj << (8 - k - bits_per_pixel_);
        }
        *pto = chi;
        bx = 0; //设定最后一个字节的起始位
        pos ++;
    }

    if(m_num > 0) { //中间的字节
        for(i = 0; i < m_num; i++) {
            chi = 0;
            k = (b_num + i * pixel_per_byte) * from_tp;
            for(j = 0; j < pixel_per_byte; j++) {
                chj = *(pfrom + j * from_tp + k)&rmask;
                chi |= chj << (8 - (j + 1) * bits_per_pixel_);
            }
            *(pto + pos) = chi;
            pos ++;
        }
    }

    if(e_num > 0) { //最后一个字节
        chi = *(pto + pos);
        for(i = 0; i < e_num; i++) {
            k = i * bits_per_pixel_; //位偏移量，从左边起
            chi &= ~(lumask >> k);
            j = (i + b_num + m_num * pixel_per_byte) * from_tp;
            chj = *(pfrom + j)&rmask;
            chi |= chj << (8 - k - bits_per_pixel_);
        }
        *(pto + pos) = chi;
    }
}

void vga_memcpy(void *to, int to_tp, void * from, int from_tp, int num)
{
    int i, j, k;

    char * pfrom = (char*)from;
    char * pto = (char*)to;

    i = from_tp < to_tp ? from_tp : to_tp;
    memset(to, 0, num * to_tp);

    for(k = 0; k < i; k++) {
        for(j = 0; j < num; j++) {
            *(pto + j * to_tp + k) = *(pfrom + j * from_tp + k);
        }
    }
}

/*!
update vga display. width=x2-x1; height=y2-y1;

    Input:  x1,y1 -- topleft point(include)
            x2,y2 -- bottomright point(exclude)
*/
void vga_update(void *buf, int x1, int y1, int x2, int y2)
{
    volatile char *where;
    int y, x, i;
    uint8_t *pbuf = buf;

    pbuf += (y1*line_len_p_ + x1)*4;

    if(bits_per_pixel_ < 8) {
        for(y = y1; y < y2; y++) {
            i = 8 / bits_per_pixel_;
            where = fb_mem_ + y * line_len_b_ + x1 / i;
            vga_bit_memcpy(where, x1 % i, pbuf, 4, x2 - x1);
            pbuf += line_len_p_*4;
        }
    } else {
        if(bytes_per_pixel_ == 4) {
            for(y = y1; y < y2; y++) {
                where = fb_mem_ + y * line_len_b_ + x1 * bytes_per_pixel_;
                memcpy(where, pbuf, (x2 - x1)*bytes_per_pixel_);
                pbuf += line_len_p_*4;
            }
        } else {
            for(y = y1; y < y2; y++) {
                where = fb_mem_ + y * line_len_b_ + x1 * bytes_per_pixel_;
                vga_memcpy(where, bytes_per_pixel_, pbuf, 4, x2 - x1);
                pbuf += line_len_p_*4;
            }
        }
    }
}



