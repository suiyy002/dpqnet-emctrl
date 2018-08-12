#ifndef CONTROL_H
#define CONTROL_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    pthread_mutex_t mutex; //互斥锁变量
    pthread_cond_t cond; //条件变量
    int active;  //=QUITCMD,相关线程结束
} data_control;

typedef enum { kTTDisplay, kTTSave, kTTMain } ThreadType;
//enum ThreadType { kTTDisplay, kTTSave };
int control_init(data_control *mycontrol);
int control_destroy(data_control *mycontrol);
int control_activate(data_control *mycontrol);
int control_deactivate(data_control *mycontrol);
int notice_pthread(ThreadType type, int major, int minor, void *point);
void notice_clrq(void * pthnode);

#define QUITCMD 0xabcd

#define dabort() \
 {  printf("Aborting at line %d in source file %s\n",__LINE__,__FILE__); abort(); }

#ifdef __cplusplus
}
#endif

#endif /*CONTROL_H*/

