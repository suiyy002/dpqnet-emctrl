#ifndef _TRANST_ANALYSIS_
#define _TRANST_ANALYSIS_

#include "../base_func/loop_buffer.h"
#include "../IPC/share_mem.h"

struct TRST_F_HEAD;

class TranstAnalysis
{
public:
    TranstAnalysis();
    ~TranstAnalysis();

    void VarStr(TRST_F_HEAD  * ptrst_hd);
    void VarEnd(TRST_F_HEAD  * ptrst_hd);
    void AddSmplData(int vc, short *pdata, unsigned int smp_len);
    void WriteQvvrShm();
    float extrema() {return evnt_char_val_.extrema;};
    float dur() {return evnt_char_val_.dur; };
    long offset_time() { return offset_time_; };
    int vvar_type() { return evnt_char_val_.type; };
    
protected:
private:
struct EventCharVal {   //Event characteristic value
    float extrema;
    float dur;
    int type;   //0-3:"---", "Intr", "Dip", "Swell"
};
    int SetVVarType();
    bool reset_rms_; //!> Reset voltage rms calculate.
    int smpnum_cyc_;        //!> sample data number in 1 cycle
    int smpl_freq_; // sample frequency
    double rms_qdrtc_sum_[3]; //!> Quadratic sum of RMS. [3]=A/B/C
    double max_val_[3];
    double min_val_[3];
    int duration_[3];
    int limit_cnt_[3];
    LoopBuffer<float> * psquare_;   //quare of sample value. A,B,C,A,B,C...
    LoopBuffer<PqmQvvr> * qvvr_soe_;    //qvvr sequence of event buffer
    PqmQvvr qvvr_val_;
    
    float thr_dip_[2];  // voltage dip threshold, [2]=threshold/hysteresis
    float thr_swl_[2];  // voltage swell threshold, [2]=threshold/hysteresis
    float thr_intr_;    // voltage interrupt threshold
    int debug_[3];
    
	bool event_end_;
	EventCharVal evnt_char_val_;
    pthread_mutex_t mutex_; //互斥锁变量
    
    int offset_num_;   //事件触发点到第1个点的偏移点数。每次初始化为-1.
    int offset_time_; //事件触发时间到第1个点的偏移时间，unit:us
    
struct FilterCPxPara {   //extrema value filter by p9x
    int interval;    //sample interval. default is 60
    float min[24];
    int tol;    //sample value total number
    bool start;
};
    FilterCPxPara fltr_par_[3];  //A-C
    void FilterCPx(int type, int phs=0);
};


#endif

