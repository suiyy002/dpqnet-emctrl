#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "../../pqm_func/prmconfig.h"
#include "../../device/device.h"
#include "../../base_func/conversion.h"
#include "../../base_func/time_cst.h"
#include "app_prtcl_gps.h"

AppPrtclGps::AppPrtclGps()
{
    timesyn_spc_ = 20;
    sec_old_ = 0;
    right_cnt_ = 0;
    set_rtc_spc_ = 1;
    syn_cnt_ = 0;
    note2dsp_ = 0;
    refresh_daram(&note2dsp_, 2, 5);  //通知DSP马上对时
}

AppPrtclGps::~AppPrtclGps()
{
}

/*!
Description: GPS 码元处理

    Input:  rbuf -- [0]:个数 
    Output: tbuf,sz -- 实际不用，为了兼容
    Return: 一次性返回的数据帧总数，始终为0
*/
int AppPrtclGps::CmdReceive(unsigned char *rbuf, unsigned char **tbuf, int *sz)
{
    //gettimeofday(&time_st_, NULL);
    int cnt = rbuf[0];
    for (int i=1; i<=cnt; i++) {
        if(!pr_find_) {  //Find Pr
            if (rbuf[i]==82) {    //82=P
                p_cnt_++;
            } else {
                p_cnt_ = 0;
                continue;
            }
            if(p_cnt_ == 2) { //Found Pr
                p_cnt_ = 0;
                //printf("gps time = %03x %02x:%02x:%02x\n", days_, hours_, minutes_, seconds_);
                if( prmcfg->gps_single_type() <= 1) {  //B码 or B码+脉冲
                    SetIRIGBTime(cnt, i-1);
                }
                pr_find_ = 1;
                element_id_ = 1;
                seconds_ = 0;
                minutes_ = 0;
                hours_ = 0;
                days_ = 0;
            }
        } else {
            if (rbuf[i]==0xcc) {
                pr_find_ = 0;
                continue;
            }
            switch(element_id_) {
                case 1:
                case 2:
                case 3:
                case 4: //single of seconds
                    seconds_ += rbuf[i]<< (element_id_-1);
                    break;
                case 6:
                case 7:
                case 8: //tens of seconds
                    seconds_ += rbuf[i]<< (element_id_-2);
                    break;
                case 10:
                case 11:
                case 12:
                case 13: //single of minutes
                    minutes_ += rbuf[i]<< (element_id_-10);
                    break;
                case 15:
                case 16:
                case 17: //tens of minutes
                    minutes_ += rbuf[i]<< (element_id_-11);
                    break;
                case 20:
                case 21:
                case 22:
                case 23: //single of hours
                    hours_ += rbuf[i]<< (element_id_-20);
                    break;
                case 25:
                case 26:    //tens of hours
                    hours_ += rbuf[i]<< (element_id_-21);
                    break;
                case 30:
                case 31:
                case 32:
                case 33:    //single of days
                    days_ += rbuf[i]<< (element_id_-30);
                    break;
                case 35:
                case 36:
                case 37:
                case 38:    //tens of days
                    days_ += rbuf[i]<< (element_id_-31);
                    break;
                case 40:
                case 41:    //hundreds of days
                    days_ += rbuf[i]<< (element_id_-32);
                    break;
                case 98:
                    pr_find_ = 0; //数据帧最后一个非P码元
                    break;
                default:
                    break;
            }
            element_id_++;
        }
    }    
    return 0;
}

/*!
set IRIG-B time to system time

    Input:  cnt -- the num of read ;
            ofst -- offset of Pr element;
*/
void AppPrtclGps::SetIRIGBTime(int cnt, int ofst)
{
    //Convert BCD to integer
    int sec = (seconds_>>4)*10 + (seconds_&0xf);
    int min = (minutes_>>4)*10 + (minutes_&0xf);
    int hour = (hours_>>4)*10 + (hours_&0xf);
    int yday = (days_>>8)*100 + ((days_>>4)&0xf)*10 + (days_&0xf);
    //判断时间的有效性
    int sec_new = hour * 3600 + min * 60 + sec;
    if(sec_new != sec_old_ + 1) right_cnt_ = 0;
    sec_old_ = sec_new;
    if(right_cnt_ < 4) {
        right_cnt_++;
        return;
    }
    
    time_t ctime = time(NULL);
    struct tm gps_tm; 
    LocalTime(&gps_tm, &ctime);
    gps_tm.tm_sec = sec + 1;
    gps_tm.tm_min = min;
    gps_tm.tm_hour = hour;
    Yday2Mday(&gps_tm, yday); 
    gps_tm.tm_isdst = 0;
    ctime = MakeTime(&gps_tm, prmcfg->timezone(1));
    struct timeval tmval;
    tmval.tv_sec = ctime;
    tmval.tv_usec = (cnt - ofst) * 10000 + 350 + prmcfg->modify_evn_stm()*1000;
    if (tmval.tv_usec<0) {
        tmval.tv_usec += 1000000;
        tmval.tv_sec--;
    }
    gettimeofday(&time_end_, NULL);
    //printf("time_st_=%d.%06d %d.%06d\n", time_st_.tv_sec, time_st_.tv_usec, time_end_.tv_sec, time_end_.tv_usec);
    printf("gps_time=%d.%06d local_time=%d.%06d\n", tmval.tv_sec, tmval.tv_usec, time_end_.tv_sec, time_end_.tv_usec);
    if (--timesyn_spc_ <= 0) {
        if (abs(tmval.tv_usec-time_end_.tv_usec)<1000) syn_cnt_++; //误差<1ms
        else syn_cnt_ = 0;
        if (syn_cnt_>=3) {
            syn_cnt_ = 0;
            if (set_rtc_spc_ <= 0) {
                set_rtc_spc_ = 1800/prmcfg->b_time_intr();  //set rtc time per 0.5hour
                rw_rtc_time(0, &(tmval.tv_sec));
            }
            refresh_daram(&note2dsp_, 2, 5);  //通知DSP马上对时
            timesyn_spc_ = prmcfg->b_time_intr(); 
            note2dsp_ ^= 0xff;
            push_debug_info("Time synchronized ");
        } else {
            settimeofday(&tmval, NULL);
        }
    }
    if (set_rtc_spc_>0) set_rtc_spc_--;
}

/*!
Convert days element of gps IRIG-B to day of the month

    Input:  yday -- days element of gps IRIG-B
*/
void AppPrtclGps::Yday2Mday(tm * ptm, int yday)
{
    int leap, month, mday;
    if(ptm->tm_year % 400 == 0 || (ptm->tm_year % 4 == 0 && ptm->tm_year % 100 != 0)) leap = 1;
    else leap = 0;
    if(ptm->tm_mon == 0 && yday > 334+leap) ptm->tm_year--; //本机为1月份,B码为12月,则年-1
    else if(ptm->tm_mon == 11 && yday < 32) ptm->tm_year++; //本机为12月份,B码为1月,则年+1
    
    if(yday >= 1 && yday <= 31) {
        ptm->tm_mon = 0;
        ptm->tm_mday = yday;
    } else if(yday <= 59 + leap) {
        ptm->tm_mon = 1;
        ptm->tm_mday = yday - 31;
    } else if(yday <= 90 + leap) {
        ptm->tm_mon = 2;
        ptm->tm_mday = yday - 59 - leap;
    } else if(yday <= 120 + leap) {
        ptm->tm_mon = 3;
        ptm->tm_mday = yday - 90 - leap;
    } else if(yday <= 151 + leap) {
        ptm->tm_mon = 4;
        ptm->tm_mday = yday - 120 - leap;
    } else if(yday <= 181 + leap) {
        ptm->tm_mon = 5;
        ptm->tm_mday = yday - 151 - leap;
    } else if(yday <= 212 + leap) {
        ptm->tm_mon = 6;
        ptm->tm_mday = yday - 181 - leap;
    } else if(yday <= 243 + leap) {
        ptm->tm_mon = 7;
        ptm->tm_mday = yday - 212 - leap;
    } else if(yday <= 273 + leap) {
        ptm->tm_mon = 8;
        ptm->tm_mday = yday - 243 - leap;
    } else if(yday <= 304 + leap) {
        ptm->tm_mon = 9;
        ptm->tm_mday = yday - 273 - leap;
    } else if(yday <= 334 + leap) {
        ptm->tm_mon = 10;
        ptm->tm_mday = yday - 304 - leap;
    } else if(yday <= 365 + leap) {
        ptm->tm_mon = 11;
        ptm->tm_mday = yday - 334 - leap;
    }
}
