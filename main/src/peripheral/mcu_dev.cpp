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
McuDev::McuDev()
{
}

McuDev::~McuDev()
{
}

/*!
Read Power off time

    Output: time
    Return: true=succes, false=failure
*/
bool McuDev::Read0Time(time_t * time)
{
    ModbusRequest request;
    request.fcode[0] = 4;
    request.addrs = kPower0Time;
    request.qty = 2;
    
    ret = uart_obj[kPortMCU].comm_handle->TxCmd(&request);  //读取上次关机时间
    if (ret) {
        uart_obj[kPortMCU].app_prtcl->registers(time, kPower0Time, sizeof(time_t));
        return true;
    } else {
        return false;
    }
}

