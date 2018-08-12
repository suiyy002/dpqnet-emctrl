#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <cmath>
#include "thread/pthread_mng.h"
#include "pqm_func/pqmfunc.h"
#include "pqm_func/prmconfig.h"
#include "pqm_func/volt_variation.h"
//#include "pqm_func/save_func.h"
#include "device/device.h"
#include "EEW/ee_warning.h"

extern int fd_darampoll, fd_daramint, fd_sioport;

static const int daram_int_delay = 4 * 5;   //5s
static int poweron_delay = 1;	//daram 中断线程上电延时处理

extern int TRANST_VALID;

//-----------------------------------------------------------------------------
void *thread_preproc(void *myarg)
{
    WorkNode *tomain;
    WorkNode *todis;
    CleanupNode *pthnode;
    unsigned int flag;
    unsigned short buf[100];
    int num;
    long refresh_time = 2400; //双口RAM数据刷新周期，防止双口RAM数据偶尔变为无效，
    //上电后周期为10分钟，之后为5小时。
    char stri[48];
    unsigned short eew_notice; //EEW data send notice

    pthnode = (CleanupNode *) myarg;

    printf("smpl_val thread run...\n");
    for (;;) {
        p_preprc_cnt++; //增加本线程监视计数
        pqnet_ied().HandleSV(100);

        if (mainq.control.active == QUITCMD)
            break;
    }

    notice_clrq(pthnode);
    return NULL;
}

//-----------------------------------------------------------------------------
//thread_daramint的清理函数
void clean_daramint(void *myarg)
{
    CleanupNode *pthnode;
    pthnode = (CleanupNode *) myarg;

    pthread_mutex_unlock(&mainq.control.mutex);

    pthread_mutex_lock(&clrq.control.mutex);
    queue_put(&clrq.task, (node *) pthnode);
    pthread_mutex_unlock(&clrq.control.mutex);
    pthread_cond_signal(&clrq.control.cond);
    printf("thread %d--daramint shutting down...\n", pthnode->threadnum);
}

//-----------------------------------------------------------------------------
void *thread_daramint(void *myarg)
{
    int old_order; 
    int cnt;
    struct timeval curtm;
    int i_bufpos = 0;
    TRANS_TTL_BUF * ptrans_ttl;
    TRANS_DATABLK_BUF * ptrans_dat;
    int i, j, k, iarea_count;
    int is_end = 1, isfrst = 1;

    CleanupNode * pthnode = (CleanupNode *) myarg;
    pthread_cleanup_push(clean_daramint, pthnode);  //登记清理函数clean_daramint(pthnode)

    printf("daramint thread run...\n\n");
    for (;;) {
        char *bufp = (char *)volt_variation->vvr_intr_buf() + i_bufpos * TransDataIntSz;
        cnt = read(fd_daramint, bufp, 5);
        if (cnt < 0) {
            printf("Read daramint error!\n");
            continue;
        }
        if (poweron_delay) {	//Power on delay
            //printf("daram int poweron_delay=%d,received %d \n", poweron_delay, cnt); //for debug
            p_darami_cnt = 0;
            continue;
        }
        p_darami_cnt++;
        if (!cnt) continue;
        if (!TRANST_VALID) continue; //不具有暂态功能(e.g.PQNet2xx),则不处理了。

        ptrans_ttl = (TRANS_TTL_BUF*)bufp;
        iarea_count = ptrans_ttl->datablk_count;
        if (iarea_count <= 0 || iarea_count > 64) {
            printf("iarea_count is error--%d!\n", iarea_count);
            continue;
        }
        ptrans_dat = (TRANS_DATABLK_BUF*)(bufp + sizeof(TRANS_TTL_BUF));
        for (i = 0; i < iarea_count; i++) {
            //printf("blk_status=%x, is_end=%d\n", ptrans_dat->blk_status, is_end);
            if (ptrans_dat->blk_status == kTSStart&&is_end) {
                is_end = 0;
                //printf("\n111111a. Event starting, order=%d, %s()\n", ptrans_dat->order, __FUNCTION__);
                old_order = ptrans_dat->order - 1;
            }
            //printf("ptrans_dat->blk_status=%d\n", ptrans_dat->blk_status);
            if (ptrans_dat->blk_status != kTSStart && ptrans_dat->blk_status != kTSGoing && !is_end) {
                is_end = 1;
                getlocaltime(&curtm);
                volt_variation->set_transt_etime(&curtm);
               // printf("111111b. Event end, old_order=%d,status=%x, ttl_count=%d\n", ptrans_dat->order, ptrans_ttl->status, ptrans_ttl->ttl_count);
            }
            //printf("old_order=%d, order=%d, order-old_order=%d\n", old_order, ptrans_dat->order, ptrans_dat->order - old_order);
            if ((ptrans_dat->order - old_order != 1) && (ptrans_dat->order - old_order != 0)) {
                if (!(old_order == 0xFFFF && ptrans_dat->order == 0)) {
                    printf("p_daram err! old_order=%d order=%d status:%d\n", old_order, ptrans_dat->order, ptrans_dat->blk_status);
                }
            }
            old_order = ptrans_dat->order;
            ptrans_dat++;
        }
        //printf("1c. notice_pthread to kTTMain. iarea_count=%d\n", iarea_count);
        notice_pthread(kTTMain, SAMPLEINFO, TRANSIENT_DATA, bufp);
        i_bufpos++;
        i_bufpos %= SV_THRINT_M_LEN;

    }
    pthread_cleanup_pop(1); //执行清理函数并删除它
    printf("thread %d--daramint shutting down...\n", pthnode->threadnum);
    return NULL;
}


