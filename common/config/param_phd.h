/*! \file param_phd.h
    \brief Physical device parameter.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _PARAM_PHY_H_
#define _PARAM_PHY_H_

#include <stdint.h>
#include "param_ld.h"

struct ParamPHD {   //Physical device parameter
	uint16_t version; 	//version of PHDParam
	uint16_t device_model;
	char device_id[16];
	uint16_t stts_spc[kPQParaEnd];   //statistic space. unit:s
	uint16_t lcm_dely_time; //LCD turn off waiting time. unit:minute
	uint8_t ip[2][4];   //ip address. [0-1]:eth0,eth1
	uint16_t port[2];   //port number. [0-1]:eth0,eth1
	uint8_t mask1[2][4];//netmask. [0-1]:eth0,eth1
	uint8_t mac[2][2];  //mac address. [0-1]:eth0,eth1
	uint8_t gate[2][4]; //gateway. [0-1]:eth0,eth1
	LDChnnlInfo ldchnl_inf[kChannelTol];
    uint8_t frqm_spc;   //frequency measure space. 0=1s, 1=3s, 2=10s
    uint8_t rce_tol;    //Rapid Change Event record total number.
    uint16_t rce_max_dur;   //Rapid Change Event maximum record duration. unit:s
    uint8_t hrm_type;   //harmonic value type. 0=Subgroup, 1=Component, 2=Group
    uint8_t ihrm_type;  //interharmonic value type. 0=Subgroup, 1=Group, 2=1/2
	uint8_t res[14];    //reserve
};

#endif //_PARAM_PHY_H_
