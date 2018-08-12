#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <poll.h>

using namespace std;

#include "time_cst.h"
//#include "../pqm_func/prmconfig.h"
//#include "../device/device.h"

#define CUSTOMIZED_TZ  //Customized definition time zone

static pthread_mutex_t mutex_time_;
void TimeCstInit()
{
    pthread_mutex_init (&mutex_time_,NULL);
}
void TimeCstEnd()
{
    pthread_mutex_destroy(&mutex_time_); //清除互斥锁mutex
}

/*!
Convert time_t to tm <local>, it's thread-safe
    Input:  src -- source
    Output: des -- destination
*/
void LocalTime(struct tm *des, const time_t *src)
{
    if (!src||!des) return;
    time_t tmt = *src;
#ifdef CUSTOMIZED_TZ
    tmt += TimeZone()*3600;
#endif
    pthread_mutex_lock(&mutex_time_);
    memcpy(des, localtime(&tmt), sizeof(tm));
    pthread_mutex_unlock(&mutex_time_);
}

/*!
Convert time_t to tm <utc>, it's thread-safe
    Input:  src -- source
    Output: des -- destination
*/
void GmTime(struct tm *des, const time_t *src)
{
    if (!src||!des) return;
    time_t tmt = *src;
    pthread_mutex_lock(&mutex_time_);
    memcpy(des, gmtime(&tmt), sizeof(tm));
    pthread_mutex_unlock(&mutex_time_);
}

/*!
Calculate time zone
    
    Return: hours
*/
int TimeZone()
{
    int tz;
#ifdef CUSTOMIZED_TZ
    //tz = prmcfg->timezone();
#else
    time_t timei = 86400;
    tm *ptmi;

    pthread_mutex_lock(&mutex_time_);
    ptmi = localtime(&timei);
    tz = ptmi->tm_hour;
    ptmi = gmtime(&timei);
    tz -= ptmi->tm_hour;
    pthread_mutex_unlock(&mutex_time_);
    if (tz>12) {
        tz -= 24;
    } else if (tz<-12) {
        tz += 24;
    }
#endif
    return tz;
}

/*!
Convert tm to time_t

    Input:  src -- source
            type -- 0=raw, 1=consider time zone
*/
time_t MakeTime(struct tm *src, int type)
{
    if (!src) return 0;

	src->tm_isdst = 0;
	time_t tm_t = mktime(src);
#ifdef CUSTOMIZED_TZ
    if (type) tm_t -= TimeZone()*3600;
#else
    if (!type) tm_t += TimeZone()*3600;
#endif
	return tm_t;
}

/*!
    Input:  src -- source
            type -- 0=raw, 1=consider time zone
*/
void SetTime(tm *ptm, int type)
{
    struct timeval tmvi;
    tmvi.tv_sec = MakeTime(ptm, type);
    tmvi.tv_usec = 0;
//    setlocaltime(&tmvi);  //waiting
}

/*!
    Input:  type -- output format
*/
char *NowTime (int type)
{
    static char strtim[32];
    time_t time1 = time(NULL);
    tm tmi;
    LocalTime(&tmi, &time1);
    switch (type) {
        case 1:
            strftime(strtim, 32, "%y%m%d_%H%M%S", &tmi);
            break;
        case 2:
            strftime(strtim, 32, "%y%m%d", &tmi);
            break;
        case 3:
            strftime(strtim, 32, "%H%M%S", &tmi);
            break;
        default:
            strftime(strtim, 32, "%Y-%m-%d %H:%M:%S", &tmi);
            break;
    }
    return strtim;
}

/*!
    Input:  id
            se -- start or end. 1=start, 0=end.
            desc -- description
*/
void StopWatch (int id, bool se, const char * desc)
{
    static struct timeval time_st_[10], time_end_[10];
    static const char *pdsc[10];
    if (id>=10) return;
        
    if (desc!=NULL) pdsc[id] = desc;
        
    if (se) gettimeofday(&time_st_[id], NULL);
    else {
        gettimeofday(&time_end_[id], NULL);
        time_end_[id].tv_sec -= time_st_[id].tv_sec;
        if (time_end_[id].tv_usec < time_st_[id].tv_usec) {
            time_end_[id].tv_usec += 1000000;
            time_end_[id].tv_sec -= 1;
        }
        time_end_[id].tv_usec -= time_st_[id].tv_usec;
        printf("%s spend %d.%06ds\n", pdsc[id], time_end_[id].tv_sec, time_end_[id].tv_usec);
    }
}

/*!
Sleep x ms
*/
void msSleep(int x)
{
    poll(NULL, 0, x);
}

/*!
Compares two timeval

    Input:  tmv1 --
            tmv2 --
    Return: -1=tmv<tmv2, 0=tmv1==tmv2, 1=tmv1>tmv2
*/
inline int TimevalCmp(timeval *tmv1, timeval *tmv2)
{
    if (tmv1->tv_sec < tmv2->tv_sec) {
        return -1;    
    } else if (tmv1->tv_sec > tmv2->tv_sec) {
        return 1;
    } else {
        if (tmv1->tv_usec < tmv2->tv_usec) {
            return -1;
        } else if (tmv1->tv_sec > tmv2->tv_sec) {
            return 1;
        } else {
            return 0;
        }
    }
}