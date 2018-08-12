#ifndef HARMBASE_H
#define HARMBASE_H
#include "daram_usr.h"
#include "../base_func/conversion.h"
#include "../base_func/time_cst.h"
#include "prmconfig.h"
#include <cstdlib>
using namespace std;

//struct SysPara;
//struct LinePar;

typedef struct UserCurCap{
	float user_cur3;
	float user_cur5;
	float user_cur7;
	float user_cur11;
	float user_cur13;
	float user_cur_oth;
}UserCurCap;

#define MAX_HARM_NUM 50

template <class T> struct HarmData  { T array[3][MAX_HARM_NUM+1]; };    //[0-2]PhaseA-C

class CHarmBase{
public:
	CHarmBase(LinePar* par);
	~CHarmBase(){};

	void write_hamp(unsigned short *hu, unsigned short *hi);
	void write_exthamp(unsigned short *hu, unsigned short *hi, unsigned short valid);
	
	void calc_hru_i(); //计算谐波含有率
	void calc_user_cur(); //计算用户第h次谐波电流允许值
	float hrmi_limit(int nums);//计算谐波电流允许值
	float get_harm_data(int type, int phs, int vc=0, int nums=1); // 获取谐波数据 
	void set_harm_limit();
	void SetRmsDsp(unsigned short * u_rms, unsigned short * i_rms);//设置dsp板侧计算的有效值
    void SetUnits(short *addr);
    
    char units_type(int idx) { return units_type_[idx]; };
    int units(int idx) { return units_[idx]; };
    bool old_unit() { return old_unit_; };
	float u_deviation(int phs){ return u_deviation_[phs]; };
   	float i_rms(int type, int phs) {    //type:1=primary,2=secondary
   	    if (type==1) {
   	        float fi = line_para_->CT1; fi /= line_para_->CT2;
   	        return i_rms_[phs]*fi;
   	    } else return i_rms_[phs]; };
   	float u_rms(int type, int phs) {    //type:1=primary,2=secondary
   	    if (type==1) {
   	        float fi = line_para_->PT1; fi /= line_para_->PT2;
   	        return u_rms_[phs]*fi;
   	    } else return u_rms_[phs]; };
   	float u_rms_ppv(int type, int phs) {//type:1=primary,2=secondary
   	    if (type==1) {
   	        float fi = line_para_->PT1; fi /= line_para_->PT2;
   	        return u_rms_ppv_[phs]*fi;
   	    } else return u_rms_ppv_[phs]; };
   	
protected:
	HarmData <unsigned long> harmrms_[2];  //harm rms. [0-1]:Voltage,Current. unit:mV/0.1mA
	HarmData <unsigned long> harm_in_[2];  //三相各次间谐波集有效值.[0-1]:Voltage,Current..unit:mV/0.1mA
	HarmData <unsigned long> harm_aggr_[3][2];  //harm rms aggregate data. [0-2]:maximum,average,minimum; [0-1]:Voltage,Current; unit:mV/0.1mA
	HarmData <unsigned long> harm_aggr_in_[3][2];  //间谐波集有效值 aggregate data.[0-2]:maximum,average,minimum. [0-1]:Voltage,Current; unit:mV/0.1mA
	HarmData <unsigned short> harmphs_[2];  //harm phase. [0-1]:Voltage,Current. unit:0.1degree
	HarmData <float> HRui_[2];          //HR(Harmonic Ratio). [0-1]:Voltage,Current. unit:%
	HarmData <float> HRU_in_;       //HRU(Harmonic Ratio Voltage) for interharmonic. unit:%
	HarmData <float> HRU_aggr_[3];  //HRU aggregate data. [0-2]:maximum, average, minimum. unit:%
	HarmData <float> HRU_aggr_in_[3];  //HRU aggregate data for interharmonic. [0-2]:maximum, average, minimum. unit:%
	
	float THD_[2][3][3];  //[0-1]:Voltage,Current; [0-2]:all,odd,even; [0-2]:phase A-C. unit:%
	float THD_aggr_[3][2][3][3];  //[0-2]:maximum, average, minimum; [0-1]:Voltage,Current; [0-2]:all,odd,even; [0-2]:phase A-C. unit:%

	float u_rms_[3], u_rms_ppv_[3], i_rms_[3]; //三相电压, 线电压, 电流有效值.
	float u_deviation_[3];  //Voltage deviation. [0-2]:Phase A,B,C;

	float harm_u_limit_[50]; //电压谐波限值.
	float harm_i_limit_[49]; //电流谐波限值.

	UserCurCap usr_c_cp;
	SysPara *syspara_;
	LinePar *line_para_;
	
	int units_[3];      //scale factor. [0]voltage, [1]current, [2]frequency

	void calc_thd(); //Calculate THD(Total Harmonic Distortion)
	float CalcUivalid(int phase, int vc); //Calculate voltage & current rms
	float CalcVoltdv(int phase);
	short line_coef_[3];    //unit:mV
private:
	char units_type_[3];    //0-2:voltage, current, frequency; 0=1/100, 1=1/1000, 2=1/10000, 3=1/100000
	bool old_unit_;     //true=(1/100V, 1/100A, no zip), false=(1/1000V, 1/10000A, zip)
};


//谐波电压允许值, THD, Odd HRU, Even HRU
const float HamULimit[][3] = { {5.0, 4.0, 2.0},
			{4.0, 3.2, 1.6},
			{4.0, 3.2, 1.6},
			{3.0, 2.4, 1.2},
			{3.0, 2.4, 1.2},
			{2.0, 1.6, 0.8},
			{2.0, 1.6, 0.8},
			{2.0, 1.6, 0.8},
			{2.0, 1.6, 0.8},
			{2.0, 1.6, 0.8} };

//谐波电流允许值, 2,3,4,5...,25
const float HarmILimit[][24] = {{ 78, 62, 39, 62, 26, 44, 19, 21,
				  16, 28, 13, 24, 11, 12, 9.7, 18,
				   8.6, 16, 7.8, 8.9, 7.1, 14, 6.5, 12},
			{ 43, 34, 21, 34, 14, 24, 11, 11,
			   8.5, 16, 7.1, 13, 6.1, 6.8, 5.3, 10,
			    4.7, 9, 4.3, 4.9, 3.9, 7.4, 3.6, 6.8},
			{ 26, 20, 13, 20, 8.5, 15, 6.4, 6.8, 
			   5.1, 9.3, 4.3, 7.9, 3.7, 4.1, 3.2, 6.0, 
			    2.8, 5.4, 2.6, 2.9, 2.3, 4.5, 2.1, 4.1},
			{ 15, 12, 7.7, 12, 5.1, 8.8, 3.8, 4.1, 
			   3.1, 5.6, 2.6, 4.7, 2.2, 2.5, 1.9, 3.6, 
			    1.7, 3.2, 1.5, 1.8, 1.4, 2.7, 1.3, 2.5},
			{ 16, 13, 8.1, 13, 5.4, 9.3, 4.1, 4.3, 
			   3.3, 5.9, 2.7, 5.0, 2.3, 2.6, 2.0, 3.8, 
			    1.8, 3.4, 1.6, 1.9, 1.5, 2.8, 1.4, 2.6},
			{ 12, 9.6, 6.0, 9.6, 4.0, 6.8, 3.0, 3.2, 
			   2.4, 4.3, 2.0, 3.7, 1.7, 1.9, 1.5, 2.8, 
			    1.3, 2.5, 1.2, 1.4, 1.1, 2.1, 1.0, 1.9},
			{ 12, 9.6, 6.0, 9.6, 4.0, 6.8, 3.0, 3.2, 
			   2.4, 4.3, 2.0, 3.7, 1.7, 1.9, 1.5, 2.8, 
			    1.3, 2.5, 1.2, 1.4, 1.1, 2.1, 1.0, 1.9},
			{ 12, 9.6, 6.0, 9.6, 4.0, 6.8, 3.0, 3.2, 
			   2.4, 4.3, 2.0, 3.7, 1.7, 1.9, 1.5, 2.8, 
			    1.3, 2.5, 1.2, 1.4, 1.1, 2.1, 1.0, 1.9},
			{ 12, 9.6, 6.0, 9.6, 4.0, 6.8, 3.0, 3.2, 
			   2.4, 4.3, 2.0, 3.7, 1.7, 1.9, 1.5, 2.8, 
			    1.3, 2.5, 1.2, 1.4, 1.1, 2.1, 1.0, 1.9}};

// alarm_word constant define 
#define  APH_THDu	0x0001  // A相电压总谐波畸变率. 
#define  BPH_THDu	0x0002  // B相电压总谐波畸变率. 
#define  CPH_THDu	0x0004  // C相电压总谐波畸变率. 
#define  APH_HRU	0x0008  // A相电压谐波含有率超限. 
#define  BPH_HRU	0x0010  // B相电压谐波含有率超限. 
#define  CPH_HRU	0x0020  // C相电压谐波含有率超限. 
#define  APH_HI		0x0040  // A相谐波电流超限. 
#define  BPH_HI		0x0080  // B相谐波电流超限. 
#define  CPH_HI		0x0100  // C相谐波电流超限. 
#define  APH_UP		0x0200  // A相电压偏差超限. 
#define  BPH_UP		0x0400  // B相电压偏差超限. 
#define  CPH_UP		0x0800  // C相电压偏差超限. 

//Harmonic type
const int HARM_AMP = 0;	//harmonic amplitude
const int HARM_HR = 1;	//harmonic 含有率
const int HARM_THD = 2;	//harmonic 总畸变率
const int CURR_RMS = 5; //current RMS value 有效值
const int CURR_RMS2 = 7; //2nd current RMS value 有效值
const int HARM_AMP2 = 8; //harmonic 2nd amplitude

#endif //HARMBASE_H
