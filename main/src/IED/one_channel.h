#ifndef _ONE_CHANNEL_H_
#define _ONE_CHANNEL_H_
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "param_chnl.h"
#include "pqm_data_types.h"
#include "meas_para.h"

static const int kHrmSmpNum = 2048; //sample data number for harmonic per 10 cycle(¡Ö0.2s)

struct MeasureValue {
    float freq;     //frequency 1s interval
    uint8_t freq_up; //freq update. 1=update. 0=no
    time_t freq_tm;  //freq update time.
    
    //below parameter is 10-cycle interval data
    time_t time; //10-cycle data update time
    float rmsn2[3]; //rms^2. square of rms. [0-2]:A-C
    float u_devn2[2][3];    //u_dev^2. square of voltage deviation. [0-1]:Urms-under, Urms-over. [0-2]:A-C. unit:V^2
    LoopBuffer<float> *rce_rmsn2[3];    //square of the rapid change event rms over 1 cycle, 
                                        //and refreshed each kRvcIntrvl sample point. A-C.
    CComplexNum harm[3][640];   //[0-2]:A-C. [0, 1/10-63_9/10]
    float seq[3];     //sequence component. [0-2]:zero,positive,negative
    
};
struct StatisValue {
    float freq;     //frequency 1s interval
    uint8_t freq_up; //freq update. 1=update. 0=no
    time_t freq_tm;  //freq update time.
    
    //below parameter is 10-cycle interval data
    time_t time; //10-cycle data update time
    float rmsn2[3]; //rms^2. square of rms. [0-2]:A-C
    float u_devn2[2][3];    //u_dev^2. square of voltage deviation. [0-1]:Urms-under, Urms-over. [0-2]:A-C. unit:V^2
    CComplexNum harm[3][640];   //[0-2]:A-C. [0, 1/10-63_9/10]
    float seq[3];     //sequence component. [0-2]:zero,positive,negative
};


struct ChnnlAttr {  //channel attribute
    uint8_t type;   //0=not used, 1=voltage, 2=current
    uint8_t mode;   //master or slave. 1=master, 0=slave
    uint8_t slaves; //number of slave.
    uint8_t slave_idx[kChannelTol-1];   //index of slave. 1=channel1,2=channel2...
	const uint16_t *stts_spc;  //statistic space. unit:s. point to PhyDev::parm_phd_.stts_spc
};

struct RceInfo {
    union {
        uint32_t stat;  // 0=not happen, other=happened
        struct {
            uint8_t intr;
            uint8_t dip;
            uint8_t swl;
        } hpn;
    };
    timeval stm;
    timeval etm;
    uint8_t ratestr;    //rate of change event start
    uint8_t varend;
};

struct RceParam {
    timeval time;   //time of 1st sampling point
    int cnt;        //count of sampling point in rce_sv_
};

class OneChannel
{
protected:

public:
    OneChannel();
    ~OneChannel();
    
    void MeasureFreq(int cnt10, timeval *tmv);
    void MeasureRms(int (*src)[3][kHrmSmpNum], int cnt, timeval *tmv);
    void PostFft(int *src, int cnt);
    void ClearRce();

    //Accessors
    int chl_match() { return chl_match_; };
    uint8_t chl_type() { return chnnl_attr_.type; };
    uint8_t chl_mode() { return channl_attr_.mode; };
    uint8_t detect_type() { return detect_type_; };
    const FreqValue *frq_val() { return &frq_par_.frq_val; };
    uint8_t rce_intr() { for (int i=0; i<3; i++) { if (rce_info_[i].hpn.intr) return 1; }; return 0;};
    const ParamChnl *parm_chnl() { return &parm_chnl_; };
    uint32_t rce_extre() { return rce_extre_; }; //return <0 = invalid
    RceInfo *rce_info(int phs) { return & rce_info_[phs]; };
    RceParam rce_par(int phs) { RceParam par; memset(&par, 0, sizeof(par)); 
                                rce_par_[phs]->Pop(&par); return par; };
    const int32_t *rce_sv(int phs) { return rce_sv_[phs]->Pop(); };
    uint8_t slaves() { return channl_attr_.slaves; };
    uint8_t slave_idx(int n) { return channl_attr_.slave_idx[n]; };
    uint8_t trig_phs() { return trig_phs_; };
    
    //Mutators
    void set_chnnl_attr(ChnnlAttr *attr) { memcpy(&chnnl_attr_, attr, sizeof(chnnl_attr_)); };
    void set_chl_match(uint8_t type) { chl_match_ = type; };
    void set_chl_type(uint8_t type) { chnnl_attr_.type = type; };
    void set_detect_type(uint8_t type) { detect_type_ = type; };
    void set_freq_up(uint8_t val) { meas_val_.freq_up = val; };
    void set_freqst_up(uint8_t val) { meas_val_.freqst_up = val; };
    void set_frqmspc(uint8_t val) {
        switch(val) {
            case 0: frq_par_.frqmspc = 1; break;
            case 1: frq_par_.frqmspc = 3; break;
            case 2: frq_par_.frqmspc = 10; break;
        } 
        frq_par_.tmi->set_interval(frq_par_.frqmspc)};
    void set_udevspc(uint8_t val) {};
    void set_parm_chnl(uint8_t *data) { memcpy(&parm_chnl_, data, sizeof(parm_chnl_)); };
    void set_smp_frq(uint32_t val) { smp_frq_ = val; };

private:
    void MeasureSeq();
    void RotateVec(CComplexNum *c, int ang);
    float SumVec3(const CComplexNum *c1, const CComplexNum *c2, const CComplexNum *c3);
    void DetectEvnt(float *rms, int rcnt, int phs, timeval stm);
    void GetRceLimit(float *high, float *low, float *intr, int type=0);

    ChnnlAttr chnnl_attr_;
    ParamChnl parm_chnl_;
    int prmchg_cnt_;    //parameter changed count
    uint8_t detect_type_;   //the channel type be detected
    int chl_match_; //channel type match error. i.e. the type be detected is not identical with the type be set.
                    //0=matched. 1=not match 
    FreqParam frq_par_;
    MeasParam3s meas_par3s_;
    int smp_frq_;       //sampling frequency. unit:Hz
    StatisValue statis_val_;
    StatisValue statis_val_;
    RmsStatis rms_statis_;
    
    //rapid change event
    LoopBuffer<float> *rce_rmsn2_[3];   //square of the rapid change event rms over 1 cycle, 
                                        //and refreshed each kRvcIntrvl sample point. A-C.
    LoopPointer<int32_t> *rce_sv_[3];   //sample value for rce record wave.
    LoopBuffer<RceParam> *rce_par_[3];
    RceInfo rce_info_[3]; //[0-2]:A-C
    float rce_max_, rce_min_;   //(maximum value)^2 of rce rms, (minimum value)^2 of rce rms. only for voltage,unit:10mV
    uint32_t rce_extre_;    //extremum value of rce rms. primary voltage, unit:10mV
    uint8_t trig_phs_;  //rce trigger phase. 0-2=A-C
};


#endif //_ONE_CHANNEL_H_
