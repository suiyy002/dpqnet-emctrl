#ifndef COMTRADE_FUNC_H
#define COMTRADE_FUNC_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct { //param of Analog channel
    long An;    //sn
    char ch_id[65]; //channel id
    char ph[3]; //phase
    char ccbm[65];  //Circuit component by monitor
    char uu[33];    //units
    float a,b;      //ax+b
    float skew;     //the channel time skew(in μs) from start of sample period
    short min, max;
    float primary, secondary;
    char PS;        //p=primary, s=secondary
} SAChannelPara;

typedef struct { //param of Analog channel
    long Dn;    //sn
    char ch_id[65]; //channel id
    char ph[3]; //phase
    char ccbm[65];  //Circuit component by monitor
    char y;    //normal state of status channel
} SDChannelPara;

typedef struct { //param in cfg file
    char station_name[65]; char rec_dev_id[65]; char rev_year[5];
    short TT; short nA; short nD;
    SAChannelPara ** achannel_param;
    SDChannelPara ** dchannel_param;
    float lf;   //line frequecy
    short nrates; float samp; long endsamp;
    char starttm[32];   //start record wave time(1st point)
    char trigtm[32];    //trigger time
    char ft[7]; //file type
    float timemult; //time multiplication factor
    char tz_data[2];    //timezone of starttm&trigtm. [0]is hour, [1]is minutes
    char tz_recorder[2]; //timezone of recorder. [0]is hour, [1]is minutes
    char tmq, leap; //time quality, leap second
} SCfgParam;

class ComtradeFunc
{
public:
    ComtradeFunc(int idx);
    ~ComtradeFunc();

    int IniSaveData(struct timeval *tgtime);
    int SaveDataFile(int cnt, uint8_t *pbuf_u, uint8_t *pbuf_i);
    int EndSave(const char *cause, const char *type, int valid=0);
    void ClearFile();
    void Rdre2Shm();
    void SetDreAttr(uint8_t tol, uint8_t fltn);

    //Accessors
    int flt_num() { return flt_num_; };
    
    //Mutators
    void set_cause(short cause) { cause_ = cause; }
    void set_charcs(float val, float dur) { extre_ = val; dur_ = dur; };  //set characteristic value
    void set_cncttp(unsigned char val) { cncttp_ = val; };
    void set_flt_num (short number) { flt_num_ = number; }
    void set_ptct (int vc, const uint32_t *ps) { memcpy(ptct_[vc], ps, sizeof(uint32_t)*2); }
    void set_rec_dev_id(const char *id) { strcpy(cfgpara_.rec_dev_id, id); };
    void set_sample_frequency (float freq) { sample_freq_ = freq; }
    void set_trig_phs(uint8_t phs) { trig_phs_ = phs; };
private:
    void IniCfgPara(short nA, short nD);
    void RefreshInfo(int cnt);
    int SaveCfgFile(char *name, const char *cause, const char *type);
    int SaveInfFile(char *name);
    void SaveInfPseudo(FILE *fp, float pt2); //add user's pseudo requirement into info file
    void PushRdre(int type);

    int ld_idx_;
    SCfgParam cfgpara_;
    float ratio_[2];    //电压、电流比例系数
    uint32_t ptct_[2][2];  //[0-1]:PT,CT; [0-1]:Primary,Secondary
    float  sample_freq_;    //sample frequency. unit:Hz
    struct timeval starttime_;      //start record wave time(1st point)
    struct timeval trigtime_;       //trigger time
    short cause_;       //暂态事件触发原因

    int flt_num_;      //暂态存储文件序号, Fault number
    char ldname_[65];
    char iedname_[65];
    char going_;     //1=暂态进行中

    char comtrade_path_[128];   //COMTRADE FILE PATH
    char filename_[128];    //录波文件的名称
    FILE *f_strm_;
    long sn_;           //sample number
    float timestamp_;   //timestamp of sample data. unit:us
    float intrvl_;      //interval between two adjacent sampling points. unit:s
    float extre_;       //extremum. unit:V
    float dur_;         //event duration. unit:s
    uint8_t trig_phs_;  //rce trigger phase. 0-2=A-C
    unsigned char cncttp_;  //connection type. 0=wye, 1=delta
    uint8_t dre_tol_;   //disturbance record total number

    LoopBuffer<PqmRdre> *rdre_soe_; //qvvr sequence of event buffer
    pthread_mutex_t mutex_; //mutex lock
};

static const char UI[2][3] = {"U","I"};
static const char ABC[9][3] = {"AN","BN","CN","AB","BC","CA","A","B","C"};
static const char UNIT[3][3] = {"V","A","kV"};

#ifdef __cplusplus
}
#endif  //__cpluscplus

#endif  //COMTRADE_FUNC_H
