/*! \file shmem_srv61850.h
    \brief Share memory for 61850 server.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _SHMEM_SRV61850_H_
#define _SHMEM_SRV61850_H_

#include <time.h>
#include <sys/time.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//共享内存接口命令
typedef enum {
    kNoneCmd = 0, kInitSG, kSelectActSG, kSelectEditSG, 
    kConfEditSG, kStartRcdWave, kEndRcdWave, kColdReset,
    kGetIPAddrs, kGetNetmask, kGetGateway,
    kSetIPAddrs, kSetNetmask, kSetGateway, kPqmnVer, kLogin61850, kIdentify61850
} shm_APICmd;

typedef enum {
    kOverFreq, kUnderFreq, kPosVoltdv, kNegVoltdv, kAlmPst, kAlmPlt, kAlmFulct,
    kAlmUnblc, kAlmTHDu, kAlmHRU, kAlmHRI=58, 
    kAlmRmsVUplimt=107, kAlmRmsVLolimt, //暂态电压 高、低门限
    kAlmRmsAUplimt, kAlmRmsALolimt,     //暂态电流 高、低门限
    kAlmRmsVDiflimt, kAlmRmsADiflimt,    //暂态电压、电流突变门限
    kAlmTHDi,
    kAlmOddHRU, KAlmEvnHRU,     //奇次、偶次谐波含有率
    kAlmVLdintlimt,             //长期电压中断
    kAlmIUnblc,                 //电流不平衡
    kAlmINegseq                 //负序电流
} AlmType;

typedef enum {
    kSemNmVVStr, kSemNmVxStr,
    kSemNmVVEnd, kSemNmCommand=5
} SemaphoreNumber;  //信号量编号

typedef enum {
    kDRcdMade, kBrcbUpdate, kUrcbUpdate
} SafeSetDataID;

typedef struct {
	int32_t PT1,PT2;    //unit:V
	int32_t CT1,CT2;    //unit:A
	int32_t CUlevel; //当前电压等级
	float Volt_warp[2]; //正负电压偏差,unit:%
	float Short_cap; //短路容量.
	float User_cap; //用电协议容量.
	float Supp_cap;	//供电设备容量.
	char reserve[8];
} LinePar;

typedef struct {
	LinePar line_para; //线路相关参数
	
	uint16_t transt_monitor; //暂态数据监测使能
	uint16_t vvr_limit[3]; //voltage limit of swell, dip, interrupt. unit:1/1000
	uint16_t transt_i_monitor; //暂态电流监测使能
	uint16_t ivr_limit[2]; //暂态电流上限、下限值. unit:0.1A
	uint16_t reserve_2[2]; 
	uint8_t inter_enable; //间谐波处理使能.
	uint16_t inter_limit; //间谐波超限值,1/1000.
	uint16_t harm_rcd_space;//谐波数据存储间隔时间，单位s
	uint8_t hrm_save_type;//谐波记录取值类型，0=最大，1=平均值

	uint16_t freq_rcd_space;  //Frequency save space. unit:s
	uint8_t freq_save_type;   //Frequency save type.  0=Maximum，1=Average
	uint16_t voltdv_rcd_space;  //Voltage deviation save space. unit:s
	uint8_t voltdv_save_type;  //Voltage deviation save type. 0=Maximum，1=Average
	uint16_t unbalance_rcd_space;  //Unbalance save space. unit:s
	uint8_t unbalance_save_type;  //Unbalance save type. 0=Maximum, 1=Average

	uint8_t limit_type; //超限值类型，0=国标，1=自定义
	uint8_t trigger_enable; //触发录波使能，0=不触发，1=触发. bit0 total; bit1 freq; bit2 harm volt; bit3 harm current;
	                              //bit4 volt unbalance; bit5 current negtive; bit6 volt deviation
	uint16_t unbalance_thr[2];//Unbalance threshold of Voltage & Current，unit:1/1000，maximum:60000
	uint16_t neg_sequence_Ithr; //Negative sequence current threshold, unit:1/10A, maximum:60000
	uint16_t freq_limit[2];//频率正负偏差限值，单位1/100Hz
	uint16_t pst_limit;//Pst限值，单位1/100
	uint16_t plt_limit;//Plt限值，单位1/100
	uint16_t harm_ulimit[50];//电压THD及2~50次HRu限值，单位1/1000，最大值60000
	uint16_t harm_ilimit[49];//电流2~50次电流谐波限值，单位1/10A, 最大值60000

	uint8_t connect_type; //接线方式，0=Y, 1=△

	uint8_t transt_tb_enable; //暂态突变使能
	uint16_t tb_high_limit; //暂态突变上限值
	uint16_t tb_low_limit; //暂态突变下限值
	uint8_t manual_rec_enable;	///手动录波使能
	uint16_t transt_end_num;  //事件结束后，保存的周波数
	uint8_t start_cur_en; //启动电流使能
	uint16_t sc_limit;    //启动电流突升限值, unit:1/1000
	uint16_t scl_limit;   //启动电流突降限值, obsolete. waiting to re-enable

	uint8_t gps_single_type;  //GPS对时类型,0=B码+脉冲, 1=B码,2=脉冲,3=关
	uint8_t gps_pulse_type;   //脉冲对时类型,0=分脉冲, 1=秒脉冲
	uint16_t proof_time_intr[2]; //[0]:分脉冲对时间隔, [1]:秒脉冲对时间隔。脉冲个数
	uint16_t b_time_intr;     //GPS IRIG-B 对时间隔

	uint8_t cvt_modify_group;
	uint8_t cvt_modify_unit;
	uint8_t cvt_modify_en;  //CVT修正使能
	uint16_t cvt_modify[49]; //2~50次谐波CVT修正系数

	uint16_t fluct_limit; //Fluctuation limit, unit:0.01/%
	uint16_t fluct_db;    //Fluctuation deadband, unit:0.001/%
	uint16_t fluct_enable;//Fluctuation measurement enable. bit0-2:A-C
	
	uint8_t send_save_wave; //发送同步录波触发信号
	uint8_t recv_save_wave; //接收同步录波触发信号
	uint8_t recv_from_ip[16]; //要接收同步录波信号的源IP
	uint16_t ratefreq;    //Rated frequency, unit:1/100Hz
	uint8_t inharm_type;  //interharmonic group type. 0=group; 1=centred subgroup
	uint16_t harm_thdilimit; //电流THD限值，单位1/1000，最大值60000
	uint16_t ldint_limit; //long duration interrupt. unit:1/1000
	char reserve[55];
} SysPara;

typedef struct {
    char happen;        //event happened. set 1 by pqm3mn, clear 0 by 61850 svr
    char varstr;        //QVVR.VarStr.stVal -- Voltage event start
    struct timeval str_time;   //QVVR.VarStr.t -- Start time of the Voltage event
    char dipstr;        //QVVR.DipStr.stVal -- Dip event start
    char swlstr;        //QVVR.SwlStr.stVal -- Swell event start
    char intrstr;       //QVVR.IntrStr.stVal -- Interruption event start
    char varend;        //QVVR.VarEnd.stVal -- Voltage event finished
    struct timeval end_time;   //QVVR.VarEnd.t -- Finish time of the Voltage event
    int32_t vva;            //QVVR.VVa.mag -- Maximum voltage transient value(primary),unit:10mV
    int32_t vvatm;          //QVVR.VVaTm.mag -- Voltage transient duration, unit:ms
    int32_t rvtcnt[30];     //QVVR.EvtCnt.hstVal[x] -- 暂态事件直方图计数
} PqmQvvr;

typedef struct {
    char happen;        //event happened. set 1 by pqm3mn, clear 0 by 61850 svr
    struct timeval time;       //RDRE$RcdMade$t. Complete time
    char rcdmade;       //RDRE$RcdMade$stVal. TRUE = disturbance recording complete
    int32_t  fltnum;        //RDRE$FltNum$stVal. Fault number,此处用于区分不同录波文件的序号
    struct timeval trig_time;  //RDRE$RcdTrg$t. Trigger time
    char rcdtrg;        //RDRE$RcdTrg$stVal. Trigger recorder. External command to trigger recorder(TRUE)
} PqmRdre;

typedef struct {
    char Alm_stVal[128];    //GGIO.Almxx.stVal . 0=false, 1=true
    time_t Alm_t[128];      //GGIO.Almxx.t .
} PqmGgio;

typedef struct {
    char IVarStr_stVal;     //QITR.VarStr.stVal -- Current event start
    struct timeval IVarStr_time;   //QITR.VarStr.t -- Start time of the current event
    char IVarEnd_stVal;     //QITR.VarEnd.stVal -- Current event finished
    struct timeval IVarEnd_time;   //QITR.VarEnd.t -- Finish time of the current event
    int32_t MaxATrs;            //QITR.MaxATrs.mag -- Maximum current transient value,unit:uA
    int32_t ATrsTm;             //QITR.ATrsTm.mag -- Current transient duration, unit:ms
} PqmQitr;

typedef struct {
    char UVarStr_stVal;     //QVTR.VarStr.stVal -- Voltage event start
    struct timeval UVarStr_time;   //QVTR.VarStr.t -- Start time of the Voltage event
    char UVarEnd_stVal;     //QVTR.VarEnd.stVal -- Voltage event finished
    struct timeval UVarEnd_time;   //QVTR.VarEnd.t -- Finish time of the Voltage event
    int32_t MaxVTrs;            //QVTR.MaxVTrs.mag -- Maximum voltage transient value,unit:mV
    int32_t VTrsTm;             //QVTR.VTrsTm.mag -- Voltage transient duration, unit:ms
} PqmQvtr;

typedef struct {
    time_t st_time;
    time_t lt_time;
    float phpst[3];     //MFLK$MX$PhPst
    float pppst[3];     //MFLK$MX$PPPst
    float phplt[3];     //MFLK$MX$PhPlt
    float ppplt[3];     //MFLK$MX$PPPlt
    float phfluc[3];    //MFLK$MX$PhFluc
    float ppfluc[3];    //MFLK$MX$PPFluc
    float phflucf[3];   //MFLK$MX$PhFlucf
    float ppflucf[3];   //MFLK$MX$PPFlucf
    uint16_t q[3];//quality,0-2=pst,plt,fluctuation
} PqmMflk;

typedef struct {
    time_t on_time;     //LPHD$PwrUp$t
    char pwr_on;        //LPHD$PwrUp$stVal
    time_t off_time;    //LPHD$PwrDn$t
    char pwr_off;       //LPHD$PwrDn$stVal
} PqmLphd;

typedef struct {
    time_t time;
    //HA
    float ha[3][50];    //harmonic current rms.[0-2]:A-C, [0-49]:1-50
    float haang[3][50]; //harmonic current angle. [0-2]:A-C, [0-49]:1-50. real only
    //HPhV,HPPV
    float hphv[3][50];  //harmonic voltage rms for phase to neutral. [0-2]:A-C, [0-49]:1-50
    float hppv[3][50];  //harmonic voltage rms for phase to phase. [0-2]:AB-CA, [0-49]:1-50
    float hvang[3][50]; //harmonic voltage angle. [0-2]:A-C, [0-49]:1-50. real only
    //HR
    float hrphv[3][49]; //harmonic ratio for phase to neutral. [0-2]:A-C, [0-48]:2-50. unit:%
    float hrppv[3][49]; //harmonic ratio for phase to phase. [0-2]:AB-CA, [0-48]:2-50. unit:%
    //THD
    float thda[3];      //current THD. [0-2]:A-C. unit:%
    float thdodda[3];   //current odd THD. [0-2]:A-C. unit:%
    float thdevna[3];   //current even THD. [0-2]:A-C. unit:%
    float thdphv[3];    //voltage THD for phase to neutral. [0-2]:A-C. unit:%
    float thdoddphv[3]; //voltage odd THD for phase to neutral. [0-2]:A-C. unit:%
    float thdevnphv[3]; //voltage even THD for phase to neutral. [0-2]:A-C. unit:%
    float thdppv[3];    //voltage THD for phase to phase. [0-2]:AB-CA. unit:%
    float thdoddppv[3]; //voltage odd THD for phase to phase. [0-2]:AB-CA. unit:%
    float thdevnppv[3]; //voltage even THD for phase to phase. [0-2]:AB-CA. unit:%
    //Harmonic power
    float hw[4][50];    //[0-2]:A-C,all [0-49]:1-50
    float hvar[4][50];  //[0-2]:A-C,all [0-49]:1-50
    float hva[4][50];   //[0-2]:A-C,all [0-49]:1-50
    float hpf[4][50];   //[0-2]:A-C,all [0-49]:1-50
    //Total harmonic power    
    float tothw[4];     //active power. [0-3]A-C,all
    float tothvar[4];   //reactive power. [0-3]A-C,all
    float tothva[4];    //apparent power. [0-3]A-C,all
    float tothpf[4];    //power factor. [0-3]A-C,all
    uint16_t q;
} PqmMhai;

typedef struct {
    time_t time;
    float ha[3][50];    //interharmonic current rms.[0-2]:A-C, [0-49]:1-50
    float hphv[3][50];  //interharmonic voltage rms for phase to neutral. [0-2]:A-C, [0-49]:1-50
    float hppv[3][50];  //interharmonic voltage rms for phase to phase. [0-2]:A-C, [0-49]:1-50
    float hrphv[3][50]; //interharmonic voltage HR for phase to neutral.[0-2]:A-C, [0-49]:0-49. unit:%
    float hrppv[3][50]; //interharmonic voltage HR for phase to phase.[0-2]:AB-CA, [0-49]:0-49. unit:%
    uint16_t q;
} PqmMhaiIntr;

typedef struct {
    time_t time;        
    time_t hz_time;
    float hz;           
    float hzdev;        //Frequency deviation
    float a_mag[3];     //current rms. [0-2]:A-C    
    float phv_mag[3];   //voltage rms for phase to neutral. [0-2]:A-C
    float ppv_mag[3];   //voltage rms for phase to phase. [0-2]:AB-CA
    float phvdev[3];    //voltage deviation for phase to neutral. [0-2]:A-C
    float ppvdev[3];    //voltage deviation for phase to phase. [0-2]:AB-CA
    //power
    float w[4];         //A-C,all
    float var[4];       //A-C,all
    float va[4];        //A-C,all
    float pf[4];        //A-C,all
    uint16_t hz_q;    //Quality of frequency
    uint16_t q;       //Quality of frequency
} PqmMmxu;

typedef struct {
    time_t time;    //MSQI$xxx$t
    float seqa[3];  //MSQI$SeqA$cx$cVal$mag$f. pos-neg-zero
    float imbnga;   //MSQI$ImbNgA$mag$f
    float imbzroa;  //MSQI$ImbZroA$mag$f
    float seqv[3];  //MSQI$SeqV$cx$cVal$mag$f. pos-neg-zero
    float imbngv;   //MSQI$ImbNgV$mag$f
    float imbzrov;  //MSQI$ImbZroV$mag$f
    uint16_t q;
} PqmMsqi;

//共享内存数据区
struct Data4LD {
    PqmMmxu mmxu_real;
    PqmMsqi msqi_real;
    PqmMhai mhai_real;
    PqmMhaiIntr mhai_in_real;
    
    PqmMmxu mmxu_stat[4];       //[0]=maximum, [1]=average, [2]=minimum, [3]=CP95
    PqmMsqi msqi_stat[4];           //ditto
    PqmMhai mhai_stat[4];       //ditto
    PqmMhaiIntr mhai_in_stat[4];    //ditto
    
    PqmLphd lphd;
    PqmGgio ggio;
    PqmMflk mflk;
    PqmQitr qitr;
    PqmQvtr qvtr;
    PqmQvvr qvvr;
    PqmRdre rdre;
    
    //char flicker_ok;
    //char lphd_ok;
    //char statistic_ok;
    char ggio_ok;
    char qvvr_ok;
    char rdre_ok;
    
    char MonitorPoint[65]; //监测点名称
    char LDName[16];
    char brcb_up;   //statistic data has update. increased by pqm3mn, decreased by 61850
    char urcb_up;   //realtime data has update. increased by pqm3mn, decreased by 61850
    uint16_t debug_var; //varialbes for debug
    char reserve[24];   //[25]
};

const int32_t kChannelTol =  4;
//共享内存数据区
struct ShmemSvr61850 {
    Data4LD ld_data[kChannelTol];
    char StationName[65];  // 变电站名称
    char IEDName[16];
    uint16_t quit_cmd; //31729=quit
    char security_en;    //Information security enable. 0=disable, 1=enable
    char reserve[24];   //[25]
    shm_APICmd request_cmd;  //Request command from 61850 Server
    shm_APICmd response_cmd; //Reaponse command from pqm3mn
    int32_t resp_rec_num; //返回的记录数量
    char data[1024];
};

ShmemSvr61850 *InitShmSvr61850(); //Initialize share memory
int32_t SafeSetShm(ShmemSvr61850 *p, SafeSetDataID dataid, void * value, int32_t idx); //set share memory value under semaphore control
/*!
Send cmd to share memory in safe mode

    Input:  p -- pointer to share memory
            cmd -- command
            reqst -- request param
            size -- request param size
    Output: resp -- response data
    Return: number of return param. -1=failure
*/
int32_t shm_SendCmd(ShmemSvr61850 *p, shm_APICmd cmd, void *reqst, int32_t size, void *resp);
/* \brief Function shm_SendCmd(...) command list
cmd             reqst           size            resp                                return
-------------   -------         ------          ------------------------------      ------------------
kGetIPAddrs     NULL            0               char *ip. e.g."192.168.1.100"       strlen(ip)+1
kGetNetmask     NULL            0               char *netmask. e.g."255.0.0.0"      strlen(netmask)+1
kGetGateway     NULL            0               char *gateway. e.g."192.168.1.1"    strlen(gateway)+1
kSetIPAddrs     char *ip        strlen(ip)+1    none                                ignore
kSetNetmask     char *netmask   strlen(ip)+1    none                                ignore
kSetGateway     char *gateway   strlen(ip)+1    none                                ignore
kPqmnVer        NULL            0               char *ver. e.g."3.7.3_44"           strlen(ver)+1
kColdReset      NULL            0               none                                ignore
kLogin61850     uint32_t[4]     16              0=passed, 1=refused                 1
kIdentify61850  uint32_t[4]     16              0=passed, 1=refused                 1
*/

#ifdef __cplusplus
}
#endif

#endif  //_SHMEM_SRV61850_H_ 

