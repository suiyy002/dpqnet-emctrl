#ifndef _PARAM_CFG_H_
#define _PARAM_CFG_H_

//#include "harmfunc.h"
#include "generic.h"

const int kMdfyPrNum = 20;  //精度修正系数的组数
//update_flag definition
const unsigned int DaramUpdate =0x1; //DARAM参数设置改变
const unsigned int BaudrateUpdate = 0x2; //串口波特率设置改变
const unsigned int SocketPortIPUpdate = 0x4; //网络端口号设置改变
const unsigned int DaramFlagUpdate =0x8; //DARAM标志设置改变
const unsigned int SerialPrtclUpdate =0x10; //串口的通讯协议设置改变
const unsigned int SocketPrtclUpdate =0x20; //网口的通讯协议设置改变
//------------------------------------------------------------------------------

class ParamCfg{
public:
	ParamCfg();
	~ParamCfg();

    int ReadPhyParam(PhyDev *phdev);
    int ReadLDParam(LogicDev *ld);

    
protected:
private:

};

extern ParamCfg *param_cfg;
#endif //_PARAM_CFG_H_

