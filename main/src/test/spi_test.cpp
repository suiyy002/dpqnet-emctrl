/*! \file spi_test.cpp
    \brief test spi module.
*/

#include "spi_test.h"
#include "spi_dev.h"
//#include "format_convrt.h"
#include "time_cst.h"
#include "math_ext.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <sys/time.h>

using namespace std;

const SpiTest::SquarePara SpiTest::sqr_par_[] = {
    {0.00833333, 2.724},    // 1/2min
    {0.01666667, 2.211},    // 2/2min
    {0.05833333, 1.459},    // 7/2min
    {0.325,      0.906},    // 39/2min
    {0.91666667, 0.725},    // 110/2min
    {13.5,       0.402},    // 1620/2min
    {33.3333333, 2.4  },    // 4000/2min
};

struct HarmFHead {
    int smpnum; //sample data number
    int cycnum; //cycle number
    int dtype;  //data type. 1=double, 2=int
    int grpnum; //groups number
};
/*!
Save the harm data received from spi.

    Input:  pdata -- received data
            size -- size of received data in bytes
            grps -- group number
            id -- save file identifier
*/
void SpiTest::SaveHrm(int *pdata, int size, int grps, int id)
{
    char filename[128];
    sprintf(filename, "save/fft_result%02d.sav", id);
    FILE *pf = fopen(filename, "wb");
    if (pf) {
        HarmFHead fhead;
        fhead.smpnum = kHrmSmpNum;
        fhead.cycnum = 10;
        fhead.dtype = 2;
        fhead.grpnum = grps;
        fwrite(&fhead, sizeof(HarmFHead), 1, pf);
        fwrite(pdata, 1, size, pf);
        fclose(pf);
    } else {
        printf("Failed to open %s for writing!\n", filename);
    }
}

/*!
Save the pst data received from spi.

    Input:  pdata -- received data
            size -- size of received data in bytes
*/
void SpiTest::SavePst(int (*pdata)[3][kPstSmpNum], int size)
{
    if (size>0) {
        for (int i=0; i<4; i++) {
            for (int j=0; j<3; j++) {
                fwrite(pdata[i][j], sizeof(int), kPstSmpNum, fpst_[i][j]);
            }
        }
    } else {
        for (int i=0; i<4; i++) {
            for (int j=0; j<3; j++) {
                fclose(fpst_[i][j]);
            }
        }
    }
}

/*!
Simulative Pst data wave generator

    Input:  chl -- channel.0-3
            phs -- phase. 0-2:A-C
            amp -- amplitude
    Output: pbuf
*/
void SpiTest::PstWaveGen(int chl, int phs, double amp, int *pbuf)
{
    double di; 
    double pow_freq = 50;
    double smpfrq = 50*kPstSmpNum/10.0;    //sample frequency. unit:Hz
    double m = sqr_par_[sqr_type_].amp/200;
    double f = sqr_par_[sqr_type_].freq;
    
    int x = pst_x_[chl][phs];
    for (int i=0; i<kPstSmpNum; i++) {
        di = amp*(1+m*SquareWave(f*x/smpfrq))*cos(2*kM_PI*pow_freq*x/smpfrq - 2*kM_PI*phs/3);
        *pbuf = di + 0.5;
        pbuf++;
        x++;
    }
    pst_x_[chl][phs] = x;
}

/*!
Simulative harmonic data wave generator

    Input:  chl -- channel.0-3
            phs -- phase. 0-2:A-C
            amp -- amplitude
    Output: pbuf
*/
void SpiTest::HarmWaveGen(int chl, int phs, double amp, int *pbuf)
{
    float di; 
    float pow_freq = 50;
    float smpfrq = 50*kHrmSmpNum/10.0;    //Sample frequency. unit:Hz
    
    int k = 3+chl*8+phs*2;
    for (int x=0; x<kHrmSmpNum; x++) {
        di = amp*cos(2*kM_PI*pow_freq*x/smpfrq - 2*kM_PI*phs/3);
      	di += amp/50*cos(2*kM_PI*pow_freq*k*x/smpfrq + k*kM_PI/60.0);
        *pbuf = di + 0.5;
        pbuf++;
    }
}

/*!
    Input:  dev -- device name
            speed -- transfer speed.unit:Hz
            bpw -- bit per word
            mode -- spi work mode
            pst_type -- pst simulate data type
            sim_type -- simulate data type. 0=harmonic only; 1=pst only; 2=all
*/
void SpiTest::SimTest(const char *dev, uint32_t speed, uint8_t bpw, uint8_t mode, 
                        uint8_t pst_type, uint8_t sim_type)
{
    SpiDev *spi = new SpiDev(dev, 1);
    if (spi->SetSpeed(speed)<0) abort();
    if (spi->SetWordBits(bpw)<0) abort();
    if (spi->SetMode(mode)<0) abort();
    SpiApi spi_api;
    TestIni((CycNumPer2Min)pst_type);

    char hrmfile[128];
    int num;
    StopWatch (0, 1);
    int i, j, k, n;
    if (sim_type==0) n = 10; 
    else n = 3300;  //3300 loop = 11min 
    for (k = 0; k < n; k++) {  //10cycle(0.2s) per loop, last for n loop
        if (!sim_type||sim_type==2) {   //harm or all
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 3; j++) {
                    HarmWaveGen(i, j, 10000+k*100%10000, fft_tbuf_[i][j]);
                }
            }
        }
        if (sim_type) { //pst or all
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 3; j++) {
                    PstWaveGen(i, j, 10000, pst_tbuf_[i][j]);
                }
            }
        }
        if (!sim_type||sim_type==2) {   //harm or all
            num = spi_api.Transfer(spi, 3, fft_tbuf_, fft_rbuf_, kHrmSmpNum*3*4);
            if (num<=0) continue;
        }
        if (sim_type) {     //pst or all
            num = spi_api.Transfer(spi, 5, pst_tbuf_, pst_rbuf_, kPstSmpNum*3*4);
            if (num<=0) continue;
        }
        
        if (k>=0&&k<10) {    //only save 10 files
            SaveHrm(&fft_rbuf_[0][0][0], sizeof(fft_rbuf_), i*j, k);
        }
        SavePst(pst_rbuf_, sizeof(pst_rbuf_));
    }
    StopWatch (0, 0, "testspi");
    SavePst(pst_rbuf_, 0);

    delete spi;
    //printf("pi=%5.9f\n", kM_PI);
}

/*!
    Input:  type -- pst source data type.
*/
void SpiTest::TestIni(CycNumPer2Min type)
{
    sqr_type_ = type;
    memset(fpst_, 0, sizeof(fpst_));
    memset(pst_x_, 0, sizeof(pst_x_));
    char pst_file[128];
    for (int i=0; i<4; i++) {
        for (int j=0; j<3; j++) {
            sprintf(pst_file, "save/pst_result_%d%c.sav", i, 'A'+j);
            fpst_[i][j] = fopen(pst_file, "wb");
        }
    }
    
}
