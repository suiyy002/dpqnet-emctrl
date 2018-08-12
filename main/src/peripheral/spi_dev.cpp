/*! \file spi_dev.cpp
    \brief SPI device class.
*/

#include "spi_dev.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

using namespace std;

/*!
    Input:  dev -- device name
            num -- transfer package number
*/
SpiDev::SpiDev(const char *dev, int num)
{
	f_des_ = open(dev, O_RDWR);
	if (f_des_ < 0) {
	    printf("*** Can't open device %s!\n", dev);
	    abort();
	}

    if (SetSpeed(6000000)<0) abort();   //6MHz
    if (SetWordBits(32)<0) abort();  //32bits per word
    if (SetMode(SPI_CPHA)<0) abort(); 

    ioc_trnsfr_ = new spi_ioc_transfer[num];
    grp_para_ = new GroupPara[num];
    memset(grp_para_, 0, sizeof(GroupPara[num]));
    grp_num_ = num;
}

SpiDev::~SpiDev()
{
    delete [] grp_para_;
    delete [] ioc_trnsfr_;
    close(f_des_);
}

/*!
Transmit-receive data groups
一次发送多组不同参数设置的数据

    Input:  nm -- group number
    Return: data number be transfered
*/
int SpiDev::TransferGrps(int nm)
{
	int i;
    for (i=0; i<nm&&i<grp_num_; i++) {
        memset(&ioc_trnsfr_[i], sizeof(spi_ioc_transfer), 0);
        if (!grp_para_[i].tx || !grp_para_[i].rx) break;
        ioc_trnsfr_[i].tx_buf = grp_para_[i].tx;
        ioc_trnsfr_[i].rx_buf = grp_para_[i].rx;
        ioc_trnsfr_[i].len = grp_para_[i].len;
		ioc_trnsfr_[i].delay_usecs = grp_para_[i].delay_us;
		ioc_trnsfr_[i].speed_hz = grp_para_[i].speed;
		ioc_trnsfr_[i].bits_per_word = grp_para_[i].word_bits;
    }
	
    int ret = ioctl(f_des_, SPI_IOC_MESSAGE(i), ioc_trnsfr_);
    if (ret < 1) perror("can't send spi message");
    return ret;
}

/*!
Transmit-receive data

    Input:  tx -- transmit buffer address
            rx -- received buffer address
            len -- size of buffer in bytes
    Return: data number be transfered
*/
int SpiDev::Transfer(void *tx, void *rx, int len)
{
	int i, ret, retval=0;
	
    memset(&ioc_trnsfr_[0], sizeof(spi_ioc_transfer), 0);
    int grp_sz = 4096;
    int times = len/grp_sz;
    int last = len%grp_sz;
    ioc_trnsfr_[0].len = grp_sz;
	ioc_trnsfr_[0].delay_usecs = grp_para_[0].delay_us;
    for (i=0; i<times; i++) {
        ioc_trnsfr_[0].tx_buf = (unsigned long)((uint8_t*)tx+grp_sz*i);
        ioc_trnsfr_[0].rx_buf = (unsigned long)((uint8_t*)rx+grp_sz*i);
        ret = ioctl(f_des_, SPI_IOC_MESSAGE(1), ioc_trnsfr_);
        if (ret < 1) perror("can't send spi message");
        retval += ret;
    }
    if (last>0) {
        ioc_trnsfr_[0].tx_buf = (unsigned long)((uint8_t*)tx+grp_sz*i);
        ioc_trnsfr_[0].rx_buf = (unsigned long)((uint8_t*)rx+grp_sz*i);
        ioc_trnsfr_[0].len = last;
        ret = ioctl(f_des_, SPI_IOC_MESSAGE(1), ioc_trnsfr_);
        if (ret < 1) perror("can't send spi message");
    }

    return retval+ret;
}

int SpiDev::SetMode(uint8_t mode)
{
	int ret = ioctl(f_des_, SPI_IOC_WR_MODE, &mode);
	if (ret == -1) perror("can't set spi mode");

	ret = ioctl(f_des_, SPI_IOC_RD_MODE, &mode);
	if (ret == -1) perror("can't get spi mode");
	else printf("spi mode: %d\n", mode);

    return ret;
}

int SpiDev::SetWordBits(uint8_t bits)
{
	int ret = ioctl(f_des_, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1) perror("can't set bits per word");

	ret = ioctl(f_des_, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1) perror("can't get bits per word");
	else {
	    word_bits_ = bits;
        printf("bits per word: %d\n", bits);
    }
    return ret;	
}

int SpiDev::SetSpeed(uint32_t speed)
{
	int ret = ioctl(f_des_, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1) perror("can't set bits per word");

	ret = ioctl(f_des_, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1) perror("can't get bits per word");
	else printf("max speed: %d kHz\n", speed / 1000);
    
    return ret;	
}

/*!
Set parameter of the device's groups

    Input:  idx -- index groups
            rx -- received buffer address
            rx -- transmit buffer address
            len -- size of buffer in bytes
            speed -- unit:Hz
            bits -- bits per word
*/
void SpiDev::SetGrpPara(int idx, void *tx, void *rx, int len, 
                        uint32_t speed, uint8_t bits)
{
    if (idx>=grp_num_||idx<0) return;
    grp_para_[idx].tx = (unsigned long)tx;
    grp_para_[idx].rx = (unsigned long)rx;
    grp_para_[idx].len = len;
    grp_para_[idx].speed = speed;
    grp_para_[idx].word_bits = bits;
}

#include "format_convrt.h"

/*!
SPI tansmit-receive data api

    Input:  cmd -- transfer request command. 3=fft, 5=pst
            tx -- transmit buffer address
            rx -- received buffer address
            cnt -- number of data in buffer
    Return: data number be transfered. -1=error
*/
int SpiApi::Transfer(SpiDev *spi, char cmd, void *tx, void *rx, int cnt)
{
	int ret=0;
	if (cnt<=0) return cnt;
	
	int size = cnt/4 + 1;   //align to 4 byte
	size *= 12;
	size += 8;
	if (size>buf_sz_) {
	    if (tx_buf_) delete [] tx_buf_;
	    if (rx_buf_) delete [] rx_buf_;
	    tx_buf_ = new uint8_t[size];
	    rx_buf_ = new uint8_t[size];
	    buf_sz_ = size;
	} 
	memset(tx_buf_, 0x7f, 4);
	tx_buf_[4] = cmd;
	
    Cpy32To24(&tx_buf_[8], (int*)tx, cnt);
    int num = spi->Transfer(tx_buf_, rx_buf_, size);
    if (size != num) {
        printf("number error! num=%d, size=%d\n", num, size);
        num = -1;
    } else {
        if (cmd==3) {   //fft
            Cpy24To32((int*)rx, &rx_buf_[8], cnt);
        } else {    //pst
            memcpy(rx, &rx_buf_[8], 96*4);
        }
    }
    return num;
}