/*! \file meas_para.h
    \brief Measure parameter.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
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

    FreqParam() {
        memset(this, 0, sizeof(FreqParam));
        space = -1;
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
    float seq[3];     //sequence component. [0-2]:zero,positive,negative
    CComplexNum harm[3][51];    //[0-2]:A-C. [0-50]
    CComplexNum ihrm[3][51];    //interharmonic. [0-2]:A-C. [0-50]
    float hr[3][49];    //harmonic ratio. [0-2]:A-C. [2-50]. unit:%
    float ihr[3][51];   //interharmonic ratio. [0-2]:A-C. [0-50]. unit:%
    float thd[3];       //THD. [0-2]:A-C. unit:%
    float thdodd[3];    //odd THD. [0-2]:A-C. unit:%
    float thdevn[3];    //even THD. [0-2]:A-C. unit:%
};
struct MeasParam3s {
    MeasValue3s val;
    CComplexNum harm3s[3][640];   //[0-2]:A-C. [0, 1/10-63_9/10]
    
    //temporary variable
    int cnt;            //count of 10cycle
    float sum_rmsn2[3]; //sum of 10cycles rmsn2.
    float sum_u_devn2[2][3];    //sum of 10cycles u_devn2.
    CComplexNum sum_harm[3][640];   //sum of 10cycles harm.
    float sum_seq[3];     //sum of 10cycles seq.
};

struct StatisRmsVal { //rms & voltage deviation statistics
    uint8_t update; //1=update, 0=no
    time_t time;
    float rmsn2[3][4]; //rms^2. square of rms. [0-2]:A-C
    float u_devn2[3][2][4];    //u_dev^2. square of voltage deviation. [0-2]:A-C. [0-1]:Urms-under, Urms-over. unit:V^2
};
struct StatisRmsPar {    //rms & voltage deviation statistical parameter
    StatisRmsVal val;
    
    //temporary variable
    TimeInterval *tmi; //time interval for frq_val.freqst. statistic space
    int space; //statistic space. unit:s
    DataStatis *rms_st[3];
    DataStatis *udev_st[3][2];
    
    StatisRmsPar() {
        memset(this, 0, sizeof(StatisRmsPar));
        space = -1;
        tmi = new TimeInterval();
    };
    ~StatisRmsPar() {
        for (int i=0; i<3; i++) {
            delete rms_st[i];
            for (int j=0; j<2; j++) {
                delete udev_st[i][j];
            }
        }
        delete tmi;
    };
};

struct StatisHrmVal { //harmonic statistics
    uint8_t update; //1=update, 0=no
    time_t time;
    CComplexNum harm[3][51][4]; //rms^2. square of rms. [0-2]:A-C; [0-50]:0-50 order; [0-3]:avg,max,min,cp95
};
struct StatisHrmPar {    //harmonic statistical parameter
    StatisHrmVal val;
    
    //temporary variable
    TimeInterval *tmi; //time interval for frq_val.freqst. statistic space
    int space; //statistic space. unit:s
    float real[3][51];  //rms^2. square of real. [0-2]:A-C; [0-50]:0-50 order
    float image[3][51]; //rms^2. square of image. [0-2]:A-C; [0-50]:0-50 order
    
    StatisHrmPar() {
        memset(this, 0, sizeof(StatisHrmPar));
        space = -1;
        tmi = new TimeInterval();
    };
    ~StatisHrmPar() {
        for (int i=0; i<3; i++) {
            for (int j=0; j<51; j++) {
                delete real[i][j];
                delete image[i][j];
            }
        }
        delete tmi;
    };
};
#endif //_MEAS_PARA_H_
