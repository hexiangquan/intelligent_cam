#include "uart.h"
#include "log.h"

/*****************************************************************************
 Prototype    : uart_set_attrs
 Description  : set uart attributes
 Input        : int fd                 
                UartBaudrate baudrate  
                UartDataBits dataBits  
                UartStopBits stopBits  
                UartParity parity      
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 uart_set_attrs(int fd, UartBaudrate baudrate, UartDataBits dataBits, UartStopBits stopBits, UartParity parity)
{
	
	const Int32 	csize[] = {CS5, CS6, CS7, CS8,};
	const Int32 	speed[] = {B300, B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800,};
	Int32   		ret; 
	struct termios	attrs;

	/* flush data */
	tcflush(fd, TCIOFLUSH);

	/* get current attrs */
	ret = tcgetattr(fd, &attrs);
	if(ret < 0) {
		ERRSTR("get attrs failed");
		return E_IO;
	}

	/* set speed */
	ret = cfsetispeed(&attrs, speed[baudrate]);  
	ret |= cfsetospeed(&attrs, speed[baudrate]);
	if(ret) {
		ERRSTR("set speed failed");
		return E_IO;
	}

	/* enable recieve and set as local line */
	attrs.c_cflag |= (CLOCAL | CREAD);

	/* set data bits */
	attrs.c_cflag &= ~CSIZE;
	attrs.c_cflag |= csize[dataBits];

	/* set parity */
	if(parity == UART_POFF) {
		attrs.c_cflag &= ~PARENB;			//disable parity
        attrs.c_iflag &= ~INPCK;
	} else {
		attrs.c_cflag |= (PARENB | PARODD);	//enable parity
        attrs.c_iflag |= INPCK;
		if(parity == UART_PEVEN)
			attrs.c_cflag &= ~PARODD;
	}

	/* set stop bits */
	if(stopBits == UART_S1)
		 attrs.c_cflag &= ~CSTOPB;	// 1 stop bit
	else
		attrs.c_cflag |= CSTOPB;	// 2 stop bits

	/* set to raw mode, disable echo, signals */
	attrs.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	/* set no output process, raw mode */
	attrs.c_oflag &= ~OPOST;
	attrs.c_oflag &= ~(ONLCR | OCRNL);

	/* disable CR map  */
	attrs.c_iflag &= ~(ICRNL | INLCR);
	/* disable software flow control */
	attrs.c_iflag &= ~(IXON | IXOFF | IXANY);

	/* flush driver buf */
	tcflush(fd, TCIFLUSH);

	/* update attrs now */
	if(tcsetattr(fd, TCSANOW, &attrs) < 0) {
	   ERRSTR("tcsetattr err");
	   return E_IO;
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : uart_open
 Description  : open uart dev and set attributes
 Input        : const char *name       
                UartBaudrate baudrate  
                UartDataBits dataBits  
                UartStopBits stopBits  
                UartParity parity      
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 uart_open(const char *name, UartBaudrate baudrate, UartDataBits dataBits, UartStopBits stopBits, UartParity parity)
{
	if(!name || baudrate >= UART_B_MAX || dataBits >= UART_D_MAX ||
		stopBits >= UART_S_MAX || parity >= UART_P_MAX)
		return E_INVAL;

	Int32 fd, err;

	/* open uart, tell OS this is not a console */
	fd = open(name,  O_RDWR | O_NOCTTY | O_NDELAY);
	if(fd < 0) {
        ERRSTR("open uart %s err", name);
        return E_INVNAME;
    }

	/* set attrs */
	err = uart_set_attrs(fd, baudrate, dataBits, stopBits, parity);
	if(err) {
		close(fd);
		return err;
	}

	return fd;
}

/*****************************************************************************
 Prototype    : uart_set_timeout
 Description  : set recieve timeout
 Input        : Uint8 minData, minimal counts of data recieved before timeout     
                Uint16 timeout, timeout for reieve, unit: tenths of second  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/29
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 uart_set_timeout(Int32 fd, Uint8 minCnt, Uint8 timeout)
{
	struct termios	attrs;

	if(fd < 0)
		return E_INVAL;

	/* get current attrs */
	if(tcgetattr(fd, &attrs) < 0) {
		ERRSTR("get attrs failed");
		return E_IO;
	}

	/* must recieve minData bytes before read return */
	attrs.c_cc[VTIME] = timeout;
	attrs.c_cc[VMIN] = minCnt;
	
	/* flush data */
	tcflush(fd, TCOFLUSH);

	/* update attrs now */
	if(tcsetattr(fd, TCSANOW, &attrs) < 0) {
	   ERRSTR("tcsetattr err");
	   return E_IO;
	}

	return E_NO;
}


