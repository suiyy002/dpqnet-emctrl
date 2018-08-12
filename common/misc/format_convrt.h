#ifndef _FORMAT_CONVRT_H_
#define _FORMAT_CONVRT_H_
//---------------------------------------------------------------------------
#include <stdint.h>

void Cpy32To24(uint8_t *i24, int *i32, int num);
void Cpy24To32(int *i32, uint8_t *i24, int num);

#endif //_FORMAT_CONVRT_H_
