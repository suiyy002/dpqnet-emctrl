/*! \file meas_para.h
    \brief measure parameter.
*/
#ifndef _MEAS_PARA_H_
#define _MEAS_PARA_H_
#include <cstdio>
#include <cstdlib>
#include <cstring>

struct FreqValue {
    uint16_t freq; //frequency atomic value. 1s,3s,10s
    uint8_t freq_up; //freq update. 1=update, 0=no
    time_t time;    //time for freq
    
    uint16_t freqst[4]; //frequency statistic value.[0-3]:average,maximum,minimum,cp95
    uint8_t freqst_up;  //freqst update. 1=update, 0=no
    time_t timest;      //time for freq_st;
};
struct FreqParam {
    FreqValue frq_val;
    
    //temporary variable
    int cnt;        //count of 10cycle
    int freq_cnt;   //sample point count total
    TimeInterval *tmi; //time interval for frq_val.freq. measure space
    DataStatis *datast;
    TimeInterval *tmist; //time interval for frq_val.freqst. statistic space
    int frqmspc;   //frequecy measure space. unit:s
    int sz95; //size of buffer for cp95

    FreqParam() {
        tmi = new TimeInterval(); 
        tmist = new TimeInterval();
    };
    ~FreqParam() {
        if (datast) delete datast;
        delete tmist;
        delete tmi;
    };
};

struct MeasValue3s { //3s(150cycles) interval measure value
    uint8_t update; //1=update, 0=no
    time_t time;
    float rmsn2[3]; //rms^2. square of rms. [0-2]:A-C
    float u_devn2[3][2];    //u_dev^2. square of voltage deviation. [0-2]:A-C. [0-1]:Urms-under, Urms-over. unit:V^2
    CComplexNum harm[3][640];   //[0-2]:A-C. [0, 1/10-63_9/10]
    float seq[3];     //sequence component. [0-2]:zero,positive,negative
};
struct MeasParam3s {
    MeasureValue3s val;
    
    //temporary variable
    int cnt;            //count of 10cycle
    float sum_rmsn2[3]; //sum of 10cycles rmsn2.
    float sum_u_devn2[2][3];    //sum of 10cycles u_devn2.
    CComplexNum sum_harm[3][640];   //sum of 10cycles harm.
    float sum_seq[3];     //sum of 10cycles seq.
};

struct StatisValue { //Statistic measure value
    uint8_t update; //1=update, 0=no
    time_t time;
    float rmsn2[3]; //rms^2. square of rms. [0-2]:A-C
    float u_devn2[3][2];    //u_dev^2. square of voltage deviation. [0-2]:A-C. [0-1]:Urms-under, Urms-over. unit:V^2
    CComplexNum harm[3][640];   //[0-2]:A-C. [0, 1/10-63_9/10]
    float seq[3];     //sequence component. [0-2]:zero,positive,negative
};

struct RmsStatis {
    //temporary variable
    int cnt;            //count of 10cycle
    float sum_rmsn2[3]; //sum of 10cycles rmsn2.
    DataStatis *datast[3];
    TimeInterval *tmi; //time interval for frq_val.freqst. statistic space
    int sz95; //size of buffer for cp95

    RmsStatis() {
        tmi = new TimeInterval(); 
    };
    ~RmsStatis() {
        if (datast) delete datast;
        delete tmi;
    };
};


#endif //_MEAS_PARA_H_
