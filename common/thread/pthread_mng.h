#ifndef _PTHREAD_MNG_H_
#define _PTHREAD_MNG_H_

#include "control.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------
typedef struct {
    data_control control;
    queue task;
} PthreadQueue;

typedef struct {
    struct node *next;
    int major_type;
    int minor_type;
    void * point;
} WorkNode;

typedef struct {
    struct node *next;
    int threadnum;
    pthread_t tid;
} CleanupNode;

//--------------------------------------------------------------------------------
/* define major type and minor type */
//information from keyboard
static const int kKeyInfo = 10;

//Information from sample board
static const int SAMPLEINFO = 20;

  static const int REAL_WAVE = 1;
  static const int HARM_DATA = 2;
  static const int FLUCT_DATA = 3;
  static const int TRANSIENT_DATA = 4;
  static const int HSECOND_DATA = 5;   //Half-second
  static const int CLEAR_EVENT_INFO = 6;

//Information from communication interface
static const int COMMINFO = 30;

  static const int FETCH_HIS = 1;  // PC fetch history record.
  static const int FETCH_ALM = 2;  // PC fetch alarm history record.
  static const int FETCH_REAL = 3; // PC fetch real data.
  static const int SET_TIME = 4;   // PC set system date&time.
  static const int FETCH_SET = 5;  // PC read system param config.
  static const int WRITE_SET = 6;  // PC set system param config.
  static const int FLSH_INI = 7;   // PC initial flash data.
  static const int FETCH_Pst = 8;  // PC fetch Pst history record.
  static const int FETCH_FLUCT = 10;  // PC fetch fluctuation history record.
  static const int LCM_CRL = 11;      // PC control LCM power manager on or off.
  static const int INIT_UNIT = 12;      // PC initialize unit.
  
//Information from pthread timer
static const int kPTimerInfo = 40;
  //static const int kHalfSecond = 1; //0.5 second
  static const int kOneSecond = 1;
  static const int kThreeSecond = 2;
  static const int kTenSecond = 4;
  static const int kOneMinute = 8;

//Debug Information
static const int DEBUGINFO = 50;

//Save Information
static const int SAVEINFO = 90;
enum RcdSaveTypes {
    kHarmRcd=1, kFreqRcd, kUnblcRcd, kVoltdvRcd, kPstRcd, 
    kPower01Time, kSysParam, kEEWRcd, //EEW data save.
    kTranstSave, kResetFile, kEEWTRcd,   //EEW Transformer data save.
    kFlushFileAll, kResetTranstData, kEEWParam,   //EEW param save.
    kHarmCyc10, kAlterTrigger, kSavePsswd, kSaveAudit, kSaveSM4Key
};

//GUI communication command
static const int kGUICommuCmd = 100;
// minor type refer to kGuiCommand

//--------------------------------------------------------------------------------
void *thread_display(void *myarg);
void *thread_keybd(void *myarg);
void *thread_timer(void *myarg);
void *thread_serial(void *myarg);
void *thread_socket(void *myarg);
void *thread_save(void *myarg);
void *thread_darampoll(void *myarg);
void *thread_daramint(void *myarg);

//--------------------------------------------------------------------------------
extern PthreadQueue clrq;
extern PthreadQueue cwq;
extern PthreadQueue disq;
extern PthreadQueue saveq;
extern unsigned int p_comm_cnt, p_socket_cnt, p_daramp_cnt, p_darami_cnt, p_display_cnt, err_time_cnt;
extern unsigned int debug_cnt, debug_cnt1, debug_cnt2;
extern int exit_flag;
extern unsigned char isdbgdata;
extern pthread_mutex_t store_mutex;

#ifdef __cplusplus
}
#endif

#endif  //_PTHREAD_MNG_H_

