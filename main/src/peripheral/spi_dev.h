#ifndef _SPI_DEV_H_
#define _SPI_DEV_H_
//---------------------------------------------------------------------------
#include <stdint.h>
#include <linux/spi/spidev.h>   // /usr/local/arm/cross/am335xt3/devkit/arm-arago-linux-gnueabi/usr/include

class SpiDev
{
public:
    SpiDev(const char *dev, int num=1);
    ~SpiDev();

    int SetSpeed(uint32_t speed);
    int SetWordBits(uint8_t bits);
    int SetMode(uint8_t mode);
    int Transfer(void *tx, void *rx, int len);

    void SetGrpPara(int idx, void *tx, void *rx, int len, 
                        uint32_t speed=0, uint8_t bits=0);
    int TransferGrps(int nm);
    
protected:
    struct GroupPara {
        unsigned long tx;
        unsigned long rx;
        int len;
        uint32_t speed;     //unit:Hz
        uint8_t word_bits;  //bits per word
        uint16_t delay_us;  //unit:us. invalid
    };
private:
    int f_des_;     //file descriptor
    struct spi_ioc_transfer *ioc_trnsfr_;
    GroupPara *grp_para_;
    int grp_num_;   //maximum group number be transfered

    uint8_t word_bits_;  //bits per word
    //uint8_t mode_;    //work mode. SPI_CPHA | SPI_CPOL | SPI_CS_HIGH | SPI_LSB_FIRST | SPI_3WIRE | SPI_LOOP | SPI_NO_CS | SPI_READY

};

class SpiApi
{
public:
    SpiApi(){tx_buf_ = NULL; rx_buf_ = NULL; buf_sz_ = 0;};
    ~SpiApi(){
        if (tx_buf_) delete [] tx_buf_;
        if (rx_buf_) delete [] rx_buf_;
    };

    int Transfer(SpiDev *spi, char cmd, void *tx, void *rx, int cnt);

protected:

private:
    int buf_sz_;
    uint8_t * tx_buf_;
    uint8_t * rx_buf_;
};

#endif //_SPI_DEV_H_
