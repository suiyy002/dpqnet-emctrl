/*! \file dis_data_types.h
    \brief display data type definition.
    Copyright (c) 2017  Xi'an Boyuu Electric, Inc.
*/
#ifndef _PQM_DATA_TYPES_H_
#define _PQM_DATA_TYPES_H_

#include <stdint.h>

static const int kMaxHarmNum = 51;

struct FrequencyT { //unit:0.01Hz
    uint16_t freq; //frequency refresh per 1s
    uint16_t freq10; //frequency refresh per 10s
    time_t sys_tm;   //system time
};

struct WaveformT {
	int16_t smpu[3][128]; //三相各次电压采样值.
	int16_t smpi[3][128]; //三相各次电流采样值.
};

struct HarmonicT {
	uint32_t huam[3][kMaxHarmNum];   //三相0~50次谐波电压的幅值.secondary, 1/1000V
	uint16_t huph[3][kMaxHarmNum];   //三相0~50次波电压的相位. 1/10degree
	uint32_t hiam[3][kMaxHarmNum];   //三相0~50次谐波电流的幅值.secondary, 1/10000A
	uint16_t hiph[3][kMaxHarmNum];   //三相0~50次谐波电流的相位. 1/10degree
	float thd[2][3][3]; //[0-1]:voltage,current; [0-2]:all,odd,even; [0-2]:A-C
	float hr[2][3][kMaxHarmNum-2]; //[0-1]:voltage,current; [0-2]:A-C; [0-48]:2-50order
};

struct PowerT {
	float total[3][3];   //[0-3]:active,reactive,apparent; [0-2]:phase A-C. unit:W/var/VA
	float harm[3][3][kMaxHarmNum-1];    //[0-3]:active,reactive,apparent; [0-2]:phase A-C;
	                                        //[0-49]:1-50order. unit:W/var/VA
};

struct OthersT {
	float rms[2][3];  //primary rms. [0-1]:voltage(V),current(A); [0-2]:A-C.
	float u_rms_ppv[3]; //primary voltage rms phase to phase. [0-2]:A-C
	float u_deviation[3]; //voltage deviation. [0-2]:A-C. unit:%
	float pst[3];
	float plt[3];
	float unbalance[2][3];  //[0-1]:voltage,current. [0-2]:zero,positive,negative
};

struct EquipState { //equipment state
    uint16_t battery_pow; //battery power energy
    uint16_t pow01_num; //power on/off number
    time_t pow01_time[10];  //power on/off time last 10
    uint8_t switch_in[2];    //开关量输入状态
};

struct EventState { //event state
    uint16_t manual_state;  //state of manual trigger. 0=not trigger, 1=triggered by manual, 37=triggered by steady
    uint16_t evn_num;   //event number
    char smry_inf_[10][48];  //event summary information
};

struct CtrlState { //control state
    uint16_t save_cyc10;    //state of save 10cycle. 0=start, 1=stop
    uint16_t clbrt_stat;    //calibrating precision state. bit0-2 is state. 0=stop, 1=start. bit3-15 is progress count
    uint16_t getdc_stat;    //get dc component state. bit0-2 is state. 0=stop, 1=start. bit3-15 is progress count
};

struct HarmLimit { //control state
    float thdu;     //THDu.
    float hru[49];  //HRu. 2-50 order
    float irms[49]; //harmonic current rms. unit:A
};

#endif // _PQM_DATA_TYPES_H_ 
