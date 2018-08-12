#ifndef HARMFUNC_H
#define HARMFUNC_H
#include "harmbase.h"
#include <cstring>
using namespace std;

static  const int kIntrReso[3] = {8, 10, 16};   //Interharmonic resolution

class CHarmFunc:public CHarmBase{
public:
	CHarmFunc(LinePar* par);
	~CHarmFunc(){};

	unsigned short harm_quality() { return harm_quality_; };
	void write_hamp(unsigned short *hu, unsigned short *hi);
	void write_exthamp(unsigned short *hu, unsigned short *hi, unsigned short valid);
	void write_hphs(unsigned short *hu, unsigned short *hi);
	void write_exthphs(unsigned short *hu, unsigned short *hi, unsigned short valid);
	void write_interhamp(unsigned short *hui);
	unsigned char *make_inter_struct(int &cnt, unsigned short *frmbufp=NULL); 
	void update_IntBase();

    void CalcDerivedData();
	int AlarmCheck(unsigned int&alarm_word, unsigned short *array=NULL);//谐波超限检查
    void CalcRmsppv(float* ppv, float*phv);
	int get_harm_rcd(int type, void *to);
	void get_thd_rcd(void *to);
	bool lack_phs(int vc, int phs=0x0ff);
	void SetRecTime(struct timeval * tim);
	float get_phase(int phs, int vc, int hms, int range=0);
	float get_power(int type,int phase,int hmnum=1);
	void InitAggregation();
	void AggregateData(time_t time);
    void GetShmReal(PqmMmxu *mmxu, PqmMhai *mhai, PqmMhaiIntr *mhai_in);
	short line_coef(int type) { return line_coef_[type]; };
	
	void set_intrh_reso_id(int id) { intrh_reso_id_ = id; };
    unsigned short * huphs() { return harmphs_[0].array[0]; };
    unsigned short * hiphs() { return harmphs_[1].array[0]; };
	float unbalance(int vc, int type) { return unbalance_[vc][type]; };
    time_t * refresh_time() { return &refresh_time_; };

protected:
private:
	HarmData <unsigned short> harmzip_[2];  //三相各次谐波记录幅值压缩格式. [0-1]:Voltage,Current. unit:mV/0.1mA
	HarmData <float> harm_avgbuf_[2];  //harm rms average temporary buffer. [0-1]:Voltage,Current
    HarmData <float> hm_power_[4];   //harm power. [0-2]:active,reactive,apparent,pf. unit:W/var/VA/
    HarmData <float> hm_power_aggr_[3][4];   //harm power aggregate data. [0-2]:maximum,average,minimum; [0-3]:active,reactive,apparent,pf. unit:W/var/VA
    float hm_pw3_aggr_[3][4][51];   //sum of 3phase harm power aggregate data. [0-2]:maximum,average,minimum; [0-3]:active,reactive,apparent,pf. unit:W/var/VA
	float hpower_[4][4]; //total harmonic power. [0-3]:active,reactive,apparent,PF; [0-3]:phase A-C,all. unit:W/var/VA/
	float hpower_aggr_[3][4][4]; //total harmonic power aggregate data.[0-2]:maximum, average, minimum;[0-3]:active,reactive,apparent,PF; [0-3]:phase A-C,all.. unit:W/var/VA
	float power_[4][4]; //total power. [0-3]:active,reactive,apparent,pf; [0-3]:phase A-C,all. unit:W/var/VA/
	float power_aggr_[3][4][4]; //total power aggregate data. [0-2]:maximum,average,minimum; [0-3]:active,reactive,apparent,pf; [0-3]:phase A-C,all. unit:W/var/VA
	
    //float fund_power_aggr_[3][3];   //fundamental power aggregate data. [0-2]:maximum,average,minimum; [0-2]:active,reactive,apparent. unit:W/var/VA
	//float hpower_[3][4]; //total harmonic power. [0-2]:active,reactive,apparent,PF; [0-3]:phase A-C,all. unit:kW/kvar/kVA/
	//float hpower_aggr_[3][4]; //total harmonic power aggregate data.[0-2]:maximum, average, minimum;[0-2]:active,reactive,apparent. unit:kW/kvar/kVA
	//float PF_[4];    //power factor.[0-3]:phase A-C,all.
	//float PF_aggr_[3][4]; //power factor aggregate. [0-2]:maximum,average,minimum; [0-3]:phase A-C,all.
	
	unsigned int hrm_refresh_cnt_;   //谐波数据更新的次数,用于计算平均值
	time_t refresh_time_;       //谐波数据刷新的时间
	float unbalance_[2][4];     //[0-1]voltage,current, [0-3]zero, positive, negative&unbalance(primary). unit:V A %
	
	float IntBase[2][50]; //间谐波超限的基准值
	unsigned short interhuimx[6][MAX_HARM_NUM+1][(INTERH_MAX_RESO-1)*2];  /*三相0~50次间谐波电压电流幅值最大值及其相位 */
	unsigned short inter_save_[6][MAX_HARM_NUM+1][(INTERH_MAX_RESO-1)*2];  /*三相0~50次间谐波电压电流幅值记录存储缓存 */
    unsigned char interbuf_[6*(MAX_HARM_NUM+1)*((INTERH_MAX_RESO-1)*2*2+3)+9];  //间谐波结构化后的有效数据缓存
	int intrh_reso_id_;   //Interharmonic resolution identification

    unsigned short alm_array_[297]; //低8位是谐波次数; 高8位:0,1=A相电压,电流;2,3=B相电压,电流; 4,5=C相电压,电流
                                    //谐波次数为1时，表示THDu
    unsigned short harm_quality_;
	void calc_hpower();
	void calc_glpower();
	float calc_uiph3(int type,int vc);
	void CalcUnbalance();
	void CalcTrnsfmrLoss();
    void hrm_prehandle();
	void inter_limit_hdl(unsigned short *tobufp, unsigned short *frmbufp);
	void stat_hr_thd(); //统计谐波含有率及畸变率
	void zip_harm_rcd(int type);
    void WriteStat2Shm(time_t time);
    void WriteCp95Shm(time_t time); //temporary
    void WriteQuality2Shm();
};

const unsigned short INTERH_RCD_HEAD   = 0xcfce; //间谐波记录头(old:0xcfcf)
const int HrmRcdSz = 720; //一条谐波记录的大小(Byte)
const int HrmextRcdSz = 600; //一条扩展谐波(26~50)记录的大小(Byte)

extern CHarmFunc *harmfunc;
#endif //HARMFUNC_H
