#ifndef _MATH_EXT_H_
#define _MATH_EXT_H_
//---------------------------------------------------------------------------
#include <stdint.h>

static const float kM_PI = 3.14159265;
static const float kSqrt3 = 1.7320508;   //sqrt(3)

struct CComplexNum {  //custom complex number
    float real; float image; };

double SquareWave(double time, double dtyr=50);
void RotateVec(CComplexNum *c, int ang);
float VectorsSum(const CComplexNum *c, int cnt);
float VectorsDiff(const CComplexNum *c1, const CComplexNum *c2);

#endif //_MATH_EXT_H_
