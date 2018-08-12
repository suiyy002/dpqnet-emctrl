/*! \file format_convrt.cpp
    \brief data format convert function.
*/

#include "format_convrt.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

using namespace std;

/*!
Copy 32-bit integer array to 24-bit integer array

    Input:  i32 -- 32-bit integer
    Output: i24 -- 24-bit integer
            num -- number be converted
*/
void Cpy32To24(uint8_t *i24, int *i32, int num)
{
    for (int i = 0; i < num; i++) {
        memcpy(i24, (uint8_t*)i32, 3);
        i24 += 3;
        i32++;
    }
}

/*!
Copy 24-bit integer array to 32-bit integer array

    Input:  i24 -- 24-bit integer
    Output: i32 -- 32-bit integer
            num -- number be converted
*/
void Cpy24To32(int *i32, uint8_t *i24, int num)
{
    for (int i = 0; i < num; i++) {
        memcpy((uint8_t*)i32 + 1, i24, 3);
        *i32 = (*i32) >> 8;
        i24 += 3;
        i32++;
    }
}

