#ifndef APP_PRTCL_GPS_H
#define APP_PRTCL_GPS_H

#include <sys/time.h>
#include "app_prtcl.h"

//---------- Applicate layer type GPS protocol class -------------------
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
class AppPrtclGps:public AppPrtclBase
{
public:
	AppPrtclGps();
	~AppPrtclGps();
	
	int CmdReceive(unsigned char *rbuf, unsigned char **tbuf, int *sz);
	void PostProcess(void *data) {};
    int CmdSend(unsigned char **tbuf, int *sz, int mark) { return 0; };
protected:
private:
    void SetIRIGBTime(int cnt, int ofst);
    void Yday2Mday(struct tm * ptm, int yday);

    char p_cnt_; //P 码元连续收到的次数
    char pr_find_;   //是否发现 Pr 码元
    char element_id_;   // 在每个数据帧中，码元序号
    int sec_old_, right_cnt_;  
    int timesyn_spc_;   //对时间隔, unit:s  
    int set_rtc_spc_;   //realtime clocl set space. unit:s;
    int syn_cnt_;   //连续有效对时次数，误差<xms
    //以下时间变量为BCB格式
    int seconds_;
    int minutes_;
    int hours_;
    int days_;
    //for_debug
    timeval time_st_, time_end_;
    unsigned short note2dsp_;   //notice to dsp to synchronize time
};

#endif // APP_PRTCL_GPS_H 
