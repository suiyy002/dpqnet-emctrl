#ifndef _SYS_PARA_H_
#define _SYS_PARA_H_

#include <stdint.h>
#include "../IPC/share_mem.h"

struct SysParaCls {
    uint32_t CUlevel() { return syspar_.line_para.CUlevel; };
    void set_CUlevel(uint32_t val) { syspar_.line_para.CUlevel = val; };
    float Volt_warp(int type) { return syspar_.line_para.Volt_warp[type]; };
    void set_Volt_warp(int type, float val) { syspar_.line_para.Volt_warp[type] = val; };
    float Short_cap() { return syspar_.line_para.Short_cap; };
    void set_Short_cap(float val) { syspar_.line_para.Short_cap = val; };
    float User_cap() { return syspar_.line_para.User_cap; };
    void set_User_cap(float val) { syspar_.line_para.User_cap = val; };
    float Supp_cap() { return syspar_.line_para.Supp_cap; };
    void set_Supp_cap(float val) { syspar_.line_para.Supp_cap = val; };

    uint16_t transt_monitor() { return syspar_.transt_monitor; };
    void set_transt_monitor(uint16_t val) { syspar_.transt_monitor = val; }; 
    uint16_t vvr_limit(int type) { return syspar_.vvr_limit[type]; };
    void set_vvr_limit(int type, uint16_t val) { syspar_.vvr_limit[type] = val; };
    uint16_t transt_i_monitor() { return syspar_.transt_i_monitor; };
    void set_transt_i_monitor(uint16_t val) { syspar_.transt_i_monitor = val; };
    uint16_t ivr_limit(int type) { return syspar_.ivr_limit[type]; }; 
    void set_ivr_limit(int type, uint16_t val) { syspar_.ivr_limit[type] = val; }; 
    uint8_t inter_enable() { return syspar_.inter_enable; };
    void set_inter_enable(uint8_t val) { syspar_.inter_enable = val; };
    uint16_t inter_limit() { return syspar_.inter_limit; };
    void set_inter_limit(uint16_t val) { syspar_.inter_limit = val; };
    uint16_t harm_rcd_space() { return syspar_.harm_rcd_space; };
    void set_harm_rcd_space(uint16_t val) { syspar_.harm_rcd_space = val; };
    uint8_t hrm_save_type() { return syspar_.hrm_save_type; };
    void set_hrm_save_type(uint8_t val) { syspar_.hrm_save_type = val; };
    uint16_t freq_rcd_space() { return syspar_.freq_rcd_space; };
    void set_freq_rcd_space(uint16_t val) { syspar_.freq_rcd_space = val; };
    uint8_t freq_save_type() { return syspar_.freq_save_type; };
    void set_freq_save_type(uint8_t val) { syspar_.freq_save_type = val; };
    uint16_t voltdv_rcd_space() { return syspar_.voltdv_rcd_space; };
    void set_voltdv_rcd_space(uint16_t val) { syspar_.voltdv_rcd_space = val; };
    uint8_t voltdv_save_type() { return syspar_.voltdv_save_type; };
    void set_voltdv_save_type(uint8_t val) { syspar_.voltdv_save_type = val; };
    uint16_t unbalance_rcd_space() { return syspar_.unbalance_rcd_space; };
    void set_unbalance_rcd_space(uint16_t val) { syspar_.unbalance_rcd_space = val; };
    uint8_t unbalance_save_type() { return syspar_.unbalance_save_type; };
    void set_unbalance_save_type(uint8_t val) { syspar_.unbalance_save_type = val; };
    
    uint8_t limit_type() { return syspar_.limit_type; };
    void set_limit_type(uint8_t val) { syspar_.limit_type = val; };
    uint8_t trigger_enable(int type) { return (syspar_.trigger_enable>>type)&1; };
    void set_trigger_enable(int type, uint8_t val) {
        if (val) syspar_.trigger_enable |= (1<<type);
        else syspar_.trigger_enable &= (~(1<<type)); };
    uint8_t trigger_enable() { return syspar_.trigger_enable; }; 
    void set_trigger_enable(uint8_t val) { syspar_.trigger_enable = val; };
    uint16_t unbalance_thr(int vc) { return syspar_.unbalance_thr[vc]; };
    void set_unbalance_thr(int vc, uint16_t val) { syspar_.unbalance_thr[vc] = val; };
    uint16_t neg_sequence_Ithr() { return syspar_.neg_sequence_Ithr; };
    void set_neg_sequence_Ithr(uint16_t val) { syspar_.neg_sequence_Ithr = val; };
    uint16_t freq_limit(int pn) { return syspar_.freq_limit[2]; };
    void set_freq_limit(int pn, uint16_t val) { syspar_.freq_limit[2] = val; };
    uint16_t pst_limit() { return syspar_.pst_limit; };
    void set_pst_limit(uint16_t val) { syspar_.pst_limit = val; };
    uint16_t plt_limit() { return syspar_.plt_limit; };
    void set_plt_limit(uint16_t val) { syspar_.plt_limit = val; };
    uint16_t harm_ulimit(int odr) { return syspar_.harm_ulimit[odr]; };
    void set_harm_ulimit(int odr, uint16_t val) { syspar_.harm_ulimit[odr] = val; };
    uint16_t harm_ilimit(int odr) { return syspar_.harm_ilimit[odr]; };
    void set_harm_ilimit(int odr, uint16_t val) { syspar_.harm_ilimit[odr] = val; };
    
    uint8_t connect_t() { return syspar_.connect_type; };
    void set_connect_t(uint8_t val) { syspar_.connect_type = val; };
    
    uint8_t transt_tb_enable() { return syspar_.transt_tb_enable; };
    void set_transt_tb_enable(uint8_t val) { syspar_.transt_tb_enable = val; };
    uint16_t tb_high_limit() { return syspar_.tb_high_limit; };
    void set_tb_high_limit(uint16_t val) { syspar_.tb_high_limit = val; };
    uint16_t tb_low_limit() { return syspar_.tb_low_limit; };
    void set_tb_low_limit(uint16_t val) { syspar_.tb_low_limit = val; };
    uint8_t manual_rec_enable() { return syspar_.manual_rec_enable; };
    void set_manual_rec_enable(uint8_t val) { syspar_.manual_rec_enable = val; };
    uint16_t transt_end_num() { return syspar_.transt_end_num; };
    void set_transt_end_num(uint16_t val) { syspar_.transt_end_num = val; };
    uint8_t start_cur_en() { return syspar_.start_cur_en; };
    void set_start_cur_en(uint8_t val) { syspar_.start_cur_en = val; };
    uint16_t sc_limit() { return syspar_.sc_limit; };
    void set_sc_limit(uint16_t val) { syspar_.sc_limit = val; };
    uint16_t scl_limit() { return syspar_.scl_limit; };
    void set_scl_limit(uint16_t val) { syspar_.scl_limit = val; };
    
    uint8_t gps_single_type() { return syspar_.gps_single_type; };
    void set_gps_single_type(uint8_t val) { syspar_.gps_single_type = val; };
    uint8_t gps_pulse_type() { return syspar_.gps_pulse_type; };
    void set_gps_pulse_type(uint8_t val) { syspar_.gps_pulse_type = val; };
    uint16_t proof_time_intr(int sm) { return syspar_.proof_time_intr[sm]; };
    void set_proof_time_intr(int sm, uint16_t val) { syspar_.proof_time_intr[sm] = val; };
    uint16_t b_time_intr() { return syspar_.b_time_intr; };
    void set_b_time_intr(uint16_t val) { syspar_.b_time_intr = val; };
    
    uint8_t cvt_modify_group() { return syspar_.cvt_modify_group; };
    void set_cvt_modify_group(uint8_t val) { syspar_.cvt_modify_group = val; };
    uint8_t cvt_modify_unit() { return syspar_.cvt_modify_unit; };
    void set_cvt_modify_unit(uint8_t val) { syspar_.cvt_modify_unit = val; };
    uint8_t cvt_modify_en() { return syspar_.cvt_modify_en; };
    void set_cvt_modify_en(uint8_t val) { syspar_.cvt_modify_en = val; };
    uint16_t cvt_modify(int odr) { return syspar_.cvt_modify[odr]; };
    void set_cvt_modify(int odr, uint16_t val) { syspar_.cvt_modify[odr] = val; };
    
    uint16_t fluct_limit() { return syspar_.fluct_limit; };
    void set_fluct_limit(uint16_t val) { syspar_.fluct_limit = val; };
    uint16_t fluct_db() { return syspar_.fluct_db; };
    void set_fluct_db(uint16_t val) { syspar_.fluct_db = val; };
    uint16_t fluct_enable() { return syspar_.fluct_enable; };
    void set_fluct_enable(uint16_t val) { syspar_.fluct_enable = val; };
    
    uint8_t inharm_type() { return syspar_.inharm_type; };
    void set_inharm_type(uint8_t val) { syspar_.inharm_type = val; };
    uint16_t harm_thdilimit() { return syspar_.harm_thdilimit; };
    void set_harm_thdilimit(uint16_t val) { syspar_.harm_thdilimit = val; };

private:
    friend class DataBuffer;
    SysPara syspar_;
};


#endif // _SYS_PARA_H_ 
