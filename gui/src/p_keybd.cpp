#include <cstdio>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include "conversion.h"
#include "pthread_mng.h"
#include "device.h"

struct termios oldstdtio,newstdtio;
//-----------------------------------------------------------------------------

void *thread_keybd(void *myarg) {
	WorkNode *todis;
	int c_ary[4], i, flg;
	//int exit_cnt=0;

	printf("key thread run...\n");
	/* next stop echo and buffering for stdin */
	tcgetattr(0,&oldstdtio);
	tcgetattr(0,&newstdtio); /* get working stdtio */
	newstdtio.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0,TCSANOW,&newstdtio);

    CleanupNode *pthnode = (CleanupNode *) myarg;
	c_ary[3] = 0;
	do{
		for(i=0;i<3;i++){
			c_ary[i] = c_ary[i+1];
		}
		c_ary[3] = getchar();
		printf("keys=%d\n", c_ary[3]);
		c_ary[3] = key_remap(c_ary[3]);
		flg = exit_flag;
        notice_pthread(kTTMain, kKeyInfo, c_ary[3], NULL);
        if (saveq.control.active == QUITCMD) break;
	}while(flg!=0xff||c_ary[3]!=KEY_dot);
	
	tcsetattr(0,TCSANOW,&oldstdtio); /* restore old tty setings */
	if (saveq.control.active != QUITCMD) {
    	pthread_mutex_lock(&cwq.control.mutex);
	    cwq.control.active=QUITCMD;
  	    pthread_mutex_unlock(&cwq.control.mutex);
    	printf("\nprogram to be terminal... by keyboard\n");
	    pthread_cond_signal(&cwq.control.cond);
	}
    notice_clrq(pthnode);
	return NULL;

}
