#ifndef _PARSE_OPTION_H_
#define _PARSE_OPTION_H_
//---------------------------------------------------------------------------
#include <stdint.h>
#include <getopt.h>

enum CmdType { kMainProg, kTestSpi, 
    kCmdTypeEnd };

class ParseOption
{
public:
    ParseOption();
    ~ParseOption(){};
    
    int Parse(int argc, char *argv[]);

    int cmd() { return cmd_; };
    int debug() { return debug_; };
    uint8_t pst_type() { return pst_type_; };
    uint8_t sim_type() { return sim_type_; };
    const char *spi_device() { return spi_par_.device; };
    uint32_t spi_speed() { return spi_par_.speed; };
    uint8_t spi_bpw() { return spi_par_.bpw; };
    uint8_t spi_mode() { return spi_par_.mode; };

protected:
    static const char *short_opts_[kCmdTypeEnd];
    static const option *long_opts_[kCmdTypeEnd];
    static const char * help_info_[kCmdTypeEnd];
    
private:
    int PrintHelp(const char *cmd=0);
    void InitParam();
    int HandleSpi(int opt);
    int HandleMain(int opt);
        
    struct SpiParam {
        const char *device;
        uint32_t speed; //unit:Hz
        uint8_t bpw;    //bits per word
        uint8_t mode;   //work mode. SPI_CPHA | SPI_CPOL | SPI_CS_HIGH | SPI_LSB_FIRST | SPI_3WIRE | SPI_LOOP | SPI_NO_CS | SPI_READY
    } spi_par_;

    int cmd_;
    int debug_;
    uint8_t pst_type_;  //pst simulate data type
    uint8_t sim_type_;  //simulate type. 0=harmonic only, 1=all
};


#endif //_PARSE_OPTION_H_