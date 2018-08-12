#ifndef _EQUIP_PARA_H_
#define _EQUIP_PARA_H_

//#include "dis_data_types.h"

struct EquipParam {
	uint16_t version; 	//version of EquipParam
	uint32_t device_sn;    //设备编号. e.g. 300010023
	uint16_t character_dc[6]; //采样电路的特征直流分量，分别为A相电压、电流，B相电压...,unit:ADC output
	uint16_t pqm_type;    //设备型号
	uint32_t BaudRate[2]; //串口波特率. [0]=RS485, [1]=IRIG-B
	uint8_t device_num; //设备通讯编号. range:1~99
    uint8_t pst_enable;        //pst计算使能
	uint16_t transt_max;  //暂态事件最大保存数
	uint16_t transt_rcd_time; //一个暂态事件最长记录时间
	uint16_t transt_tol_time; //暂态总的记录时间限额
	uint16_t v_datum;     //精度校正基准电压,单位1/100V.
	uint16_t c_datum;     //精度校正基准电流,单位1/100A.
	uint8_t phs_adj_enable;   //精度校准相别使能.
	uint8_t vc_adj_enable;    //精度校准电压电流使能.
	uint8_t current_clamp_en; //电流钳使能.
	uint8_t current_clamp_type;   //电流钳型号的序号.
	uint8_t current_clamp_ratio; //电流钳变比序号.
	uint16_t lcm_dely_time;       //LCM关闭等待时间. unit:minute
	uint16_t socket_server_port;  //TCP/IP端口号
	uint8_t ad_sample_rate;	//0=1024/8,1=2048/10,2=4096/10
	uint16_t reset_hyst_time;  //Reset Hysteresis time. unit:hours
	uint8_t wav_sample_rate;	//Record wave sampling rate. 0=10240Hz, 1=20480Hz
    int8_t timezone;           //bit0-4:pqm timezone,[0,24]-12 = [-12,12],unit:hours. bit5:gps timezone type,0=utc,1=local
    uint16_t harmnm10cyc;    //harmonic number of 10 cycle
    uint8_t freq_evaltm;    //frequency evaluate space time.0=1s, 1=10s
    int8_t modify_evn_stm;   //modify event start time
    uint8_t save_wave_en;          //录波数据保存使能
    uint8_t aggrgt_time_en;        //10min aggregation data time verify
    uint8_t rcd_wv_fmt;           //record wave save format.0=Boyuu, 1=ComTraDE
	uint8_t zero_thr[2];       //voltage/Current. unit:V/mA
	int16_t time_err;     //system time error. unit:ms
	uint8_t fltr_cpx;  //event wave filter threshold cpxx. [0,10]
	uint8_t clock_src;  //clock source. 0=rtc, 1=system
	uint8_t u_extrm;  //where voltage extrema value from. 0=cpu, 1=dsp
	uint16_t harm_rec_svmax; //harmonic record save maximum number. 
	uint8_t harm_rec_sven; //harmonic record save enable.
	uint8_t fix_smpl;     //fix frequency sample. 0x79=on, othe=off
	uint8_t hrtbt_61850;  //61850 server heartbeat monitor.  0x79=on, othe=off
	uint8_t security_en;  //Information security enable. 0=disable, 1=enable
	uint8_t time_diff;  //Permissible time difference on 61850 security check.unit s
	uint8_t audtlog_size;  //Audit log record file size. unit:kByte
	uint8_t cmtrd_sv_path;    //Comtrade file save path
	uint8_t alarm_enable;		//steady data alarm enable. bit0-4:harm,freq,pst,unbalance,voltdev
	uint8_t signl_simu_en;    //signal simulation enable.  0=disable, 1=enable
	uint8_t real_wave_zero; //主板侧实时波形过零点调节使能，0=开，1=关
	ChannelPara chnnl[4];   //
	uint8_t res[82];		//预留字段
};

struct ModifyPara {
	float sample_pt; //电压互感器变比系数
	float sample_ct; //电流互感器变比系数
	uint16_t v_res_ratio[3]; //采样板电压电阻系数,单位1/10000.
	uint16_t c_res_ratio[3]; //采样板电流电阻系数,单位1/10000.
	uint16_t CT1; //CT一次侧. 
	int16_t v_line_coef[3]; //电压线性度修正系数,单位mV.
	uint16_t version;  //bit15-8=0xa5, bit7-0 ver from 0 to 255
	char reserve[18];
};
const int kMdfyPrNum = 20;  //精度修正系数的组数

struct NetwkParam { //network parameters
    char ip[32];
    char nmask[32];     //subnet mask
    char gateway[32];
    char mac_addr[32];  //machine address
    char ntp_ip[32];    //ntp server ip
};

struct DebugPara{
	unsigned char debug_file_num; //debug文件的序号
	unsigned char debug_enable; //debug使能
	unsigned char Pst_mdfy_enable; //闪变修正使能
	unsigned char max_val_type;//电压谐波记录最大值取值类型，0=最大，1=第2大.obsolete
	unsigned char impule_filter_enable;//是否对冲击信号进行软件滤除
	unsigned char real_wave_zero;//主板侧实时波形过零点调节使能，0=开，1=关
	char reserve[58];   //保留
};

enum kSteadyAlmEn {kAlmHarmEn, kAlmFreqEn, kAlmPstEn, kAlmUnblcEn, kAlmVoltdvEn};

struct EquipParaCls {
	uint16_t version() { return eqppar_.version; };
	void set_version(uint16_t val) { eqppar_.version = val; };
	uint32_t device_sn() { return eqppar_.device_sn; };
	void set_device_sn(uint32_t val) { eqppar_.device_sn = val; };
	uint16_t character_dc(int idx) { return eqppar_.character_dc[idx]; };
	void set_character_dc(int idx, uint16_t val) { eqppar_.character_dc[idx] = val; };
	uint16_t pqm_type() { return eqppar_.pqm_type; };
	void set_pqm_type(uint16_t val) { eqppar_.pqm_type = val; };
	uint32_t BaudRate(int type) { return eqppar_.BaudRate[type]; };
	void set_BaudRate(int type, uint32_t val) { eqppar_.BaudRate[type] = val; };
	uint8_t device_num() { return eqppar_.device_num; };
	void set_device_num(uint8_t val) { eqppar_.device_num = val; };
    uint8_t pst_enable() { return eqppar_.pst_enable; };
    void set_pst_enable(uint8_t val) { eqppar_.pst_enable = val; };
	uint16_t transt_max() { return eqppar_.transt_max; };
	void set_transt_max(uint16_t val) { eqppar_.transt_max = val; };
	uint16_t transt_rcd_time() { return eqppar_.transt_rcd_time; };
	void set_transt_rcd_time(uint16_t val) { eqppar_.transt_rcd_time = val; };
	uint16_t transt_tol_time() { return eqppar_.transt_tol_time; };
	void set_transt_tol_time(uint16_t val) { eqppar_.transt_tol_time = val; };
	uint16_t v_datum() { return eqppar_.v_datum; };
	void set_v_datum(uint16_t val) { eqppar_.v_datum = val; };
	uint16_t c_datum() { return eqppar_.c_datum; };
	void set_c_datum(uint16_t val) { eqppar_.c_datum = val; };
	uint8_t phs_adj_enable() { return eqppar_.phs_adj_enable; };
	void set_phs_adj_enable(uint8_t val) { eqppar_.phs_adj_enable = val; };
	uint8_t vc_adj_enable() { return eqppar_.vc_adj_enable; };
	void set_vc_adj_enable(uint8_t val) { eqppar_.vc_adj_enable = val; };
	uint8_t current_clamp_en() { return eqppar_.current_clamp_en; };
	void set_current_clamp_en(uint8_t val) { eqppar_.current_clamp_en = val; };
	uint8_t current_clamp_type() { return eqppar_.current_clamp_type; };
	void set_current_clamp_type(uint8_t val) { eqppar_.current_clamp_type = val; };
	uint8_t current_clamp_ratio() { return eqppar_.current_clamp_ratio; };
	void set_current_clamp_ratio(uint8_t val) { eqppar_.current_clamp_ratio = val; };
	uint16_t lcm_dely_time() { return eqppar_.lcm_dely_time; };
	void set_lcm_dely_time(uint16_t val) { eqppar_.lcm_dely_time = val; };
	uint16_t socket_server_port() { return eqppar_.socket_server_port; };
	void set_socket_server_port(uint16_t val) { eqppar_.socket_server_port = val; };
	uint8_t ad_sample_rate() { return eqppar_.ad_sample_rate; };
	void set_ad_sample_rate(uint8_t val) { eqppar_.ad_sample_rate = val; };
	uint16_t reset_hyst_time() { return eqppar_.reset_hyst_time; };
	void set_reset_hyst_time(uint16_t val) { eqppar_.reset_hyst_time = val; };
	uint8_t wav_sample_rate() { return eqppar_.wav_sample_rate; };
	void set_wav_sample_rate(uint8_t val) { eqppar_.wav_sample_rate = val; };
    int8_t timezone(int type=0) {
        int8_t i;
        if (type==0) {
            i = eqppar_.timezone&0x1f;
            return i - 12;
        } else return (eqppar_.timezone>>5)&1; };
    /*!
        Input:  val -- for pqm timezone. [-12, 12]
                    -- for gps timezone type. 0=utc, 1=local
                type -- 0=pqm timezone, 1=gps timezone type
    */
    void set_timezone(int8_t val, int type=0) {
        if (type==0) {
            if (val > 12 || val < -12) val = 8;
            eqppar_.timezone &= 0xe0;
            eqppar_.timezone |= val+12;    
        } else {
            if (val > 1) val = 1;
            eqppar_.timezone &= 0xdf;
            eqppar_.timezone |= (val<<5);
        } };
    uint16_t harmnm10cyc() { return eqppar_.harmnm10cyc; };
    void set_harmnm10cyc(uint16_t val) { eqppar_.harmnm10cyc = val; };
    uint8_t freq_evaltm() { return eqppar_.freq_evaltm; };
    void set_freq_evaltm(uint8_t val) { eqppar_.freq_evaltm = val; };
    int8_t modify_evn_stm() { return eqppar_.modify_evn_stm; };
    void set_modify_evn_stm(int8_t val) { eqppar_.modify_evn_stm = val; };
    uint8_t save_wave_en() { return eqppar_.save_wave_en; };
    void set_save_wave_en(uint8_t val) { eqppar_.save_wave_en = val; };
    uint8_t aggrgt_time_en() { return eqppar_.aggrgt_time_en; };
    void set_aggrgt_time_en(uint8_t val) { eqppar_.aggrgt_time_en = val; };
    uint8_t rcd_wv_fmt() { return eqppar_.rcd_wv_fmt; };
    void set_rcd_wv_fmt(uint8_t val) { eqppar_.rcd_wv_fmt = val; };
	uint8_t zero_thr(int vc) { return eqppar_.zero_thr[vc]; };
	void set_zero_thr(int vc, uint8_t val) { eqppar_.zero_thr[vc] = val; };
	int16_t time_err() { return eqppar_.time_err; };
	void set_time_err(int16_t val) { eqppar_.time_err = val; };
	uint8_t fltr_cpx() { return eqppar_.fltr_cpx; };
	void set_fltr_cpx(uint8_t val) { eqppar_.fltr_cpx = val; };
	uint8_t clock_src() { return eqppar_.clock_src; };
	void set_clock_src(uint8_t val) { eqppar_.clock_src = val; };
	uint8_t u_extrm() { return eqppar_.u_extrm; };
	void set_u_extrm(uint8_t val) { eqppar_.u_extrm = val; };
	uint16_t harm_rec_svmax() { return eqppar_.harm_rec_svmax; };
	void set_harm_rec_svmax(uint16_t val) { eqppar_.harm_rec_svmax = val; };
	uint8_t harm_rec_sven() { return eqppar_.harm_rec_sven; };
	void set_harm_rec_sven(uint8_t val) { eqppar_.harm_rec_sven = val; };
	uint8_t fix_smpl() { return eqppar_.fix_smpl; };
	void set_fix_smpl(uint8_t val) { eqppar_.fix_smpl = val; };
	uint8_t hrtbt_61850() { return eqppar_.hrtbt_61850; };
	void set_hrtbt_61850(uint8_t val) { eqppar_.hrtbt_61850 = val; };
	uint8_t security_en() { return eqppar_.security_en; };
	void set_security_en(uint8_t val) { eqppar_.security_en = val; };
	uint8_t time_diff() { return eqppar_.time_diff; };
	void set_time_diff(uint8_t val) { eqppar_.time_diff = val; };
	uint8_t audtlog_size() { return eqppar_.audtlog_size; };
	void set_audtlog_size(uint8_t val) { eqppar_.audtlog_size = val; };
	uint8_t cmtrd_sv_path() { return eqppar_.cmtrd_sv_path; };
	void set_cmtrd_sv_path(uint8_t val) { eqppar_.cmtrd_sv_path = val; };
    uint8_t alarm_enable(kSteadyAlmEn type) { return eqppar_.alarm_enable&(1<<type); };
    void set_alarm_enable(kSteadyAlmEn type, uint8_t val) { 
        if (val) eqppar_.alarm_enable |= (1<<type); 
        else eqppar_.alarm_enable &= (~(1<<type)); 
    };
	uint8_t signl_simu_en() { return eqppar_.signl_simu_en; };
	void set_signl_simu_en(uint8_t val) { eqppar_.signl_simu_en = val; };
	uint8_t real_wave_zero() { return eqppar_.real_wave_zero; };
	void set_real_wave_zero(uint8_t val) { eqppar_.real_wave_zero = val; };
	uint8_t chnnl_bus(int idx) { return eqppar_.chnnl[idx].bus; };
	void set_chnnl_bus(int idx, uint8_t bus) { sys_para_sg_.chnnl[idx].bus = bus; };
	uint8_t chnnl_type(int idx) { return eqppar_.chnnl[idx].vc; };
	void set_chnnl_type(int idx, uint8_t type) { sys_para_sg_.chnnl[idx].vc = type; };

	float sample_pt() { return mdfypar_[now_mdfy_grp_].sample_pt; };
	void set_sample_pt(float val) { mdfypar_[now_mdfy_grp_].sample_pt = val; };
	float sample_ct() { return mdfypar_[now_mdfy_grp_].sample_ct; };
	void set_sample_ct(float val) { mdfypar_[now_mdfy_grp_].sample_ct = val; };
	uint16_t v_res_ratio(int phs) { return mdfypar_[now_mdfy_grp_].v_res_ratio[phs]; };
	void set_v_res_ratio(int phs, uint16_t val) { mdfypar_[now_mdfy_grp_].v_res_ratio[phs] = val; };
	uint16_t c_res_ratio(int phs) { return mdfypar_[now_mdfy_grp_].c_res_ratio[phs]; };
	void set_c_res_ratio(int phs, uint16_t val) { mdfypar_[now_mdfy_grp_].c_res_ratio[phs] = val; };
	uint16_t CT1() { return mdfypar_[now_mdfy_grp_].CT1; };
	void set_CT1(uint16_t val) { mdfypar_[now_mdfy_grp_].CT1 = val; };
	int16_t v_line_coef(int phs) { return mdfypar_[now_mdfy_grp_].v_line_coef[phs]; };
	void set_v_line_coef(int phs, int16_t val) { mdfypar_[now_mdfy_grp_].v_line_coef[phs] = val; };

    void get_ip(char *ip) { strncpy(ip, netpar_.ip, 32); };
    void set_ip(char *ip) { strncpy(netpar_.ip, ip, 32); };
    void get_nmask(char *mask) { strncpy(mask, netpar_.nmask, 32); };
    void set_nmask(char *mask) { strncpy(netpar_.nmask, mask, 32); };
    void get_gateway(char *gate) { strncpy(gate, netpar_.gateway, 32); };
    void set_gateway(char *gate) { strncpy(netpar_.gateway, gate, 32); };
    void get_mac_addr(char *mac) { strncpy(mac, netpar_.mac_addr, 32); };
    void set_mac_addr(char *mac) { strncpy(netpar_.mac_addr, mac, 32); };
    void get_ntp_ip(char *ip) { strncpy(ip, netpar_.ntp_ip, 32); };
    void set_ntp_ip(char *ip) { strncpy(netpar_.ntp_ip, ip, 32); };

	uint8_t debug_enable() { return dbgpar_.debug_enable; };
	void set_debug_enable(uint8_t val) { dbgpar_.debug_enable = val; };
	uint8_t Pst_mdfy_enable() { return dbgpar_.Pst_mdfy_enable; };
	void set_Pst_mdfy_enable(uint8_t val) { dbgpar_.Pst_mdfy_enable = val; };
private:
    friend class DataBuffer;
    EquipParam eqppar_;
	ModifyPara mdfypar_[kMdfyPrNum];
	int now_mdfy_grp_;
    NetwkParam netpar_;
    DebugPara dbgpar_;
    
};


#endif // _EQUIP_PARA_H_ 
