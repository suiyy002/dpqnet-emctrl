/*! \file parse_option.cpp
    \brief Parse command line option.
*/

#include "parse_optini.h"
#include "config.h"
#include "spi_dev.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;


ParseOption::ParseOption()
{
    InitParam();

    spi_par_.device = "/dev/spidev2.0";
    spi_par_.speed = 6000000;
    spi_par_.bpw = 32;
    spi_par_.mode = SPI_CPHA;
    debug_ = 0;
    pst_type_ = 0;
    sim_type_ = 0;  //only harmonic
}

/*!
    Return: 0,1=normal, -1=exception, >1=continue
*/
int ParseOption::Parse(int argc, char *argv[])
{
    if (strstr(argv[0], MAIN_PROG)) {
        if ( strcspn(argv[1], "-")==0 ) {   //begin with a hyphen delimiter '-'
            cmd_ = kMainProg;
        } else {
            argc--; argv++;
            if (argc>0) return Parse(argc, argv);
            return -1;
        }
    } else if (!strcmp(argv[0], "help")) {
        if (argc<2) return PrintHelp();
        return PrintHelp(argv[1]);
    } else if (!strcmp(argv[0], TEST_SPI)) {
        cmd_ = kTestSpi;
    } else {
        return -1;
    }
    
    const char *sopts = short_opts_[cmd_];
    const option *lopts = long_opts_[cmd_];
    int ret = 0;
    while (1) {
        int c = getopt_long(argc, argv, sopts, lopts, NULL);

        if (c == -1) break;

        switch (cmd_) {
            case kMainProg:
                ret = HandleMain(c);
                break;
            case kTestSpi:
                ret = HandleSpi(c);
                break;
        }
        if (ret != 0) break;
    }
    return ret;
}

int ParseOption::HandleSpi(int opt)
{
    switch (opt) {
        case 'D':
            spi_par_.device = optarg;
            break;
        case 's':
            spi_par_.speed = atoi(optarg)*1000;
            break;
        case 'b':
            spi_par_.bpw = atoi(optarg);
            break;
        case 'H':
            spi_par_.mode |= SPI_CPHA;
            break;
        case 'O':
            spi_par_.mode |= SPI_CPOL;
            break;
        case 'C':
            spi_par_.mode |= SPI_CS_HIGH;
            break;
        case 'p':
            pst_type_ = atoi(optarg);
            break;
        case 'm':
            if (!strcmp("harm", optarg)) {
                sim_type_ = 0;
            } else if (!strcmp("pst", optarg)) {
                sim_type_ = 1;
            } else if (!strcmp("all", optarg)) {
                sim_type_ = 2;
            }
            break;
        default:
            return PrintHelp(TEST_SPI);
    }
    return 0;
}

int ParseOption::HandleMain(int opt)
{
    switch (opt) {
        case 'V':
            printf(MAIN_PROG" version %d.%d.%d\n", _VERSION_MAJOR, _VERSION_MINOR, _VERSION_PATCH);
            return 1;
        case 'd':
            debug_ = atoi(optarg);
            return 2;
            break;
        default:
            return PrintHelp();
    }
}

int ParseOption::PrintHelp(const char *cmd)
{
    if (cmd==NULL) {
        puts(help_info_[kMainProg]);
        return -1;
    }
    if (!strcmp(cmd, TEST_SPI)) {
        puts(help_info_[kTestSpi]);
    } else {
        puts("unknown command!\n");
        return -1;
    }
    return 0;
}
 
void ParseOption::InitParam()
{
    ParseOption::short_opts_[kMainProg] = main_sopts;
    ParseOption::long_opts_[kMainProg] = main_lopts;
    ParseOption::help_info_[kMainProg] = main_help;

    ParseOption::short_opts_[kTestSpi] = spi_sopts;
    ParseOption::long_opts_[kTestSpi] = spi_lopts;
    ParseOption::help_info_[kTestSpi] = spi_help;
}


