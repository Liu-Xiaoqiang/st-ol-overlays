#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "general.h"
        

void sendData(int fd)
{
    int written;
    int mustWrite;
    int index;
    printf("Sending %d record of \'" TEST_STRING "\'\n",  NUMBER_OF_MESSAGES);
    for(index = 0; index < NUMBER_OF_MESSAGES; index++)
    {
        usleep(2000);
        mustWrite = testDataSize;
        written = 0;
        do
        {
        	usleep(100);
            int n = write(fd, testData + written, mustWrite);
            if(n > 0)
            {
                written += n;
                mustWrite -= n;
            }
        }while(mustWrite);
    }
    sleep(2); //required to make flush work, for some reason
    tcflush(fd,TCIOFLUSH);
}


int main(int argc,char* argv[])
{
	if(argc != 2)
	{
		printf("Useg: \n");
		printf("      ./send /dev/ttyX\n");
		return 1;
	}
	int fd = openPort(argv[1]);			//Sets the port name
	if(fd < 0)
	{
	    fprintf(stderr, "Error opening comport %s.\n", argv[1]);
	    return 1;
	}
	usleep(5000);
    printf("Starting test for port %s\n", argv[1]);
	sendData(fd);
    close(fd);
    return 0;
}
