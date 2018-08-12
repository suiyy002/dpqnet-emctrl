#include <stdio.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <errno.h>

#include "socket_device.h"

SocketDevice::SocketDevice(int fd)
{
	fd_ = fd;
	fcntl(fd_, F_SETFL, O_NONBLOCK);     //Set read & write to no block
}

SocketDevice::~SocketDevice()
{
    //shutdown(fd_, 2);
    close(fd_);
}

/*!
Read data from socket device
    
    Input:  size -- maximum bytes read from socket 
    Output: buf --
    Return: the number of bytes actually read. <0 = error
*/
int SocketDevice::Read(void *buf, size_t size)
{
	ssize_t cnt;    //ssize_t -- signed size_t
	
	unsigned char *ptr = (unsigned char *)buf;
	
	do {
	    cnt = read(fd_, ptr, size);
	    if(cnt<0){
	    	if(errno == EINTR) continue;
	    }
    } while(0);

	if ( cnt<0 ) {
		if (errno == ECONNRESET) {  //connection reset by client
            perror("connection reset by client!\n");
       	} else perror("receive_buf error");
    } else if ( cnt==0 ) {  // connection closed by client 
		printf("connection closed by client!\n");
	}
	return cnt;
}

/*!
Write data to socket device
    
    Input:  size -- maximum bytes write to socket 
    Output: buf --
    Return: the number of bytes actually write??. <0 = error
*/
int SocketDevice::Write(void *buf, size_t size)
{
	ssize_t nleft, nwritten;
	char *ptr;
	ptr = (char *)buf;
	nleft = size;

	while (nleft>0) {
		if ((nwritten = write(fd_, ptr, nleft))<=0) {
			if(errno == EINTR) nwritten = 0;
			else return -1;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return size;
}
