#ifndef _APP_PRTCL_PQB_H_
#define _APP_PRTCL_PQB_H_

#include "app_prtcl.h"
#include "app_prtcl_pq.h"

//---------- Applicate layer type B protocol class -------------------
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
class AppPrtclPqB:public AppPrtclBase, public AppPrtclPQ
{

struct FetchRcd {
    long left_num; //还余多少记录
    unsigned char pkgsn; //最近1包数据的包序列号
    int buf_tol; //当前缓存的记录总数
    int buf_idx; //要从记录缓存中读取的记录号
    long head;//取记录的起点
    unsigned int offset;//对于以组构成的记录，head就是组起点, offset是组内偏移
};

public:
	AppPrtclPqB();
	~AppPrtclPqB();
	
	int CmdReceive(unsigned char *rbuf, unsigned char **tbuf, int *sz);
	void PostProcess(void *data);
    int CmdSend(unsigned char **tbuf, int *sz, int cnt) { return 0; };
protected:
private:
	int tx_trnst_rec(uchar *rbuf);

	int TxPower01Time(uchar *rbuf);

    int stat_transt(timeval &start_time, timeval &end_time);
	int locate_transt_pos(timeval &time, unsigned int tol, unsigned int pos, int end);
	int man_rec(uchar *rbuf);
	int tx_latest_rec(uchar *rbuf);
    int TxEewRec(int cmd, uchar *rbuf);
    int TxOtherRec(int cmd, uchar *rbuf);
    int TxHarmRec(bool first, uchar *rbuf);
	
	FetchRcd fetch_rcd;
	unsigned char m_rx_buffer[2048];  //接收缓存 

	struct CommDataHead *m_data_hd; //数据头
};

#endif // _APP_PRTCL_PQB_H_ 
