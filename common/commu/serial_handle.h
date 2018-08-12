#ifndef _SERAIL_HANDLE_H_
#define _SERAIL_HANDLE_H_

class SerialHandle {
public:
	SerialHandle(int max);
	~SerialHandle();
	
    int Run(int timeout);
    void CreateCommuObj(int sn, int rate, int phy_t, int app_t);

protected:
private:
    int FindIdleFD(int sn);
    void DeleteCommuObj(int idx);
	
    class CommuDisptchr **commu_dispch_;
    struct pollfd *pl_fd_;  //use for poll()
    int *srlpt_num_;        //serial port number
    int max_connect_;       //maximum port be opened

    class TimerCstm *timer_idle_;  //idle timer
};

#endif // _COMMU_HANDLE_H_ 
