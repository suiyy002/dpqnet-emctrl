#ifndef _VGA_H
#define _VGA_H

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------
#define GRAPHICS_ADDR_REG 0x3ce         /* Graphics address register. */
#define GRAPHICS_DATA_REG 0x3cf         /* Graphics data register. */

#define SET_RESET_INDEX 0               /* Set/Reset Register index. */
#define ENABLE_SET_RESET_INDEX 1        /* Enable Set/Reset Register index. */
#define DATA_ROTATE_INDEX 3             /* Data Rotate Register index. */
#define GRAPHICS_MODE_INDEX 5           /* Graphics Mode Register index. */
#define BIT_MASK_INDEX 8                /* Bit Mask Register index. */

//-------------------------------------------------------------------------
#define __OUT1_U(s,x) \
static void out##s(unsigned x value, unsigned short port) {

#define __OUT2_U(s,s1,s2) \
__asm__ __volatile__ ("out" #s " %" s1 "0,%" s2 "1"

#define __OUT_U(s,s1,x) \
__OUT1_U(s,x) __OUT2_U(s,s1,"w") : : "a" (value), "Nd" (port)); } \
__OUT1_U(s##_p,x) __OUT2_U(s,s1,"w") __FULL_SLOW_DOWN_IO : : "a" (value), "Nd" (port));} \

#define __IN1_U(s) \
static RETURN_TYPE in##s(unsigned short port) { RETURN_TYPE _v;

#define __IN2_U(s,s1,s2) \
__asm__ __volatile__ ("in" #s " %" s2 "1,%" s1 "0"

#define __IN_U(s,s1,i...) \
__IN1_U(s) __IN2_U(s,s1,"w") : "=a" (_v) : "Nd" (port) ,##i ); return _v; } \
__IN1_U(s##_p) __IN2_U(s,s1,"w") __FULL_SLOW_DOWN_IO : "=a" (_v) : "Nd" (port) ,##i ); return _v; } \

#define __INS_U(s) \
static void ins##s(unsigned short port, void * addr, unsigned long count) \
{ __asm__ __volatile__ ("cld ; rep ; ins" #s \
: "=D" (addr), "=c" (count) : "d" (port),"0" (addr),"1" (count)); }

#define __OUTS_U(s) \
static void outs##s(unsigned short port, const void * addr, unsigned long count) \
{ __asm__ __volatile__ ("cld ; rep ; outs" #s \
: "=S" (addr), "=c" (count) : "d" (port),"0" (addr),"1" (count)); }


//-------------------------------------------------------------------------
int  vga_init(int *dis_w, int *dis_h, int *bits);
void vga_close();
void vga_clear();
void vga_update(void *buf, int x1, int y1, int x2, int y2);
	
//-------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif

#endif /* _VGA_H */


