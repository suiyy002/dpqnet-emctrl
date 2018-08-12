/*! \file time_cst.h
    \brief customized defined time function.
*/

#ifndef TIME_CST_H_
#define TIME_CST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>

void TimeCstInit();
void TimeCstEnd();

char *NowTime (int type);
int TimeZone();
void SetTime(struct tm *ptm, int type);
void LocalTime(struct tm *des, const time_t *src);
void GmTime(struct tm *des, const time_t *src);
time_t MakeTime(struct tm *src, int type);
void StopWatch (int id, bool se, const char * desc=NULL);
void msSleep(int ms);
int TimevalCmp(timeval *tmv1, timeval *tmv2);

#ifdef __cplusplus
}
#endif

#endif //TIME_CST_H_

