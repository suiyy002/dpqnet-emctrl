#include <stdio.h>
#include <unistd.h>
//#include <asm/io.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "keyboard.h"


//type:0,LED_NUM; 1,LED_CAP; 2,LED_SCR 
//on_off:0,off; 1,on
void set_kb_leds(int type, int on_off)
{
	unsigned char oleds, nleds = 0;
    unsigned long li;
/*	if (ioctl(0, KDGETLED, &li)) {
		perror("KDGETLED");
		fprintf(stderr, "Error reading current led setting. Maybe stdin is not a VT?\n");
		return;
	}*/
	if (ioctl(0, KDGKBLED, &oleds)) {
		perror("KDGETLED");
		fprintf(stderr, "Error reading current led setting. Maybe stdin is not a VT?\n");
		return;
	}
	//oleds = li&0xff;
	switch(type){
		case 0:	nleds = LED_NUM;
			break;
		case 1:	nleds = LED_CAP;
			break;
		case 2:	nleds = LED_SCR;
		default:break;
	}
	printf("oleds=%x nleds=%x\n", oleds, nleds);
	if(on_off){
		oleds |= nleds;
	}else{
		oleds &= (~nleds);
	}
	
	if (ioctl(0, KDSKBLED, oleds)) {
		perror("KDSKBLED");
		return;
		//exit(1);
	}
}

int key_remap(int keynum)
{
	switch(keynum){
		case KEY_dotMAP:
			keynum = KEY_dot;
			break;
		case KEY_0MAP:
			keynum = KEY_0;
			break;
		case KEY_ESCMAP:
			keynum = KEY_ESC;
			break;
		case KEY_HMSMAP:
			keynum = KEY_HMS;
			break;
		case KEY_PHSMAP:
			keynum = KEY_PHS;
			break;
		case KEY_SETMAP:
			keynum = KEY_SET;
			break;
		case KEY_OTHMAP:
			keynum = KEY_OTH;
			break;
		default:
			break;
	}
	return keynum;
}

