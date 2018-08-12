#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_0 	48
#define KEY_1 	49
#define KEY_2 	50
#define KEY_3 	51
#define KEY_4 	52
#define KEY_5 	53
#define KEY_6 	54
#define KEY_7 	55
#define KEY_8 	56
#define KEY_9 	57

#define KEY_ESC 119 //'w'
#define KEY_HMS 101 //'e'
#define KEY_PHS 114 //'r'
#define KEY_dot 46  //'.'
#define KEY_OTH	47  //'/'
#define KEY_SET	10 //"enter"
	
#define KEY_START 246 //

#define KEY_dotMAP 121  //'y'
#define KEY_0MAP   106  //'j' 
#define KEY_ESCMAP 112  //'p'
#define KEY_HMSMAP 91  //'['
#define KEY_PHSMAP 59  //';'
#define KEY_SETMAP 39   //'''
#define KEY_OTHMAP 32   //'Space'


void set_kb_leds(int type, int on_off);
int key_remap(int keynum);

#ifdef __cplusplus
}
#endif

#endif  //_KEYBOARD_H_

