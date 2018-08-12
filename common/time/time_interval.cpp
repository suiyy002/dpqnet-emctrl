#include <cstdio>
#include <cstdlib>
#include <string>
using namespace std;

#include "time_interval.h"

TimeInerval::TimeInerval()
{
    interval_ = 10;
    curr_time_ = 0;
    next_time_ = 0;
}

/*!
Judge timer whether is out

    Input:  time
    Return: 0=not out, 1=out, 2=out of range
*/
int TimeInerval::TimeOut(long time)
{
    int ret = 0;
    if (time < curr_time_ || time >= next_time_) {
        if (time == next_time_) { //It's time
            ret = 1;
            //adj_time_ = false;
        } else {    //Current time is out of range, because time setting etc
            ret = 2;
            //adj_time_ = true;
        }
        curr_time_ = time;
        int n = time%interval_;
        if (n==0) {
            next_time_ = time + interval_;
        } else {
            if (n<interval_/3) next_time_ = time - n + interval_;
            else next_time_ = time - n + interval_*2;
        }
    }
}
