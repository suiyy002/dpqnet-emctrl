#include <cstdlib>
#include <cstdio>
#include <unistd.h>

#include "thread/pthread_mng.h"
#include "pqm_func/pqmfunc.h"
#include "pqm_func/harmfunc.h"
#include "pqm_func/save_func.h"
#include "pqm_func/volt_variation.h"
#include "ComTraDE/comtrade_func.h"
//#include "pqm_func/other_func.h"
#include "device/device.h"
#include "EEW/ee_warning.h"

void *thread_save(void *myarg)
{

    CleanupNode *pthnode;
    WorkNode *cursave;
    int savetype;
    int dog_stat;
    void * point;

    printf("save thread run...\n");
    pthnode = (CleanupNode *) myarg;

    for (;;) {
        pthread_mutex_lock(&saveq.control.mutex);
        while (saveq.task.head == NULL && saveq.control.active != QUITCMD) {
            pthread_cond_wait(&saveq.control.cond, &saveq.control.mutex);
        }
        if (saveq.control.active == QUITCMD) {
            pthread_mutex_unlock(&saveq.control.mutex);
            break;
        }
        cursave = (WorkNode *) queue_get(&saveq.task);
        pthread_mutex_unlock(&saveq.control.mutex);

        savetype = cursave->minor_type;
        point = cursave->point;
        free(cursave);
        if (pqmfunc->get_power_stat() && savetype != kFlushFileAll
            && savetype != kSysParam) {
            //printf("battery low, not save!\n");
            continue;
        }
        pthread_mutex_lock(&store_mutex);
        switch (savetype) {
            case kHarmRcd:
                save_func->SaveHarmonic();
                break;
            case kSysParam:
                prmcfg->save_syspara(1);
                harmfunc->calc_user_cur(); //计算用户第h次谐波电流允许值
                harmfunc->set_harm_limit(); //设置谐波限值
                harmfunc->update_IntBase(); //更新间谐波超限比较基准值,一定要放在前面2个函数之后
                break;
            case kTranstSave:
                volt_variation->save_transt_data(point);
                break;
            case kResetFile:
                printf("clear save record!\n");
                save_func->ClearDB();
                save_func->ClearRecData();  //Clear other steady data
                prmcfg->clear_trst_rcd(); //清空结构save_rcd中暂态相关数据
                prmcfg->save_syspara(2);
                volt_variation->reset_transt_data();
                comtrade_func->ClearFile();
                notice_pthread(kTTMain, SAMPLEINFO, CLEAR_EVENT_INFO, NULL);
                break;
            case kResetTranstData:
                prmcfg->clear_trst_rcd(); //清空结构save_rcd中暂态相关数据
                prmcfg->save_syspara(2);
                volt_variation->reset_transt_data();
                comtrade_func->ClearFile();
                notice_pthread(kTTMain, SAMPLEINFO, CLEAR_EVENT_INFO, NULL);
                break;
            case kFlushFileAll:    //把缓存中的数据以物理方式写入到文件
                prmcfg->save_syspara(2);
                printf("Flush the file!\n");
                break;
            case kPower01Time:
                save_func->SavePower01Time();
                break;
            case kFreqRcd:
                save_func->SaveRecData(kFreqSave);
                break;
            case kUnblcRcd:
                save_func->SaveRecData(kUnblcSave);
                break;
            case kVoltdvRcd:
                save_func->SaveRecData(kVoltdvSave);
                break;
            case kPstRcd:
                save_func->SaveRecData(kPstSave);
                break;
            case kEEWRcd:
            case kEEWTRcd:
                pee_warning->SaveRcdFile(savetype);
                break;
            case kEEWParam:
                pee_warning->SaveParamFile();
                break;
            case kHarmCyc10:
                save_func->Savecyc10(point);
                break;
            case kAlterTrigger:
                save_func->ClearDB(prmcfg->harmrec_svmax());
                break;
            default:
                break;
        }
        pthread_mutex_unlock(&store_mutex);
    }
    prmcfg->save_syspara();

    notice_clrq(pthnode);
    return NULL;
}


