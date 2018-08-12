#ifndef PHY_PRTCL_GPS_H
#define PHY_PRTCL_GPS_H

#include "phy_prtcl.h"

//---------- Physical layer type GPS protocol class -------------------

class PhyPrtclGps:public PhyPrtclBase
{
public:
	PhyPrtclGps();
	~PhyPrtclGps();
	
	int UnpackData(uchar *rxdata, int cnt, uchar *rxbuf, int *offset);
	int PackData(uchar *idata, uchar* &odata, int &num) { return 0;};
	int get_rxmaxn() { return RxMaxNum; };
protected:
private:
	static const int RxMaxNum = 200; //最大接收缓存大小

	int m_rx_cnt; //接收到的数据的总数
};

#endif // PHY_PRTCL_GPS_H 
