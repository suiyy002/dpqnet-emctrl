#ifndef _COMMU_DISPTCHR_H_
#define _COMMU_DISPTCHR_H_
#include <stdlib.h>

//Communiction dispatcher
class CommuDisptchr {
public:
	CommuDisptchr();
	~CommuDisptchr();
	
	void SetAssocObj(void *dev, int phy, int app);
	int CeiveTrans();
    void PostProcess(void *data=NULL);
    int Restart();
    void Transmit();
    
protected:
private:
	int rx_max_num;
	unsigned char *phy_buffer; //物理层缓存
	unsigned char *rdbuf;  //设备读取缓存
	int send_mark_; //if data be send in last CeiveTrans() call. 1=true

	class CommuDevice *comm_dev_;
	class PhyPrtclBase *phy_prtcl_;
	class AppPrtclBase *app_prtcl_;
};

#endif // _COMMU_DISPTCHR_H_ 
