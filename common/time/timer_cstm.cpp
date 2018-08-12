#include <cstdio>
#include <cstdlib>
#include <string>
#include <ctime>
using namespace std;

#include "timer_cstm.h"

/*!
Start timer

    Input:  dur -- timer duration. unit:s
*/
void TimerCstm::Start(int dur)
{
    time(&stime_);
    duration_ = dur;
}

/*!
Judge timer whether is out

    Input:  id -- timer identification
    Return: true=out, false=not out
*/
bool TimerCstm::TimeOut()
{
    time_t nowt = time(NULL);
    if ((nowt-stime_) > duration_) {
        return true;
    } else {
        return false;
    }
}
