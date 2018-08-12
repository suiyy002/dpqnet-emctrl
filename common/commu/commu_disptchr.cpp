#include "protocol/prtcl_interface.h"
#include "device/commu_device.h"
#include "commu_disptchr.h"

CommuDisptchr::CommuDisptchr()
{
    phy_buffer = NULL;
    rdbuf = NULL;
    phy_prtcl_ = NULL;
    app_prtcl_ = NULL;
    comm_dev_ = NULL;
    send_mark_ = 0;
}

CommuDisptchr::~CommuDisptchr()
{
    if (app_prtcl_) delete app_prtcl_;
    if (phy_prtcl_) delete phy_prtcl_;
    if (comm_dev_) delete comm_dev_;

    if(phy_buffer != NULL) {
        delete [] phy_buffer;
    }
    if(rdbuf != NULL) {
        delete [] rdbuf;
    }
}

/*!
Set association object for communication proccess

    Input:  dev_t -- communication device file descriptor
            phy_t -- physical protocol type
            app_t -- application protocol type
            idx -- index of communication object. 0~
*/
void CommuDisptchr::SetAssocObj(void *dev, int phy_t, int app_t, int idx)
{
    if (comm_dev_) delete comm_dev_;
    comm_dev_ = (CommuDevice*)dev;
    if (phy_prtcl_) delete phy_prtcl_;
    phy_prtcl_ = CreatePhyPrtcl((kPhyPrtclType)phy_t);
    if (app_prtcl_) delete app_prtcl_;
    app_prtcl_ = CreateAppPrtcl((kAppPrtclType)app_t, idx);

    rx_max_num = phy_prtcl_->get_rxmaxn();
    
    if(phy_buffer != NULL) {
        delete [] phy_buffer;
    }
    phy_buffer = new uchar[rx_max_num * 2];
    if(rdbuf != NULL) {
        delete [] rdbuf;
    }
    rdbuf = new uchar[rx_max_num];
}

/*!
Receive and transmit

    Return: 0=接收到有效数据; 1=接收到无效数据; 2=接收数据时,系统出错或通讯中断
            3=接收缓存未初始化; 4=通讯处理对象未初始化
*/
int CommuDisptchr::CeiveTrans()
{
    int i, n, j;
    int ofst[10];

    if(rdbuf == NULL || phy_buffer == NULL) return 3;
    if(comm_dev_ == NULL || phy_prtcl_ == NULL || app_prtcl_ == NULL) return 4;
    int cnt = comm_dev_->Read(rdbuf, rx_max_num);
    if(cnt <= 0) return 2;
    n = phy_prtcl_->UnpackData(rdbuf, cnt, phy_buffer, ofst);
    unsigned char *prbuf, *ptbuf[MaxWindowsSz], *ptmp;
    int sz[MaxWindowsSz];
    for(i = 0; i < n; i++) {
        prbuf = phy_buffer + ofst[i];
        cnt = app_prtcl_->CmdReceive(prbuf, ptbuf, sz);
        for (j = 0; j < cnt; j++) {
            phy_prtcl_->PackData(ptbuf[j], ptmp, sz[j]);    //在物理层打包要发送数据
            comm_dev_->Write(ptmp, sz[j]);
            delete [] ptbuf[j];
        }
    }
    if(n) {
        send_mark_ = 1;
        return 0;
    }
    return 1;
}

void CommuDisptchr::Transmit()
{
    unsigned char *ptbuf[MaxWindowsSz], *ptmp;
    int sz[MaxWindowsSz];
    int cnt = app_prtcl_->CmdSend(ptbuf, sz, send_mark_);
    send_mark_ = 0;
    for (int j = 0; j < cnt; j++) {
        phy_prtcl_->PackData(ptbuf[j], ptmp, sz[j]);    //在物理层打包要发送数据
        comm_dev_->Write(ptmp, sz[j]);
        delete [] ptbuf[j];
    }
}

void CommuDisptchr::PostProcess(void *data)
{
    app_prtcl_->PostProcess(data); 
}

int CommuDisptchr::Restart()
{
    return comm_dev_->Restart(); 
}

