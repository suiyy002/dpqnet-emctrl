#ifndef _PHY_PRTCL_MODBUS_H_
#define _PHY_PRTCL_MODBUS_H_

#include "phy_prtcl.h"

//---------- Physical layer type GPS protocol class -------------------

class PhyPrtclModbus:public PhyPrtclBase
{
public:
	PhyPrtclModbus();
	~PhyPrtclModbus();
	
	int UnpackData(uchar *rxdata, int cnt, uchar *rxbuf, int *offset);
	int PackData(uchar *idata, uchar* &odata, int &num);
	int get_rxmaxn() { return RxMaxNum; };
protected:
private:
    uint16_t CRC16(uint8_t *msg, uint16_t count);
	static const int RxMaxNum = 300; //最大接收缓存大小
};

#endif // _PHY_PRTCL_MODBUS_H_ 
