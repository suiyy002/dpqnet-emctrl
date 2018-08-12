/*******************************************************************************
Copyright , 2011, XI'AN Boyuu Electric. Co., Ltd.
FileName: main.cpp
Author:   seapex
Version :  1.0
Date: 2011-10-15
Description: 系统开始运行时:初始化系统运行参数, 申请双口RAM、并口等资源，全局数据初始化，
             加载显示线程、按键处理线程、时间线程、通讯处理线程等多线程。
系统启动后:负责多个线程间的消息转发。
other:
*******************************************************************************/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "pthread_mng.h"
#include "device.h"
#include "view.h"
#include "utility.h"
#include "debug_info.h"

#define dabort() \
 {  printf("Aborting at line %d in source file %s\n",__LINE__,__FILE__); abort(); }

#define START_WAIT_TIME 1

PthreadQueue clrq;	//cleanup queue of pthread
PthreadQueue cwq;	//main pthread queue
PthreadQueue disq;	//display thread queue
PthreadQueue saveq;	//save thread queue


unsigned int debug_cnt, debug_cnt1 = 0, debug_cnt2;
int exit_flag = 0;

static int main_arg_num;
static char * main_arg[10];
static int numthreads_; //线程数(不包括主线程)
static int wait_for_stdy; //delay time for data is steady when system just started
static pthread_t dint_id; //pthread id of thread_daramint
static FILE *stdout_save=NULL; //The file which stdout reopen to  

SocketClient *g_sock_client = NULL;
ViewPqm * view_pqm = NULL;

#define CNNCT_MAX 5 //最多建立5个socket连接

//-----------------------------------------------------------------------------
void join_threads(void)
{
    CleanupNode *curnode;

    //printf("joining threads...\n");

    while (numthreads_) {
        pthread_mutex_lock(&clrq.control.mutex);

        /* below, we sleep until there really is a new cleanup node.*/
        while (clrq.task.head == NULL) {
            pthread_cond_wait(&clrq.control.cond, &clrq.control.mutex);
        }

        /* at this point, we hold the mutex and there is an item in the
           list that we need to process.  First, we remove the node from
           the queue.  Then, we call pthread_join() on the tid stored in
           the node.  When pthread_join() returns, we have cleaned up
           after a thread.  */
        curnode = (CleanupNode *) queue_get(&clrq.task);
        pthread_mutex_unlock(&clrq.control.mutex);
        pthread_join(curnode->tid, NULL);
        printf("joined with thread %d\n", curnode->threadnum);
        free(curnode);
        numthreads_--;
    }
}

//-----------------------------------------------------------------------------
int create_threads(void)
{
    int x;
    CleanupNode *curnode;
    int retv = 0;

    pthread_attr_t attr;
    pthread_attr_init(&attr); //初始化属性值，均设为默认值
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); //设为绑定的
    for (x = 0; x < 3; x++) {
        curnode = (CleanupNode*)malloc(sizeof(CleanupNode));
        if (!curnode) return 1;
        curnode->threadnum = x;
        switch (x) {
            case 0:
                printf("created thread %d timer\n", x);
                retv = pthread_create(&curnode->tid,
                        NULL, thread_timer, (void *) curnode);
                        //&attr, thread_timer, (void *) curnode); //将定时器线程设为与轻进程绑定 2009-2-1 seapex
                break;
            case 1:
                printf("created thread %d socket\n", x);
                retv = pthread_create(&curnode->tid,
                        NULL, thread_socket, (void *) curnode);
                break;
            case 2:
                printf("created thread %d keybd\n", x);
                retv = pthread_create(&curnode->tid,
                        NULL, thread_keybd, (void *) curnode);
                break;
            default:
                numthreads_--;
                break;
        }
        if (retv) return 1;
        numthreads_++;
    }
    return retv;
}

//-----------------------------------------------------------------------------
//初始化线程控制队列
void initialize_structs(void)
{
    printf("Initialize and active queue structs..\n");
    if (control_init(&clrq.control)) {
        dabort();
    }
    queue_init(&clrq.task);

    if (control_init(&cwq.control)) {
        control_destroy(&clrq.control);
        dabort();
    }
    queue_init(&cwq.task);

    control_activate(&cwq.control);
}

void cleanup_structs(void)
{
    control_destroy(&clrq.control);
    control_destroy(&cwq.control);
    printf("Cleanup queue structs..\n");
}

void initialize_values(void)
{
    //Redirect stdout
    char stri[36];
    if ( main_arg_num <= 1 || strcmp(main_arg[0], "dbg") ) {
        sprintf(stri, "/tmp/debug.txt");
        stdout_save = freopen(stri, "w+", stdout);
    }
    printf("Initialize values...\n");
    numthreads_ = 0;

    TimeCstInit();
    
    messageq_guic().InitQGui(CNNCT_MAX);
    g_data_buf = new DataBuf*[CNNCT_MAX];
    memset(g_data_buf, 0, sizeof(DataBuf*)*CNNCT_MAX);
    
    g_sock_client = new SocketClient(CNNCT_MAX);
    view_pqm = new ViewPqm(g_sock_client);
    char *name = "/tmp/sockgui";
    view_pqm->BindSocket(name, NULL, 0, 3, 30);
}

void cleanup_values()
{
    printf("Cleanup values..\n");
    delete view_pqm;
    delete g_sock_client;
    delete g_data_buf;
    
    TimeCstEnd();
}

//-----------------------------------------------------------------------------
void on_create()
{
    system("echo [?25l"); //禁止显示光标
    freopen(STDIN_DEV, "r", stdin); //重定向标准输入
    set_kb_leds(0, 1);//NumLock on
    set_kb_leds(1, 0);//CapsLock off
    set_kb_leds(2, 1);//ScrollLock off
    //printf("key=%x\n", getchar());
    initialize_values();
    initialize_structs();
    pthread_mutex_init(&debug_mutex, NULL); //初始化调试信息互斥锁

    // CREATION
    if (create_threads()) {
        printf("Error starting threads... cleaning up.\n");
        join_threads();
        dabort();
    }

    printf("%s\n", NowTime(0));
}
//-----------------------------------------------------------------------------
void on_terminal()
{
    time_t tmt = time(NULL);
    tm tmi; char tm_str[24];
    LocalTime(&tmi, &tmt);
    strftime(tm_str, 20, "%Y-%m-%d %H:%M:%S", &tmi);
    printf("%s\n", tm_str);

    pthread_mutex_unlock(&cwq.control.mutex);

    pthread_cancel(dint_id);
    control_deactivate(&disq.control);
    control_deactivate(&saveq.control);

    // CLEANUP
    join_threads();

    cleanup_structs();
    cleanup_values();
    pthread_mutex_destroy(&debug_mutex); //清除互斥锁mutex
    TimeCstEnd();

    fclose(stdin);
    printf("OVER ^Q^\n"); //???
    //fclose(stdin);
}

/*!
Description:This function handles the ^c, and allows us to cleanup on exit
*/
int be_end = 0;
void ctrlCfun (int i)
{
    be_end = 0x5a;
}

int main(int argc, char *argv[])
{
    WorkNode *curtask;
    int majortype, minortype;
    void * point;

    //读取命令行参数
    int i = 0;
    main_arg_num = argc;
    while (argc > 1 && i < 10) {
        main_arg[i] = argv[i+1];
        i++;
        argc--;
    }
    on_create();
    

    signal(SIGINT, ctrlCfun);  // Set the ^c catcher
    printf ("program is running, hit ^c to exit ... \n");
	view_pqm->HandleMsg(kKeyInfo, KEY_START);
    for (;;) {
        pthread_mutex_lock(&cwq.control.mutex);
        while (cwq.task.head == NULL && cwq.control.active != QUITCMD) {
            pthread_cond_wait(&cwq.control.cond, &cwq.control.mutex);
        }
        if (cwq.control.active == QUITCMD) {
            pthread_mutex_unlock(&cwq.control.mutex);
            break;
        }
        curtask = (WorkNode *) queue_get(&cwq.task);
        pthread_mutex_unlock(&cwq.control.mutex);
        majortype = curtask->major_type;
        minortype = curtask->minor_type;
        point = curtask->point;
        free(curtask);
        //printf("%s majortype=%d, minortype=%d\n", __FUNCTION__, majortype, minortype);
        switch (majortype) {
            case kPTimerInfo:
                if (stdout_save) fflush(stdout_save);
            case kKeyInfo:
            case kGUICommuCmd:
        		view_pqm->HandleMsg(majortype, minortype);
                break;
            default:
                break;
        }
        if (be_end==0x5a) {
            pthread_mutex_lock(&cwq.control.mutex);
            cwq.control.active=QUITCMD;
            pthread_mutex_unlock(&cwq.control.mutex);
        	printf("\nprogram to be terminal... by ^c\n");
        }
    }
    on_terminal();
    if (stdout_save) fclose(stdout_save);
    //system("echo [?25h"); //显示光标
}


