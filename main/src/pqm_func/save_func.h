#ifndef SAVE_FUNC_H
#define SAVE_FUNC_H
#include "daram_usr.h"
#include "../base_func/conversion.h"
#include "../base_func/time_cst.h"
#include "../base_func/loopbuf_sort.h"
#include "other_func.h"
#include "prmconfig.h"
#include "../database/sqlite_db.h"

#pragma pack(1)

struct OClockRcd{  //Record in one o'clock
    char oclock;    //o'clock
    char unit;      //bit0~1:Voltage; bit2~3:Current; bit4~5:Frequency. 0=1/100, 1=1/1000, 2=1/10000, 3=1/100000
    short count;    //number of frequency data in this o'clock, [360,1200,3600]
    /*
    union {
        PstRecord [count] {    //Pst o'clock record data
            unsigned short secs;    //seconds of record in a o'clock. [0-3600)
            unsigned short data[3]; //Pst, unit:refer to OClockRcd Voltage
            unsigned short q;   //quality
            unsigned short rsv; //reserved
        };
        FreqRecord [count] {    //Frequency o'clock record data
            unsigned short secs;    //seconds of record in a o'clock. [0-3600)
            unsigned short data;    //frequency, unit:refer to OClockRcd
            unsigned short q;   //quality
            unsigned short rsv; //reserved
        };
        UnblcRecord [count] {  //Unbalance o'clock record data
            unsigned short secs;        //seconds of record in a o'clock. [0-3600)
            unsigned short data[2][3];  //[0-1]Voltage, current; [0-2] zero, positive, negative. 
            unsigned short q;   //quality
            unsigned short rsv[2]; //reserved
        };
        VoltdvRecord [count] {  //Voltage deviation o'clock record data
            unsigned short secs;        //seconds of record in a o'clock. [0-3600)
            unsigned short data[3][3];  //[0-2]rms, pos&neg deviation(unit:1/10000); [0-2] A,B,C
            unsigned short q;   //quality
            unsigned short rsv; //reserved
        };
    }; */
};

struct RcdFileHead {  //Record save file data structure
    long date;  //yyyymmdd. e.g. 20150814
    short count;    //number of OClockRcd in one day, [1-24]
    char reserved[6];
    //OClockRcd [count];
};

struct Hrm10CycBuf {
    struct timeval time[15];
    bool on_off;    //true=on, false=off
    float data[15][12];   //[0-14]=cycle; [0-11]=VA:amp,phase;VB:amp,phase;...IA:amp,phase;...IC:amp,phase.
    float rms[15][6];   //[0-14]=cycle; [0-5]=Ua,Ia,Ub,Ib,Uc,Ic
};

#pragma pack()

class SaveFunc
{
public:
	SaveFunc();
	~SaveFunc();

	int pst_save_delay() { return pst_save_delay_; };
	void set_power_01_tm(int type, time_t time){ power_01_tm_[type] = time; };
	void set_pst_save_delay(int val) { pst_save_delay_=val; };
	void set_harm_valid(int val) { harm_valid_ = val; };

	void SaveHandle(void); //记录保存处理
	int DetectSaveHarm(const time_t time);
    int DetectSaveRcd(const time_t time, SaveFileType type);
	void SaveHarmonic();
    void SaveRecData(SaveFileType type);
    int ReadRecData(char * buf, time_t* stime, time_t* etime, short max_num, SaveFileType type);
    int ReadPltData(char * buf, time_t* stime, time_t* etime, short max_num);
    void SavePower01Time();
    int ReadPower01Time(time_t* tmt, int cnt);
    void ClearDB(int num=0);
    void Savecyc10(void *p);
    inline void ChgTimeFrame(long time, int space, long *last_tm, long * next_tm) {
        *last_tm = time;
        tm tmi;
        GmTime(&tmi, &time);
        int k = (tmi.tm_min*60 + tmi.tm_sec)%space;
        if (k) *last_tm -= k;
        *next_tm = *last_tm + space;
    };
    int ReadHarms2Buf(long stime, long etime);
    int GetHarmFrmBuf(unsigned char **p) { return sqlite_db->GetHarmonic(p); }; 
    void ClearRecData();

protected:

private:
	SysPara *syspara;

    LoopBufSort<long> *psave_status_[kVoltdvSave+1]; //store record filename in sequence. e.g. 20150809=2015-8-9
    FILE *f_rcd_[kVoltdvSave+1];
    RcdFileHead rcd_file_head_[kVoltdvSave+1];
    OClockRcd oclock_rcd_[kVoltdvSave+1];
    int oclock_offset_[kVoltdvSave+1];  //Offset of current OClockRcd in the file
    bool adj_time_[kVoltdvSave+1];  //If need adjust time
    long last_time_[kVoltdvSave+1], next_time_[kVoltdvSave+1];
    
    short save_buf_[14];
    unsigned short thd_buf_[2][3];
  	time_t hrm_save_time_; //谐波保存时间
  	//time_t save_time_[kVoltdvSave+1]; //记录保存时间
	int harm_save_delay_, pst_save_delay_; //程序启动时，第一个谐波记录和闪变记录不保存
	time_t power_01_tm_[2];	//暂存开关机时间.[0-1]:off,on
	unsigned short harm_valid_; //扩展谐波有效标识. bit0=1:26~50有效; bit1=1:间谐波有效

    void LoadStatus(SaveFileType type);
    void SaveStatus(SaveFileType type, long date);
    bool OpenRecFile(SaveFileType type, tm *tim);
    int GetUnitVal(SaveFileType type);
    bool LocateRead(FILE *f_strm, tm *tim, RcdFileHead * rec_hd, OClockRcd *o_rec, int size);
    void LocateSave(FILE *f_strm, tm *tim, RcdFileHead * rec_hd, OClockRcd *o_rec, int size, int unit);
    
	void SetOctecInShort(unsigned short*p, int index, unsigned char val){
		int m=index/2; int ofst = (index%2)*8;
		p[m] |= (val<<ofst);
	};
};

const int kPower01MaxSz = 10;

extern SaveFunc *save_func;

#endif	//SAVE_FUNC_H
