#ifndef EE_WARNINGE_H
#define EE_WARNINGE_H

#include "stdio.h"
#include "loop_buffer.h"

const int kTHeatRecentNum = 3;  //变压器预警最近数据缓冲数量
const int kTwDisNum = 160;      //变压器预警数据显示点数

#pragma pack(1)
struct EEWCapParam { //Electric Equipment -- Capacitor Warning parameter
    short life_thr;     //life warning threshold. unit: 0.001 pu(per unit)
    short life_thr_cnt; //life warning threshold count. default=200 
    short w24h_thr;     //24h accumulate warning threshold. unit:second
    short volt1_thr;    //continue over voltage 1 threshold. unit:0.001 pu
    short voltdur1_thr; //continue over voltage 1 duration threshold. unit:second
    short volt2_thr;    //continue over voltage 2 threshold. unit:0.001 pu
    short voltdur2_thr; //continue over voltage 2 duration threshold. unit:second
    short curr_thr;     //over currents threshold. unit:0.001pu
    short cap_thr;      //over capacity threshold. unit:0.001pu
    short peakv_thr;    //over peak voltage threshold. unit:0.001pu
    char reserve[12];
};

struct EEWThrDaram { //Electric Equipment -- Warning threshold to be write to daram
    short c_volt_thr;   //capacitor over voltage threshold. unit: 0.01V
    short c_curr_thr;   //capacitor over currents threshold. unit:0.001A
    long c_cap_thr;     //capacitor over capacity threshold. unit:0.001var
    short c_peakv_thr;  //capacitor over peak voltage threshold. unit:0.01V
    short c_volt1_thr;  //capacitor continue over voltage1 threshold. unit: 0.01V
    short c_volt2_thr;  //capacitor continue over voltage2 threshold. unit: 0.01V
    char reserve[2];
};

struct EEWraParam { //Electric Equipment -- Gamma-Alpha Warning parameter
    short Th;           //r-a Threshold. unit: 0.001 pu(per unit), default=1.35
    long Ms;            //capacity of short circuit. unit:0.01MVA
    long Mc;            //capacity of capacitor. unit:0.01kvar
    long Uc;            //Nominal voltage of capacitor. unit:V
    short x;            //Reactance ratio of reactor. unit:0.01%
    long charct_hr;     //primary characteristic value of harmonic. bit0~23 correspond to 2~25 harmonic
    short beta;         //阻抗夹角β. unit:0.01°. default=90°
    char reserve[10];
};

struct EEWTParam {  //Electric Equipment -- Transformer Warning parameter
    short Pec_rpu;      //pu of Eddy Current loss, default=0.15, unit=1/1000
    long In;        //I_N of transformer, unit=1/100A
    char reserve[10];
};

struct EEWRecord{    //Electric Equipment Warning Record
    timeval start_time; //开始时间
    char type;      //数据类型。电容类：1=过电压，2=过电流，3=过容限，4=峰值过电压
                    //          变压器类: 8=变压器过热预警
    char phase;     //相别。1=A, 2=B, 3=C。变压器类忽略此项。
    long max;       //最大值。二次侧的值。
                    //单位：过电压为1/100V,过电流1/10000A, 过容限为1/100000var
                    //      变压器过热预警 1/1000 pu
    long duration;  //持续时间。 单位ms。变压器过热预警为 s
    char reserve[13];
    char version;   //版本号。从0开始递增
};

struct EEWStatistic {    //Electric Equipment Warning Statistics Record
    timeval update_time; //更新时间
    short warn_cnt[3] ; //Capacitor: A,B,C三相的预警次数。
                        //Transformer:[0] maximum,unit:0.001; [1-2] duration,unit:s
    char up_phs;    //最近更新的相别，1~3分别对应A~C相。变压器类忽略此项
    char over;      //预警次数是否超限，bit0~2分别对应A~C相, 1=超，0=未超。
                    //目前仅对寿命预警有效
};

enum EEWType {  //预警类型
    //电容器类
    kCWarnLife,     //寿命预警
    kCWarn24h,      //24h累计预警
    kCWarnOVolt1,   //持续过电压1预警
    kCWarnOVolt2,   //持续过电压2预警
    kCWarnOCurr,    //过电流预警
    kCWarnOCap,     //过容限预警
    kCWarnOPeak,    //峰值过电压预警
    //变压器类
    kTWarnOHeat=8,    //变压器过热预警
};

#define EEW_STAT_NUM 20  //统计类型的数目
struct EEWFileHead {  //Electric Equipment Warning save file head
    EEWStatistic eew_stat[EEW_STAT_NUM];    //Capacitor:[0-6]分别对应 寿命、24h累积、持续过电压1,2、过电流、过容限、峰值
                                            //Transformer:[0-2]分别对应: 最近、第2近、第3近变压器过热预警
    timeval lastest_time;   //最后一条记录对应的时间
    short rcd_num;  //预警记录总数
    short recent;   //最新更新，bit0~6 分别对应：寿命、24h累积、持续过电压1,2、过电流、过容限、峰值
                    //Transformer ignore this
    char reserve[4];
};

#define EEW_BUF_SIZE 50
struct EEWBuffer {  //Electric Equipment Warning Records Buffer
    EEWRecord eew_buf[EEW_BUF_SIZE];
    char size;
};

struct EEWSaveIdx {  //EEW record idx ready to save
    char num[3];
    char ary[3][EEW_STAT_NUM];
};

struct EEW24hEstimate {  //24小时累积预警评估
    int yday;
    long sum;       //unit:ms
    bool overrun;   //Out of limit
};
struct EEWDaramData {
    short type; //bit0~7, 1=over-voltage. 2=over-current, 3=over-capacity，4=peak voltage
    //bit8~15, phase. 1=A, 2=B, 3=C
    long max;   //unit:1/100V, 1/10000A, 1/100000var
    long duration;  //unit:ms
    short reserve; //凑够4的整数倍
};

struct EEWTData {   //Transformer warning data
    float P_Lr;     //等效负荷率 P_{Lr}
    float I_Th_pu;  //电流热效应限值 I_{Th-pu}
    float I_rms_pu; //电流有效值标幺值 I_{rms-pu}
};

#pragma pack()

class CEEWarning
{
public:
    CEEWarning();
    ~CEEWarning();
    EEWFileHead* p_eew_file_head() {return &eew_file_head_;};
    EEWFileHead* p_eewt_file_head() {return &eewt_file_head_;};
    short* p_eew_ra_thr() {return &eew_ra_thr_[0][0];};
    EEWCapParam * p_eew_cap_param() {return &eew_cap_param_;};
    EEWThrDaram * p_eew_thr_daram() {return &eew_thr_daram_;};
    EEWraParam * p_eew_ra_param() {return &eew_ra_param_;};
    EEWTParam * p_eew_t_param() {return &eew_t_param_;};
    EEWTData * p_eew_t_data() {return &eew_t_data_;};
    
    bool daram_update(){if(daram_update_) {daram_update_ = false; return true;} return false;};
    long show_harmonic() { return show_harmonic_; };
    void set_show_harmonic(long val) { show_harmonic_ = val; };
    void set_para_update(int type);
    char cap_life_warning() { return eew_file_head_.eew_stat[0].over; };
    
    int ReadRcdFile(char * buf, timeval* time, time_t etime, short max_num, int cmd);
    void ReadFromDaram(short notice);
    void SetStartTime(short notice);
    void SaveRcdFile(int type);
    void SaveParamFile();
    void TWHandle(float t_p_c, float t_p_ec, float I1);
    int GetTwBuffer(int num, float* pdata[3]);

protected:
    EEWFileHead eew_file_head_; //预警记录存储文件头
    EEWFileHead eewt_file_head_; //变压器预警记录存储文件头
    EEWBuffer eew_buffer_;      //预警记录缓存，暂存最新的记录
    EEWCapParam eew_cap_param_; //电容运行预警阈值
    EEWThrDaram eew_thr_daram_; //需写入 DARAM 中的预警阈值
    short eew_ra_thr_[24][3];   //gamma-alpha threshold set.{ath2,r2,K2},{ath3,r3,K3},...{ath25,r25,K25}
                                //athn,unit:0.01%; rn,unit:0.01; Kn,unit:0.001pu
    EEWraParam eew_ra_param_;   //gamma-alpha parameter.
    EEWTParam eew_t_param_;     //Transformer parameter.
    
    EEWDaramData eew_daram_buf[EEW_BUF_SIZE];
private:
    void IniParam();
    int PositionRcd(FILE * f_strm, timeval *time, short start, short end);
    int CompareTimeval(timeval *time1, timeval *time2);
    void UpdateStatistic(EEWRecord *pdata);
    void set_daram_update();
    
    int max_deep_;  //The maximum count that Function PositionRcd be called
    bool daram_update_;     //The param in daram need to be update?
	timeval start_time_;
	EEWSaveIdx save_idx_;
    EEWRecord save_buf_[EEW_BUF_SIZE];  //capacitor warning event save buffer
    int save_size_;
    bool last_saved_;
    EEW24hEstimate eew24h_[3];
    long show_harmonic_;    //harmonic be showed. bit0~23 correspond to 2~25 harmonic
    bool param_update_;     //If EEW param is update

    LoopBuffer<EEWTData> * ptw_buffer_; //transformer warning data buffer, for display & event handle
    LoopBuffer<EEWRecord> * ptw_record_; //transformer warning event save buffer
    EEWTData eew_t_data_;               //transformer warning data;
    EEWRecord eew_t_rcd_;               //transformer warning save record
    float tw_disbuf_[3][200];  //[0-2]分别对应P_Lr,I_Th_pu,I_rms_pu.transformer warning data for display
    int limit_cnt_, tw_count_;
    float tw_max_;
};

static const char * kEEWRcdFile = "save/eew/eew_rcd.sav"; //Electric equipment warning data save file
static const char * kEEWTRcdFile = "save/eew/eew_trcd.sav"; //Transformer warning data save file
static const char * kEEWParaFile = "save/eew/eew_para.sav"; //Electric equipment warning param file

extern CEEWarning *pee_warning;

#endif //EE_WARNINGE_H

