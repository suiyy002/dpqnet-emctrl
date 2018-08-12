#ifndef _EEW_PARA_H_
#define _EEW_PARA_H_

#include <stdint.h>
#include "../EEW/ee_warning.h"

struct EWWParaCls {
//------- EEWCapParam -------------------------------------------------------------
    int16_t life_thr() { return cappar_.life_thr; };
    void set_life_thr(int16_t val) { cappar_.life_thr = val; };
    int16_t life_thr_cnt() { return cappar_.life_thr_cnt; };
    void set_life_thr_cnt(int16_t val) { cappar_.life_thr_cnt = val; };
    int16_t w24h_thr() { return cappar_.w24h_thr; };
    void set_w24h_thr(int16_t val) { cappar_.w24h_thr = val; };
    int16_t volt1_thr() { return cappar_.volt1_thr; };
    void set_volt1_thr(int16_t val) { cappar_.volt1_thr = val; };
    int16_t voltdur1_thr() { return cappar_.voltdur1_thr; };
    void set_voltdur1_thr(int16_t val) { cappar_.voltdur1_thr = val; };
    int16_t volt2_thr() { return cappar_.volt2_thr; };
    void set_volt2_thr(int16_t val) { cappar_.volt2_thr = val; };
    int16_t voltdur2_thr() { return cappar_.voltdur2_thr; };
    void set_voltdur2_thr(int16_t val) { cappar_.voltdur2_thr = val; };
    int16_t curr_thr() { return cappar_.curr_thr; };
    void set_curr_thr(int16_t val) { cappar_.curr_thr = val; };
    int16_t cap_thr() { return cappar_.cap_thr; };
    void set_cap_thr(int16_t val) { cappar_.cap_thr = val; };
    int16_t peakv_thr() { return cappar_.peakv_thr; };
    void set_peakv_thr(int16_t val) { cappar_.peakv_thr = val; };
//------- EEWraParam -------------------------------------------------------------
    uint16_t Th() { return rapar_.Th; };
    void set_Th(uint16_t val) { rapar_.Th = val; };
    uint32_t Ms() { return rapar_.Ms; };
    void set_Ms(uint32_t val) { rapar_.Ms = val; };
    uint32_t Mc() { return rapar_.Mc; };
    void set_Mc(uint32_t val) { rapar_.Mc = val; };
    uint32_t Uc() { return rapar_.Uc; };
    void set_Uc(uint32_t val) { rapar_.Uc = val; };
    uint16_t x() { return rapar_.x; };
    void set_x(uint16_t val) { rapar_.x = val; };
    uint32_t charct_hr() { return rapar_.charct_hr; };
    void set_charct_hr(uint32_t val) { rapar_.charct_hr = val; };
    uint16_t beta() { return rapar_.beta; };
    void set_beta(uint16_t val) { rapar_.beta = val; };
//------- EEWTParam -------------------------------------------------------------
    uint16_t Pec_rpu() { return trsfmpar_.Pec_rpu; };
    void set_Pec_rpu(uint16_t val) { trsfmpar_.Pec_rpu = val; };
    uint32_t In() { return trsfmpar_.In; };
    void set_In(uint32_t val) { trsfmpar_.In = val; };

    uint32_t show_harmonic() { return show_harmonic_; };
    void set_show_harmonic(uint32_t val) { show_harmonic_ = val; };

    EEWFileHead *c_fhead() { return &c_fhead_; };
    EEWFileHead *t_fhead() { return &t_fhead_; };
    int8_t cap_life_warning() { return c_fhead_.eew_stat[0].over; };
    EEWTData * p_eew_t_data() { return &ptw_buffer_[tw_num_>0?tw_num_-1:0]; };
    int16_t *p_eew_ra_thr() { return &ra_thr_[0][0]; };

private:
    friend class DataBuffer;
    EEWCapParam cappar_;
    EEWraParam rapar_;
    EEWTParam trsfmpar_;
    uint32_t show_harmonic_;
    
    EEWFileHead c_fhead_;   //Capacitor warning record file header
    EEWFileHead t_fhead_;   //Transformer warning record file header
    static const int kTRecMaxNum = 320; //maximum number of transformer record buffer
    uint16_t tw_num_;    //transformer warning record number
    EEWTData ptw_buffer_[kTRecMaxNum];  //transformer warning record buffer
    int16_t ra_thr_[24][3];   //gamma-alpha threshold set.{ath2,r2,K2},{ath3,r3,K3},...{ath25,r25,K25}
                            //athn,unit:0.01%; rn,unit:0.01; Kn,unit:0.001pu

};

#endif // _EEW_PARA_H_ 
