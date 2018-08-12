#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <zlib.h>

#include "phy_prtcl_pqb.h"
#include "conversion.h"

PhyPrtclPqB::PhyPrtclPqB()
{
	m_rx_cnt = 1;
}

PhyPrtclPqB::~PhyPrtclPqB()
{
}

/*！
从接收的数据中把有效的应用层数据提取出来

    Input:	rxdata --
            cnt -- 从物理设备收到的数据及其个数
    Output:	rxbuf -- 接收数据缓存; 
            offset -- 有效数据包在rxbuf中的起始位置
    Return:	接收到的有效数据包的个数
*/
static const int FrmHdSz = 4; //Frame Head Size
static const int HeadSz = sizeof(CommDataHead); //Data Head Size
int PhyPrtclPqB::UnpackData(uchar *rxdata, int cnt, uchar *rxbuf, int *offset)
{
  int i,j,k,hd_end,reved_hd, not_all;
  int retval = 0;
  unsigned long sumchk;

  if(cnt>0&&cnt<RxMaxNum){
	for(i=0;i<cnt;i++){
		rxbuf[m_rx_cnt+i] = rxdata[i];
	}
	m_rx_cnt += cnt;
	j = 0;
	do{
		reved_hd = 0;
		while((j+FrmHdSz)<=m_rx_cnt) {  //搜索帧头
			if((rxbuf[j]==0x5c)&&(rxbuf[j+1]==0x7c)&&(rxbuf[j+2]==0x3b)&&(rxbuf[j+3]==0xcf)) {
				reved_hd = 1;
				break;
			}
			j++;
		}

		if(reved_hd){ //收到有效帧头
			not_all = 0;
			hd_end = j+FrmHdSz+HeadSz;
			if (hd_end<=m_rx_cnt) { //收到了完整的数据头
				k = j+FrmHdSz;
				memcpy(&m_data_hd, &rxbuf[k],HeadSz);
				if (m_data_hd.body_len>RxMaxNum) { // received bytes num isn't correct
					m_rx_cnt = 0;
					push_debug_info("B Received bytes num wrong!");
					return retval;
				}
				
				if ((hd_end+m_data_hd.body_len)<=m_rx_cnt) { //Received all
					if(m_data_hd.dev_num==dev_num_||m_data_hd.dev_num==0){ //address number is same
						sumchk = adler32(0, NULL, 0);
						if (m_data_hd.compress) {
							sumchk = adler32(sumchk, &rxbuf[k+4], HeadSz-4);
						} else {
							sumchk = adler32(sumchk, &rxbuf[k+4], 
									m_data_hd.body_len+HeadSz-4);
						}
						if (sumchk==m_data_hd.adler_sum) { //Check sum is correct
							retval++;
							*offset = j+FrmHdSz;
							offset++;
							if(retval>8) m_rx_cnt = 0;
						} else {
							push_debug_info("Check sum error!");
						}
					}  					
					j = hd_end + m_data_hd.body_len - 1;
				} else {
					not_all = 1;
				}
			} else {
				not_all = 1;
			}
			if (not_all) { //未收到完整的数据帧
				for(i=0;(j+i)<m_rx_cnt;i++){
					rxbuf[i] = rxbuf[j+i];
				}
				m_rx_cnt = i;
				j = i;
			}
		} else {
			//push_debug_info("No Received Head!");
			rxbuf[0] = rxbuf[j];
			rxbuf[1] = rxbuf[j+1];
			rxbuf[2] = rxbuf[j+2];
			m_rx_cnt = 3;
       	    j = 3;
		}
	} while(j<m_rx_cnt);
  }
  return retval;
}

//---------------------------------------------------------------------------
//Description:	打包要发送的数据
//Input:	iarray,num-要打包的数据及其数目;
//Output:	oarray, num: 打包后的数据及其数目
//Return:	0,正确; 1,缓存溢出
int PhyPrtclPqB::PackData(uchar *idata, uchar* &odata, int &num)
{
	int i;
	unsigned long li,chksum;
	CommDataHead *d_hd;
	li = num*101/100+20;
	if(li>TxBufNum) return 1;
	
	m_tx_buf[0] = 0xee;
	m_tx_buf[1] = 0xee;
	m_tx_buf[2] = 0x55;
	m_tx_buf[3] = 0xaa;
	m_tx_buf[4] = 0x3c;
	m_tx_buf[5] = 0xc3;
	
	memcpy(&m_data_hd, idata, HeadSz);
	if (m_data_hd.compress) { //数据体需要压缩
		compress(&m_tx_buf[6+HeadSz], &li,
			   idata+HeadSz, m_data_hd.body_len);
		//printf("body_len=%d %d\n",m_data_hd.body_len, li);
		m_data_hd.body_len = li;
		i = HeadSz-4;
	} else {
		memcpy(&m_tx_buf[6+HeadSz], idata+HeadSz, m_data_hd.body_len);
		//printf("body_len=%d\n",m_data_hd.body_len);
		i = HeadSz-4+m_data_hd.body_len;
	}
	memcpy(&m_tx_buf[10], &m_data_hd.dev_num, HeadSz-4);
	chksum = adler32(0, NULL, 0);
	m_data_hd.adler_sum = adler32(chksum, &m_tx_buf[10], i);
	memcpy(&m_tx_buf[6], &m_data_hd, sizeof(m_data_hd.adler_sum));

	i = 6+HeadSz+m_data_hd.body_len;
	//m_tx_buf[i++]=0xcc;
	//m_tx_buf[i++]=0xcc;
	//m_tx_buf[i++]=0xcc;
	//m_tx_buf[i++]=0xcc;
	
	odata = m_tx_buf;
	num = i;
	return 0;
}
