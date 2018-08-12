#ifndef PHYPRTCL_H
#define PHYPRTCL_H

//#include "../../pqm_func/prmconfig.h"

#include "stdint.h"
typedef unsigned char uchar;

//---------- Physical layer protocol base class -------------------
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
class PhyPrtclBase{
public:
	PhyPrtclBase(){};
	virtual ~PhyPrtclBase(){};
	
	//从接收的数据中把有效数据提取出来
	// 返回值:接收到的有效数据包的个数
	// rxdata,cnt(I):从物理设备收到的数据及其个数
	// rxbuf(O):接收数据处理缓存
	// offset(O):有效数据包在rxbuf中的起始位置
	virtual int UnpackData(uchar *rxdata, int cnt, uchar *rxbuf, int *offset)=0;
	
	//打包要发送的数据
	// 返回值: 0,正确; 1,缓存溢出
	// iarray, num(I): 要打包的数据及其数目
	// oarray, num(O): 打包后的数据及其数目
	virtual int PackData(uchar *idata, uchar* &odata, int &num)=0;

	//获取最大接收缓存数
	virtual int get_rxmaxn()=0;
protected:
private:
};

#endif // PHYPRTCL_H 
