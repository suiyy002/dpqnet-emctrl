/*! \file match_ext.cpp
    \brief Extension mathematical functions.
*/

#include "math_ext.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

using namespace std;

/*!
Generate square wave. 
    input:  time -- cycle is 1
            dtyr -- duty ratio. unit:%
*/
double SquareWave(double time, double dtyr)
{
    dtyr /= 100;
    int t = time;
    if(time<0){
	    if((time-t)>=-dtyr) return -1; //low level
    	else return 1;                 //high level
	}else{
	    if((time-t)>=dtyr) return -1;  //low level
    	else return 1;                 //high level
    }
}

/*!
Rotate vector

    Input:  ang -- angle be rotated. 0=120, 1=240
    Output: c -- complex number be rotated
*/
void RotateVec(CComplexNum *c, int ang)
{
    float cos_ang, sin_ang;
    if (!ang) {   //+120бу i.e. -240бу
        cos_ang = -0.5;
        sin_ang = kSqrt3/2;
    } else {        //+240бу i.e. -120бу
        cos_ang = -0.5;
        sin_ang = -kSqrt3/2;
    }
    float fr = c->real;
    float fi = c->image;
    c->real = fr * cos_ang - fi * sin_ang;
    c->image = fr * sin_ang + fi * cos_ang;
}

/*!
Calculate sum of vector

    Input:  c -- vectors
            cnt -- count of vectors
    Return: mod(sum(c))
*/
inline float VectorsSum(const CComplexNum *c, int cnt)
{
    float fr=0, fi=0;
    for (int i=0; i<cnt; i++,c++) {
        fr += c->real;
        fi += c->image;
    }
    
    return sqrt(fr * fr + fi * fi);
}

/*!
Calculate the difference between two vectors

    Input:  c1 -- vector1
            c2 -- vector2
            cnt -- count of vectors
    Return: mod(c1-c2)
*/
float VectorsDiff(const CComplexNum *c1, const CComplexNum *c2)
{
	float fr = c1->real-c2->real;
	float fi = c1->image-c2->image;
	
    return sqrt(fr * fr + fi * fi);
}
