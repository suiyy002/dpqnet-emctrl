#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "serial_device.h"
//#include "../thread/pthread_mng.h"
//#include "../GUI/view.h"
//#include "../pqm_func/pqmfunc.h"

#define MODEMDEVICE "/dev/ttyS" 

SerialDevice::SerialDevice()
{
    fd_ = NULL;
}
SerialDevice::~SerialDevice()
{
    if (fd_) Close();
}

/*!
Open serial device and initialize

    Input:  sn -- number of serial port. 0=COM0,1=COM1...
            baud -- Baudrate
    Return: file description. <0=error
*/
int SerialDevice::Open(int sn, unsigned int baud)
{
	struct termios newtio;
	char str[20];
	dev_num_ = sn;
	baud_ = baud;
	sprintf(str, "%s%i", MODEMDEVICE, dev_num_);
	fd_ = open(str, O_RDWR | O_NOCTTY | O_NDELAY );
	if (fd_ <0) { perror(str); return fd_; }
	fcntl(fd_, F_SETFL, O_NONBLOCK); //设置读写操作为非阻塞
	tcgetattr(fd_,&oldtio_); //save current port settings
	//tcgetattr(fd_,&newtio); //get current port settings
	memset(&newtio, 0, sizeof(newtio));// clear struct for new port settings

	//Set Control Options c_cflag
	newtio.c_cflag |= CLOCAL|CREAD;  //使能接收，设置本地状态... 此两位必须时刻保持使能
	if(cfsetspeed(&newtio,baud_)<0){ //设置输入输出波特率
		printf("This baudrate is not supported!!\n"); //for debug
		cfsetspeed(&newtio,115200);
	}

	//Set Output Options c_oflag
	//printf("baudrate=%d\n", baud_); //for debug
	newtio.c_cflag &= ~PARENB;  //无奇偶校验
	newtio.c_cflag &= ~CSTOPB;  //一位停止位
	newtio.c_cflag &= ~CSIZE;//设置字符大小为8位
	newtio.c_cflag |= CS8;
	newtio.c_cflag &= ~CRTSCTS;//禁止硬件流控
	newtio.c_oflag &= ~OPOST;//选择原始数据输出
		
	//Set Local Options c_lflag
    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  //选择原始数据输入
	//newtio.c_lflag = 0; //~(ICANON|ECHO|ECHOE|ISIG|OPOST); //选择原始输入法

	//Set Input Options c_iflag
	//newtio.c_iflag &= ~(INPCK|ISTRIP); //屏蔽输入奇偶参数
	//newtio.c_iflag &= ~(IXON|IXOFF|IXANY); //禁止软件流控
	newtio.c_iflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */         
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 1 chars received */
                                                                                
    tcflush(fd_, TCIOFLUSH); //清空输入输出缓存
	tcsetattr(fd_,TCSANOW,&newtio);//set current port
	return fd_;
}

//---------------------------------------------------------------------------
//关闭串口设备
void SerialDevice::Close()
{
    if (fd_<0) return;
    tcsetattr(fd_, TCSANOW, &oldtio_); //resume old set
    close(fd_);
}

/*!
Read data from socket device
    
    Input:  size -- maximum bytes read from socket 
    Output: buf --
    Return: the number of bytes actually read. <0 = error
*/
int SerialDevice::Read(void *buf, size_t size)
{
	int cnt = read(fd_, buf, size);
	return cnt;
}

/*!
Read data from serial device
    
    Input:  size -- maximum bytes read from socket 
    Output: buf --
    Return: the number of bytes actually read. <0 = error
*/
int SerialDevice::Write(void *buf, size_t size)
{
	int i, j, k;
	
	char *cpi = (char*)buf;
	i = size/1024;
	j = size%1024;
	for(k=0;k<i;k++){
		write(fd_, cpi, 1024);
		cpi += 1024;
		tcdrain(fd_); //等待，直到输出队列的数据全部发送
	}
	write(fd_, cpi, j);
	return 0;
}

/*!
Reopen serial port device

    Return: serial port device file description
*/
int SerialDevice::Restart()
{
    Close();
    Open(dev_num_, baud_);
    return fd_;
}

