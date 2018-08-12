#ifndef _PARAM_CHNL_H_
#define _PARAM_CHNL_H_

#include <stdint.h>

enum PQParaType { kPQParaFreq, kPQParaHrm, kPQParaUnblc,
    kPQParaUdev,     //include Urms & Irms
    kPQParaEnd };

struct ParamChnl {   //Channel parameter
	uint16_t version;   //version of ChannelParam
	uint32_t trns_rto[2];   //transformation ratio. [0-1]:PT1,PT2 unit:V or CT1, CT2 unit:A
	uint8_t ulevel;         //voltage level. 0=380V, 1=6kV, 2=10kV, 3=35kV, 4=110kV, 5=220kV, 6=330kV, 7=500kV
	uint8_t connect_type;   //voltage connect type. 0=wye, 1=delta
	uint16_t vdev_lmt;      //voltage deviation limit. uint:1/1000.
	uint16_t unblnc_lmt;    //negtive unbalance limit. unit:1/1000. 
	uint16_t negcmp_lmt;    //current negtive component limit. unit: A. 
	uint8_t lmt_type;       //limit type. 0=GB, 1=customization
	uint16_t freq_lmt;      //frequency limit. unit:1/1000Hz
	uint16_t pst_lmt;       //Pst limit. unit:0.01
	uint16_t plt_lmt;       //Plt limit. unit:0.01
	uint16_t hrm_lmt[50];   //[0]:THDu, unit:1/1000. [1-49]:2~50 harmonics limit. HRu for voltage, unit:1/1000. 
	                        //amplitude for current, unit:1/10A
	uint16_t fluct_db;      //Fluctuation deadband, unit:0.001/%
	uint16_t fluct_en;      //Fluctuation measurement enable. bit0-2:A-C
	
	uint8_t rce_en;     //rapid change event monitor enable
	uint8_t rce_rate_en;    //rate of change event monitor enable
    uint16_t rce_limit[3];  //limit of rce. [0-2]:swell,dip,interrupt. unit:1/1000
	uint16_t rce_rate_lmt[2];   //limit of rate of change. [0-1]:high,low. unit:1/1000

	uint16_t rce_end_num;    //record cycle number after rce
	//uint8_t manual_rec_enable;	//manual record wave enable

	uint32_t res[31];       //reserve
};

const static int kChannelTol = 4;   //Total number of channel
#endif //_PARAM_CHNL_H_
