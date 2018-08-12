#ifndef _SPI_TEST_H_
#define _SPI_TEST_H_
//---------------------------------------------------------------------------
#include <stdint.h>
#include <cstdio>
using namespace std;

enum CycNumPer2Min { kCycle1, kCycle2, kCycle7,
    kkCycle39, kkCycle110, kCycle1620, kCycle4000
};

class SpiTest
{
public:
    SpiTest() {};
    ~SpiTest() {};

    void SimTest(const char *dev, uint32_t speed, uint8_t bpw, uint8_t mode, uint8_t pst_type, uint8_t sim_type);

protected:
    struct SquarePara {
        float freq; //square wave frequency. unit:Hz
        float amp;  //square wave amplitude. unit:%
    };

    static const SquarePara sqr_par_[kCycle4000+1];

private:
    void TestIni(CycNumPer2Min type);
    void HarmWaveGen(int chl, int phs, double amp, int *pbuf);
    void PstWaveGen(int chl, int phs, double amp, int *pbuf);
    void SaveHrm(int *pdata, int size, int grps, int id);
    void SavePst(int (*pdata)[3][kPstSmpNum], int size);

    
    int fft_tbuf_[4][3][kHrmSmpNum]; //fft data transmit buffer. [0-3]:channel; [0-2]:PhaseA-C;
    int fft_rbuf_[4][3][kHrmSmpNum]; //fft data receive buffer.  [0-3]:channel; [0-2]:PhaseA-C;

    int pst_tbuf_[4][3][kPstSmpNum]; //fluctuation data transmit buffer. [0-3]:channel; [0-2]:PhaseA-C;
    int pst_rbuf_[4][3][kPstSmpNum]; //fluctuation data receive buffer.  [0-3]:channel; [0-2]:PhaseA-C;

    CycNumPer2Min sqr_type_;      //square wave type
    FILE *fpst_[4][3];  //pst save file stream.[0-3]:4channel, [0-2]:A-C phase
    int pst_x_[4][3];   //sampling point count.[0-3]:4channel, [0-2]:A-C phase
};

#endif //_SPI_TEST_H_
