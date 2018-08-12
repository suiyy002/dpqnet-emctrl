/*! \file main.cpp
    \brief DPQNet300 mainboard progress.
    Copyright (c) 2016  Xi'an Boyuu Electric, Inc.
*/
#include "config.h"
#include "spi_dev.h"
#include "parse_option.h"
#include "spi_test.h"
#include "pthread_mng.h"
#include "watchdog.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

using namespace std;

static int for_debug_ = 0;  //0:close watchdog

#define dabort() \
 {  printf("Aborting at line %d in source file %s\n",__LINE__,__FILE__); abort(); }

PthreadQueue g_clrq;	//cleanup queue of pthread
PthreadQueue g_mainq;	//main pthread queue
PthreadQueue g_saveq;	//save thread queue
pthread_mutex_t store_mutex; //存储操作互斥锁变量

unsigned int p_comm_cnt, p_socket_cnt, p_daramp_cnt,  p_darami_cnt, p_sockdis_cnt; //线程监视计数

SocketClient *g_sock_client = NULL;

#define CNNCT_MAX 5 //最多建立5个socket连接

/*!
test module function

    Return: <0=exception; 0,1=normal exit; >1,continue;
*/
int TestModule (int argc, char *argv[])
{
    ParseOption parse_opt;
    int ret = parse_opt.Parse(argc, argv);
    if (ret < 0) return ret;

    SpiTest spi_test;

    switch (parse_opt.cmd()) {
        case kMainProg:
            for_debug_ = parse_opt.debug();
            return ret;
            break;
        case kTestSpi:
            spi_test.SimTest(parse_opt.spi_device(), parse_opt.spi_speed(), parse_opt.spi_bpw(),
                             parse_opt.spi_mode(), parse_opt.pst_type(), parse_opt.sim_type());
            break;
        default:
            return -1;
            break;
    }
    return 0;
}

int CreateThread()
{
    int x;
    CleanupNode *curnode;
    int retv = 0;

    pthread_attr_t attr;
    pthread_attr_init(&attr); //初始化属性值，均设为默认值
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); //设为绑定的
    for (x = 0; x < 4; x++) {
        curnode = (CleanupNode*)malloc(sizeof(CleanupNode));
        if (!curnode)
            return 1;
        curnode->threadnum = x;
        switch (x) {
            case 0:
                printf("created thread %d save\n", x);
                retv = pthread_create(&curnode->tid,
                                      NULL, thread_save, (void *) curnode);
                //&attr, thread_save, (void *) curnode); // 设为与轻进程绑定 gjq 2009-10-20
                break;
            case 1:
                printf("created thread %d timer\n", x);
                retv = pthread_create(&curnode->tid,
                                      NULL, thread_timer, (void *) curnode);
                //&attr, thread_timer, (void *) curnode); //将定时器线程设为与轻进程绑定 2009-2-1 seapex
                break;
            case 2:
                printf("created thread %d serial\n", x);
                retv = pthread_create(&curnode->tid,
                                      &attr, thread_serial, (void *) curnode); //将串口线程设为与轻进程绑定 2009-2-1 seapex
                //NULL, thread_serial, (void *) curnode);
                break;
            case 3:
                printf("created thread %d socket\n", x);
                retv = pthread_create(&curnode->tid,
                                      NULL, thread_socket, (void *) curnode);
                break;
            case 4:
                printf("created thread %d smpl_val\n", x);
                retv = pthread_create(&curnode->tid,
                                      NULL, thread_smpl_val, (void *) curnode);
                break;
            case 5:
                printf("created thread %d smpl_val\n", x);
                retv = pthread_create(&curnode->tid,
                                      NULL, thread_smpl_val, (void *) curnode);
                break;
            case 6:
                printf("created thread %d socket_dis\n", x);
                retv = pthread_create(&curnode->tid,
                                      NULL, thread_socket_dis, (void *) curnode);
                break;
            default:
                numthreads--;
                break;
        }
        if (retv) return 1;
        numthreads++;
    }
    return retv;
}

void OpenDevice()
{

}
void CloseDevice()
{

}

/*!
Initailize value
*/
void InitValue(void)
{
    int i;
    p_daramp_cnt = p_comm_cnt = p_socket_cnt = err_time_cnt = 0;
    comm_idle_cnt = 0;
    param_cfg = new ParamCfg;
    //重定向标准输出
    char stri[36];
    if ( main_arg_num <= 1 || strcmp(main_arg[0], "dbg") ) {
        sprintf(stri, "/tmp/debug.txt");
        stdout_save = freopen(stri, "w+", stdout);
    }
    printf("Initialize values...\n");
    numthreads = 0;

    TimeCstInit();
    
    comtrade_func = new ComtradeFunc;
    harmfunc = new CHarmFunc(&prmcfg->syspara()->line_para);
    other_func = new OtherFunc;
    save_func = new SaveFunc;
    volt_variation = new VoltVariation;
    pqmfunc = new Cpqm_func;
    pee_warning = new CEEWarning;
    wait_for_stdy = START_WAIT_TIME;
    fire_listener_prd_type(prmcfg->pqm_type());
    //chk_net_file();
    
}

void CleanupValue()
{
    printf("Cleanup values..\n");
    delete pee_warning;
    delete pqmfunc;
    delete volt_variation;
    delete save_func;
    delete other_func;
    delete harmfunc;
    delete prmcfg;
    delete comtrade_func;
    TimeCstEnd();
}

/*!
Initialize and active queue structs
*/
void InitQueue()
{
    printf("Initialize and active queue structs..\n");
    if (control_init(&clrq.control)) {
        dabort();
    }
    queue_init(&g_clrq.task);

    if (control_init(&g_mainq.control)) {
        control_destroy(&g_clrq.control);
        dabort();
    }
    queue_init(&g_mainq.task);

    if (control_init(&g_saveq.control)) {
        control_destroy(&g_clrq.control);
        control_destroy(&g_mainq.control);
        dabort();
    }
    queue_init(&g_saveq.task);

    control_activate(&g_mainq.control);
    control_activate(&g_saveq.control);
}

void CleanupQueue(void)
{
    control_destroy(&clrq.control);
    control_destroy(&mainq.control);
    control_destroy(&saveq.control);
    printf("Cleanup queue structs..\n");
}


void JoinThread()
{
    CleanupNode *curnode;

    //printf("joining threads...\n");

    while (numthreads) {
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
        numthreads--;
    }
}


void OnCreate()
{
    OpenDevice();

    InitValue();
    InitQueue();
    pthread_mutex_init(&store_mutex, NULL); //初始化数据存储互斥锁
    pthread_mutex_init(&debug_mutex, NULL); //初始化调试信息互斥锁

    if (CreateThread()) {
        printf("Error starting threads... cleaning up.\n");
        JoinThread();
        dabort();
    }

    printf("%s\n", NowTime());
}

void ColdReboot()
{
    watchdog().SetTimeout(7);
    pthread_mutex_lock(&mainq.control.mutex);
    mainq.control.active = QUITCMD;
    pthread_mutex_unlock(&mainq.control.mutex);
    printf("\ncold reboot...\n");
}

void OnTerminal()
{
    printf("%s\n", NowTime());

    pthread_mutex_unlock(&mainq.control.mutex);
    control_deactivate(&saveq.control);

    JoinThread();

    CleanupQueue();
    CleanupValue();

    pthread_mutex_destroy(&debug_mutex); //清除互斥锁mutex
    pthread_mutex_destroy(&store_mutex);
    CloseDevice();
}

void AdjustTime()
{
    time_t time1, time2;
    struct timeval tmvl1, tmvl2;

    time(&time1); //Get time of OS
    time2 = rtc_dev().Get(); //Get time of RTC
    if (prmcfg->clocksrc()) {   //system clock
        tmvl1.tv_sec = 0;
        tmvl1.tv_usec = prmcfg->timeerr() * 1000;
    } else {    //rtc clock
        tmvl1.tv_sec = time2 - time1;
        tmvl1.tv_usec = 0;
    }
    adjtime(&tmvl1, &tmvl2); //make a gradual adjustment to time of OS
    printf("tmvl2=%d.%d\n", tmvl2.tv_sec, tmvl2.tv_usec);

    if (prmcfg->clocksrc() && abs(time1 - time2) >= 3) { //time error>=3s
        rtc_dev().Set(time1); //Set time of RTC
    }
}

void HandlePtmr(int subtype)
{
    static int cntj = 0, cnti = 0;
    static unsigned int commst = 0, sockst = 0, darampst = 0, displayst = 0, st61850 = 0;

    if (stdout_save) fflush(stdout_save);
    switch (subtype) {
        case FEED_DOG: //cycle:5s
            if (wait_for_stdy > 0) {
                wait_for_stdy --;
            }
            if (commst != p_comm_cnt && darampst != p_daramp_cnt
                && sockst != p_socket_cnt) {
                commst = p_comm_cnt;
                sockst = p_socket_cnt;
                darampst = p_daramp_cnt;
                cntj = 0;
                watchdog().Feed();
            } else { //任何一个线程监视计数未变
                cntj++;
                printf("wtdog cntj=%d, serial=%d, socket=%d, daramp=%d, display=%d\n"
                       , cntj, p_comm_cnt, p_socket_cnt, p_daramp_cnt, p_display_cnt);
                push_debug_info("wtdog cntj");
                if (cntj >= watchdog_cnt) {
                    ColdReboot();
                    break;
                }
            }
            if (Monitor61850() < 0) break;

            if (++cnti >= 720) { //720*5S≈1小时
                cnti = 0;
                if (++comm_idle_cnt >= prmcfg->reset_hyst_time()) { //set-time reboot
                    ColdReboot();
                }
                AdjustTime();
            }
            break;
        default:
            break;
    }
}

/*!
    Called by:  main()
*/
void HandleSample(int subtype, void * p)
{
    switch (subtype) {
        case HARM_DATA:
            pqmfunc->harm_hdl();
            //notice_pthread(kTTDisplay, SAMPLEINFO, HARM_DATA, NULL);
            break;
        case FLUCT_DATA://每16*5个周波处理一次
            pqmfunc->fluct_hdl();
            break;
        case TRANSIENT_DATA:
            volt_variation->transient_hdl(p);
            break;
        case HSECOND_DATA: //每0.5秒处理一次
            pqmfunc->HalfSecHandle();
            break;
        case CLEAR_EVENT_INFO: //Clear summary infomation of event
            volt_variation->SetEventSummary(0);
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    int ret = 0;
    WorkNode *curtask;
    int majortype, minortype;
    void * point;

    if (argc > 1) {
        ret = TestModule(argc, argv);
        //printf("%s %s():ret=%d\n", __FILE__, __FUNCTION__, ret);
        if (ret <= 1) exit(ret);
    }
    if (for_debug_) {
        watchdog().Disable();
    } else {
        watchdog().Enable();
    }

    OnCreate();

    signal(SIGINT, ctrlCfun);  // Set the ^c catcher
    printf ("program is running, hit ^c to exit ... \n");

    for (;;) {
        pthread_mutex_lock(&mainq.control.mutex);
        while (mainq.task.head == NULL && mainq.control.active != QUITCMD) {
            pthread_cond_wait(&mainq.control.cond, &mainq.control.mutex);
        }
        if (mainq.control.active == QUITCMD)
            break;
        curtask = (WorkNode *) queue_get(&mainq.task);
        pthread_mutex_unlock(&mainq.control.mutex);

        majortype = curtask->major_type;
        minortype = curtask->minor_type;
        point = curtask->point;
        free(curtask);
        //printf("%s majortype=%d, minortype=%d\n", __FUNCTION__, majortype, minortype);
        switch (majortype) {
            case PTMRINFO:
                HandlePtmr(minortype);
                break;
            case SAMPLEINFO:
                HandleSample(minortype, point);
                break;
            default:
                break;
        }
        if (be_end == 0x5a || shmem_func().quit_cmd()) {
            pthread_mutex_lock(&mainq.control.mutex);
            mainq.control.active = QUITCMD;
            pthread_mutex_unlock(&mainq.control.mutex);
            printf("\nprogram to be terminal... by ^c\n");
            //pthread_cond_signal(&mainq.control.cond);
        }
        //printf("%s end\n", __FUNCTION__);

    }

    OnTerminal();

    return ret;
}


