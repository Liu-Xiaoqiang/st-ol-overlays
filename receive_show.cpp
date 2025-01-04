#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "general.h"


BOOL waitForSync = TRUE;

int parse(char * newBuffer, int newBufferSize, char * port)
{
    static unsigned long longCounter = 0;
    static unsigned long dataIndex = 0;
    char c;
    int retry;
    
    for(int index = 0; index < newBufferSize; index++)
    {
        c = *(newBuffer + index);
        if(waitForSync && c != testData[0]) continue;
        waitForSync = FALSE;
        if(c != testData[dataIndex])
        {
            // out of sync
            printf("** Out of sync **\n");
            fflush(stdout);
            // maybe it is a Sync
            if(c == testData[0]) 
            {
                dataIndex = 1;
            }
            else 
            {
                waitForSync = TRUE;
                dataIndex = 0;
            }
			return 1;
        }
        else
        {
            if(++dataIndex == testDataSize)
            {
                // All Done
                dataIndex = 0;
		printf("%08lu\n", ++longCounter);
                fflush(stdout);
            }
        }
    }
    return 1;
}

#define BUFFER_SIZE     100

int main(int argc, char * argv[])
{
	int fd, res, ret=1;
	if(argc != 2)
	{
		printf("Useg: \n");
		printf("      ./receive /dev/ttyX\n");
		return 1;
	}
    fd = openPort(argv[1]);
  	if(fd < 0)
	{
	    fprintf(stderr, "Error opening comport %s.\n", argv[1]);
	    return 1;
	}

    char buffer[BUFFER_SIZE];
    printf("Waiting for \'"TEST_STRING"\'\n");
    do
    {   
        /* loop for input */
        res = read(fd, buffer, BUFFER_SIZE);
        if(res > 0)
        {
            ret = parse(buffer, res, argv[1]);
        }
        else
        {
            printf("Read error %d", errno);
        }
    } while(ret);
	
    return 0;
}
