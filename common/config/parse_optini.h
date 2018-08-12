#ifndef _PARSE_OPTINI_H_
#define _PARSE_OPTINI_H_
//---------------------------------------------------------------------------
#include "parse_option.h"
#include <cstdio>
using namespace std;

#define MAIN_PROG "dpqnet_mn"
#define TEST_SPI "testspi"

const char * ParseOption::short_opts_[kCmdTypeEnd];
const option * ParseOption::long_opts_[kCmdTypeEnd];
const char * ParseOption::help_info_[kCmdTypeEnd];

//main command
static const char * main_sopts = "";
static const option main_lopts[] = {
    { "help",    0, 0, 'h' },
    { "version", 0, 0, 'V' },
    { "debug", 1, 0, 'd' },
    { NULL, 0, 0, 0 },
};
static const char * main_help =
    "Usage: "MAIN_PROG" [--version] [--help] [--debug <num>]\n"
    "                 <command> [<args>]\n\n"
    "The "MAIN_PROG" commands are:\n"
    "   testspi     Test peripheral spi\n"
    "\nSee '"MAIN_PROG" help <command>' for help information on a specific command.\n";

//testspi command
static const char *spi_sopts = "D:s:b:HOCp:m:";
static const option spi_lopts[] = {
    { "device",  1, 0, 'D' },
    { "speed",   1, 0, 's' },
    { "bpw",     1, 0, 'b' },
    { "cpha",    0, 0, 'H' },
    { "cpol",    0, 0, 'O' },
    { "cs-high", 0, 0, 'C' },
    { "pst",     1, 0, 'p' },
    { "sim",     1, 0, 'm' },
    { NULL, 0, 0, 0 },
};
static const char * spi_help =
    "  -D --device <path/name> device to use (default /dev/spidev2.0)\n"
    "  -s --speed <xxx> max speed (kHz)\n"
    "  -b --bpw <xxx>   bits per word \n"
    "  -H --cpha        clock phase\n"
    "  -O --cpol        clock polarity\n"
    "  -C --cs-high     chip select active high\n"
    "  -p --pst <x>     pst type. 0(1/2min)-6(4000/2min)\n"
    "  -m --sim <xxx>   simulate type. harm | pst | all \n";

#endif //_PARSE_OPTINI_H__