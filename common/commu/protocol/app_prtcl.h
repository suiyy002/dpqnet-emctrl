#ifndef APPPRTCL_H
#define APPPRTCL_H

#include <stdio.h> 
typedef unsigned char uchar;

class AppPrtclBase {
public:
	AppPrtclBase() {};
	virtual ~AppPrtclBase() {};
	
	//通讯命令处理
	//virtual int cmd_treat(
	virtual int CmdReceive(
		unsigned char *rbuf,  //I,接收到的数据
		unsigned char * *tbuf, int *sz //O, 要发送的应答数据及其数目
		)=0;
	//判断取记录是否结束
	//Post process. data -- user define
	virtual void PostProcess(void *data)=0;
	//主动发送命令处理
    virtual int CmdSend(unsigned char **tbuf, int *sz, int cnt)=0;
protected:
private:
};

#endif // APPPRTCL_H
