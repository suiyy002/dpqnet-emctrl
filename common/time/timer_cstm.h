/*! \file timer_cstm.h
    \brief customized timer class.
*/
#ifndef _TIMER_CSTM_H_
#define _TIMER_CSTM_H_

class TimerCstm {
public:
    TimerCstm() { stime_=0; duration_=0; };
    ~TimerCstm() {};
    
    void Start(int dur);
    bool TimeOut();
private:
    time_t stime_;      //timer start time
    time_t duration_;   //timer duration;  
};

#endif //_TIMER_CSTM_H_

