/******************************************************************************
 * File: uart-loopback.c
 * Description: Test UART loopback from userspace.
 *
 *****************************************************************************/

// Include files

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <stdarg.h>


// Constants
#define MAX_FRAME_SIZE          5

#define UARTIO_OK              0
#define ERR_OPEN               -1
#define ERR_CLOSE              -2
#define ERR_READ               -3
#define ERR_WRITE              -4
#define ERR_DATA_MISMATCH      -5

int error(char *sFormat, ...) {
  va_list args;

  // not in quiet mode?
  // output error
  printf("ERROR: ");

  va_start(args, sFormat);
  vprintf(sFormat, args);
  va_end(args);

  printf("\n");
  return -1;
}

// io_open
// Call to open device

int io_open(char *sDevice,int nOpen) {
	// can open it?
	int hDevice=open(sDevice,nOpen);

	if(hDevice<0) {
		error("io failed to open %s",sDevice);
		return -1;
	}
	return hDevice;
}

// io_close
// Call to close device

int io_close(int hDevice) {
	// can close it?
	if(close(hDevice)<0) {
		error("io failed to close device");
		return -1;
	}
	return 0;
}

void usage() {
	printf("\nUsage:\n\n");
	printf("\nExample:\n");
	printf("uart-loopback /dev/ttyO2\n");
	printf("\n");
}

int main(int argc, char **argv) {
  unsigned char frame[MAX_FRAME_SIZE] = {0x01, 0x02, 0x03, 0x04, 0x05};
  unsigned char cResponse[MAX_FRAME_SIZE];
  char sDevice[128];
  unsigned char bytes_read=0;
  unsigned char bytes_write=0;
  
  //check params
  if(argc!=2) {
    usage();
    exit(1);
  }

  // Copy port argument
  strcpy(sDevice,argv[1]);
  
  // Open uart port
  int hDevice=io_open(sDevice,O_RDWR | O_NOCTTY);
  if(hDevice<0) {
    return ERR_OPEN;
  }
  
  // Set attributes
  struct termios options;
  tcgetattr(hDevice, &options);
  options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                    | INLCR | IGNCR | ICRNL | IXON);
  options.c_oflag &= ~OPOST;
  options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  options.c_cflag &= ~(CSIZE | PARENB);
  options.c_cflag |= B9600 | CS8 | CREAD | CLOCAL;
  options.c_cc[VTIME] = 1;
  options.c_cc[VMIN] = 0;
  //options.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
  //options.c_iflag = IGNPAR | ICRNL;
  tcflush(hDevice, TCIOFLUSH);
  tcsetattr(hDevice, TCSANOW, &options);
  
  // write
  if ((bytes_write=write(hDevice,frame,sizeof(frame)))!=(sizeof(frame))) {
    io_close(hDevice);
    error("io failed to write at %s. Bytes Written:%d",sDevice,bytes_write);
    return ERR_WRITE;
  }
  printf("Loopback Test (%s): Bytes Written: %d, Write Success\n",sDevice,bytes_write);
  
  // read response
  bytes_read=read(hDevice, cResponse, MAX_FRAME_SIZE);
  if (bytes_read<MAX_FRAME_SIZE) {
    io_close(hDevice);
    error("io failed to read from %s, bytes read: %d",sDevice, bytes_read);
    return ERR_READ;
  }
  printf("Loopback Test (%s): Bytes Read: %d, Read Success\n",sDevice,bytes_read);
  
  // Compare response
  printf("Loopback Test: Bytes Received: ");
  for (int i=0; i<MAX_FRAME_SIZE; i++) {
    printf("%d ", cResponse[i]);
    if(frame[i]!=cResponse[i]) {
      error("Data Mismatch");
      io_close(hDevice);
      return ERR_DATA_MISMATCH;
    }
  }
  printf("\n");
  
  // close device
  io_close(hDevice);
  
	return 0;
}
