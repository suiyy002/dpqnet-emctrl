/*! \file generic.cpp
    \brief Generic functions.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _GENERIC_H_
#define _GENERIC_H_
//---------------------------------------------------------------------------
#include <stdint.h>

#define setbit(x,y) (x|=(1<<y)) //��X�ĵ�Yλ��1
#define clrbit(x,y) (x&=~(1<<y)) //��X�ĵ�Yλ��0
#define spybit(x,y) (x&(1<<y)) //���X�ĵ�Yλ

int CompareInt(const void *arg1, const void *arg2);
int CompareFloat(const void *arg1, const void *arg2);
char *GenGuid(uint16_t *guid);
int AssertPath(const char * path);
int FloatFmt(float fdata, int totl, int decim);
unsigned int AdjSaveSpc(unsigned int space, int type);

#endif //_GENERIC_H_
