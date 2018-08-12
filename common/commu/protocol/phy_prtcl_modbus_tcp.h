#ifndef _PHY_PRTCL_MODBUS_TCP_H_
#define _PHY_PRTCL_MODBUS_TCP_H_

#include "phy_prtcl.h"

//---------- Physical layer type GPS protocol class -------------------

class PhyPrtclModbusTcp:public PhyPrtclBase
{
public:
	PhyPrtclModbusTcp();
	~PhyPrtclModbusTcp();
	
	int UnpackData(uchar *rxdata, int cnt, uchar *rxbuf, int *offset);
	int PackData(uchar *idata, uchar* &odata, int &num) { return 0;};
	int get_rxmaxn() { return RxMaxNum; };
protected:
private:
	static const int RxMaxNum = 300; //最大接收缓存大小

	int rx_cnt_; //接收到的数据的总数
	uint16_t tx_trn_id_;   //Request transaction Identifier
	uint16_t rx_trn_id_;   //Response transaction Identifier
};

#endif // _PHY_PRTCL_MODBUS_TCP_H_ 
