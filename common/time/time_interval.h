/*! \file time_interval.h
    \brief time interval.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _TIME_INTERVAL_H_
#define _TIME_INTERVAL_H_

class TimeInerval {
public:
    TimeInerval();
    ~TimeInerval() {};
    
    int TimeOut(long time);
    
    //accessors
    //long curr_time() { return curr_time_; };
    //mutators
    void set_interval(int val) { interval_ = val; };
private:
    int interval_;      //time interval. uint:s
    long curr_time_;    //current time
    long next_time_;
};

#endif //_TIME_INTERVAL_H_
