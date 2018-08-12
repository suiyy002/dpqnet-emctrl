#ifndef _RCE_FUNC_H_
#define _RCE_FUNC_H_

#include "comtrade_func.h"

#define TransDataIntSz (sizeof(TRANS_DATABLK_BUF)*(INT_BUF_NUM/2+8)+sizeof(TRANS_TTL_BUF))

enum TriggerCause {kTrigNone, kTrigVLmt, kTrigVVar, kTrigIStr, kTrigILmt, kTrigManual, kTrigSteady};
enum EventTypes {kEvtTypeNone, kEvtTypeIntr, kEvtTypeDip, kEvtTypeSwell};
struct RceState {
    TriggerCause trig_c;
    EventTypes event_t;
    timeval rtime;  //record start time. time of 1st sv be recorded.
    timeval ttime;  //trigger time. event start time
    timeval etime;  //event end time
    uint32_t dur;   //duration. unit:ms
    uint32_t extre; //extremum. unit:10mV
    //int start;
};

struct EvntSmmryInfo { char str[48]; };
enum EventMark {kRealMark=1, kPstMark, kPltMark, kFluctMark, kFreqStatMark, kFreqRealMark, kMmxuStatMark, kMhaiStatMark, kMsqiStatMark};

class RceFunc
{
public:
	RceFunc(int idx);
	~RceFunc();

    void SetRceData(int vc, int32_t **sv, int cnt, timeval *time);
    void EndRce(uint32_t extr);
    void SetRatio(int vc, const uint32_t *ps);
    void SetDuration(timeval *tmv);
    void Soe2Shm();
    void SetEventSummary(int type);
    int GetEvntInfo(int max, EvntSmmryInfo *info);

    //Accessors
    //uint32_t duration() { return rce_stat_.dur; };
    int overtime() { if (rce_stat_.dur>(max_dur_+200)) return 1; return 0;};
    uint8_t stdy_event() { return stdy_event_; };
    TriggerCause trigger_cause() { return rce_stat_.trig_c; };
    
    //Mutators
    void set_chnl_idx(uint8_t vidx, uint8_t cidx) { chnl_idx_[0] = vidx; chnl_idx_[1] = cidx; };
    void set_dre_attr(uint8_t tol, uint8_t fltn) {comtrd_fun_->SetDreAttr(tol, fltn); flt_num_ = comtrd_fun_->flt_num();};
    void set_event_type(EventTypes val) { rce_stat_.event_t = val; };
    void set_stdy_event(int type) { if(type) stdy_event_ |= (1<<type); else stdy_event_ = 0; };
    void set_trigger_cause(TriggerCause val) { rce_stat_.trig_c = val; };
    void set_trig_phs(uint8_t phs) { comtrd_fun_->set_trig_phs(phs); };
    void set_ttime(timeval *val) { memcpy(rce_state_.ttime, val, sizeof(timeval)); };
protected:

private:
    void PushQvvr(int type);
    void Qvvr2Shm();

    uint8_t ld_idx_; //LD index
    uint8_t chnl_idx_[2];   //channel index. [0-1]:voltage/current. range:1~kChannelTol, 0=no used
    RceState rce_stat_;
    uint8_t stdy_event_;  //Event be triggered by steady alarm data
                //bit1 freq; bit2 harm volt; bit3 harm current; bit4 volt unbalance; 
                //bit5 current negtive; bit6 volt deviation; 
    
    ComtradeFunc *comtrd_fun_;
    float ratio_[2]; //[0-1]:PT,CT
    
    int16_t savebuf_[2][5120][3];   //[0-1]:voltage,current; [0-2]:A-C
    int16_t *psvbuf_[2]; //[0-1]:savebuf_u_/savebuf_i_
    int buf_cnt_;   //savebuf count
    int flt_num_;   //Fault number
    uint32_t max_dur_;   //maximum record duration. unit:ms
    
    LoopBuffer<PqmQvvr> *qvvr_soe_;    //qvvr sequence of event buffer
    pthread_mutex_t mutex_; //mutex lock
    LoopBuffer<EvntSmmryInfo> *evnt_smmry_info_;

};

static const char CauseOfEvent[7][8] = {"---", "VLmt", "VVar", "IStr", "ILmt", "Man", "Stdy"};
static const char TypeOfEvent[4][8] = {"---", "Intr", "Dip", "Swell"};

#endif	//_RCE_FUNC_H_
