#ifndef OTHER_FUNC_H
#define OTHER_FUNC_H
#include "daram_usr.h"
#include "../base_func/conversion.h"
#include "../base_func/time_cst.h"
#include "../base_func/loop_buffer.h"
#include "prmconfig.h"

enum SaveFileType {
    kPstSave, kFreqSave, kUnblcSave, kVoltdvSave
};
enum AggregateType { kAggrMax, kAggrAvg, kAggrMin };

/*
Unbalance data structure
  [0-1]voltage,current, [0-4]zero, positive, negative,
  unbalance(secondary)&zerounbalance. unit:0.01V mA 1/10000
*/
template <class T> struct UnblncData { T array[2][5]; };

/*
Voltage deviation data structure
  [0-3]:rms,rms-over,rms-under. unit:V.  [0-2]:A,B,C. 
*/
template <class T> struct VoltDvData { T array[3][3]; };

struct  FreqSaveModel {
    time_t time;
    unsigned short data;    //unit: 1/100Hz
    unsigned short q;       //quality
};

struct  UnblncSaveModel {
    time_t time;
    unsigned short data[2][3];   //[0-1]Voltage, current; [0-2] zero, positive, negative. unit:0.01V 1/1000A
    unsigned short q;       //quality. bit15-3(0-12). bit0--adj_time_.
    unsigned short reserved;
};

struct  VoltDvSaveModel {
    time_t time;
	unsigned short data[3][3];  //[0-2]RMS(format & unit:same as harmonic); pos&neg deviation(unit:1/10000); [0-2] A,B,C
    unsigned short q;       //quality
};

struct  PstSaveModel {
    time_t time;
	unsigned short data[3];  //[0-2]A-C. unit:1/1000
    unsigned short q;       //quality
};

class OtherFunc
{
public:
	OtherFunc();
	~OtherFunc();
	
    void InitData(SaveFileType type);
    void SetPst(float * fluckt);
    void SetRms(unsigned short * u_rms, unsigned short * i_rms);
	void SetUnbalance(unsigned short * pu_seq_compnt, unsigned short * pi_seq_compnt);
    void SetFrequency(unsigned short *freq);
    void SetFluct();
    int get_save_data(SaveFileType type, void *pdata);
    int get_save_size(SaveFileType type, int sr);
    void Aggregate(SaveFileType type, time_t time, bool adj_tm);
    void AlarmCheck(unsigned int &words, time_t time);
	float unbalance(int vc, int type);
    void GetShmReal(PqmMmxu *mmxu, PqmMsqi *msqi);
    void WriteStat2Shm(SaveFileType type, time_t time);

    unsigned short frequency(int type=2) { 
        if (type<2) return frequency_[type];
        else return frequency_[freq_idx_]; };
    float Plt(int phs){ return Plt_[phs]; };
    float Pst(int phs){ return Pst_[phs]; };
	float u_deviation(int phs){ return (u_deviation_.array[0][phs]-u_din_)/u_din_; };
   	float i_rms(int type, int phs) {    //type:1=primary,2=secondary
   	    if (type==1) {
   	        float fi = line_para_->CT1; fi /= line_para_->CT2;
   	        return i_rms_[phs]*fi;
   	    } else return (float)i_rms_[phs]; };
   	float u_rms(int type, int phs) {    //type:1=primary,2=secondary
   	    if (type==1) {
   	        float fi = line_para_->PT1; fi /= line_para_->PT2;
   	        return u_deviation_.array[0][phs]*fi;
   	    } else return (float)u_deviation_.array[0][phs]; };
   	float u_rms_ppv(int type, int phs) {    //type:1=primary,2=secondary
   	    if (type==1) {
   	        float fi = line_para_->PT1; fi /= line_para_->PT2;
   	        return u_rms_ppv_[phs]*fi;
   	    } else return u_rms_ppv_[phs]; };

protected:

private:
	SysPara *syspara_;
	LinePar *line_para_;

	float u_din_;    //The declared input voltage
	int units_[3];   //scale factor. [0]voltage, [1]current, [2]frequency

	unsigned short frequency_[2];  //0=1ps, 1=10ps. unit:1/100Hz
	int freq_idx_; //
	long freq_aggr_[3];   //Frequency aggregate data. [0-2]:maximum, average, minimum. unit:1/100Hz
	long freq_aggr2_[3];  //Frequency aggregate data be dedicated to 61850. [0-2]:maximum, average, minimum. unit:1/100Hz
	int freq_cnt_;  //count for freq_aggr2_
	FreqSaveModel freq_svtmp_; //Frequecy temp buffer
	LoopBuffer<FreqSaveModel> * freq_save_; //Frequecy save loopbuffer

	UnblncData<unsigned short> unbalance_;
	UnblncData<unsigned short> unblc_aggr_[3];  //Unbalance aggregate data. [0-2]:maximum, average, minimum.
	UnblncData<float> unblc_average_;  //Unbalance average accumulation buffer
	UnblncSaveModel unblc_svtmp_; //Unbalance temp buffer
	LoopBuffer<UnblncSaveModel> * unblc_save_; //Unbalance save loopbuffer

	//Below all is Secondary value
	float u_rms_ppv_[3], i_rms_[3], u_dev_avg_[3]; //current & phase to phase voltage rms. [0-2]phase A,B,C. unit:refer to units_
	float i_rms_aggr_[3][3]; //Current. [0-2]:maximum, average, minimum. [0-2]phase A,B,C. unit:A
	VoltDvData<float> u_deviation_;
	VoltDvData<float> voltdv_aggr_[3]; //Voltage data. [0-2]:maximum, average, minimum. unit:V
	float voltdv_[3][3];   //[0-2]:maximum,average,minimum. [0-2]phase A,B,C. unit:%
	VoltDvSaveModel voltdv_svtmp_; //Voltage deviation temp buffer
	LoopBuffer<VoltDvSaveModel> * voltdv_save_; //Voltage deviation save loopbuffer

	float Pst_[3], Plt_[3], Pst12_buf_[3][20]; //Pst12_buf_ 存储了最近2小时的Pst
	int Pst_cnt_, Pst12_cnt_;
	PstSaveModel Pst_save_;  //Pst save buffer
	timeval pst_tmval_, plt_tmval_;
	
	float fluct_max_[3];    //maximum fluctuation in 10min. unit:V
    int fluct_cnt_[3];      //count in 2hours
    
	int refresh_cnt_[kVoltdvSave+1];   //data refresh count, be used to calculate average value
	unsigned short quality_[kVoltdvSave+1];
	
    void InitFreq();
    void InitRms();
    void InitUnblc();
    
	bool AlarmCheckUnbalance(unsigned int &words, time_t time);
	bool AlarmCheckFreq(unsigned int &words, time_t time);
    bool AlarmCheckDeviation(unsigned int&word, time_t time);
    bool AlarmCheckPst(unsigned int&word);
    void AggregatePst(time_t time, bool adj_tm);
    void AggregateFreq(time_t time, bool adj_tm);
    void AggregateUnblc(time_t time, bool adj_tm);
    void AggregateVoltdv(time_t time, bool adj_tm);
    void WriteCp95Shm(SaveFileType type, time_t time);
    void WriteQuality2Shm(SaveFileType type, int subtype=0);
};

extern OtherFunc *other_func;

#endif	//OTHER_FUNC_H
