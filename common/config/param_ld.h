/*! \file param_ld.h
    \brief Logical device parameter.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _PARAM_LD_H_
#define _PARAM_LD_H_

#include <stdint.h>

union LDChnnlInfo {
    uint8_t idx[2]; //channel index. [0-1]:voltage/current. range:1~kChannelTol, 0=no used
    uint16_t stat;  //channel state. 0=not used
};

struct ParamLD {   //Logical device parameter
	uint16_t version; 	//version of LDParam
	float capacity[3];  //[0-2]:minimum short circuit, equipment supply, user agreement. unit:MVA
	uint8_t dre_tol;    //total number of disturbance record
	uint8_t flt_num;    //fault number
	uint32_t res[12];   //reserve
};

#endif //_PARAM_LD_H_
