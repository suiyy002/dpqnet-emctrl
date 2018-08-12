#ifndef PHYPRTCLB_H
#define PHYPRTCLB_H

#include "phy_prtcl.h"

//---------- Physical layer type B protocol class -------------------

#pragma pack(1) 
struct CommDataHead { //数据头结构定义
	unsigned long adler_sum;    //Adler32 校验和
	unsigned short dev_num;     //设备的通讯编号
	unsigned char prtcl_type;   //协议类型，0=自定义协议A，1=自定义协议B
	unsigned char prtcl_ver;    //协议版本
	unsigned char cmd;          //通讯命令
	unsigned short body_len;    //数据体的长度
	unsigned char compress;     //数据体是否压缩，0=未压缩，1=压缩
	unsigned char compress_alg; //数据体压缩算法, 1=ZLIB
	unsigned char frm_sn;       //帧序号
	unsigned char window_sz;    //窗口大小
	unsigned char equip_type;   //10=PQM-1，20=PQM-2，30=PQM-3/PQM302，31=PQM-3A/PQM302A，32=PQM202，36=PQA800
	unsigned char unit_scale;   //单位精度。bit0~1:voltage; bit2~3:current;bit4~5:frequency
	                            //0=1/100, 1=1/1000, 2=1/10000, 3=1/100000
	unsigned char reserve[3]; //保留
};
#pragma pack() 

class PhyPrtclPqB:public PhyPrtclBase{

public:
	PhyPrtclPqB();
	~PhyPrtclPqB();
	
	int UnpackData(uchar *rxdata, int cnt, uchar *rxbuf, int *offset);
	int PackData(uchar *idata, uchar* &odata, int &num);
	int get_rxmaxn(){ return RxMaxNum*2; };
	void set_dev_num(uint16_t val) { dev_num_ = val; };
protected:
private:
	static const int RxMaxNum = 2048; //最大接收缓存大小
	static const int TxBufNum = 15360; //最大发送缓存大小
	uchar m_tx_buf[TxBufNum];

	int m_rx_cnt; //接收到的数据的总数
	CommDataHead m_data_hd; //数据头缓存
	uint16_t dev_num_;    //device communication number
};

#endif // PHYPRTCLB_H 
