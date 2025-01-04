#ifndef __INCLUDE_GENERAL__
#define __INCLUDE_GENERAL__

#define _POSIX_SOURCE 1 /* POSIX compliant source */

typedef int BOOL;

#define FALSE                                   (0)
#define TRUE                                    (!FALSE)
#define ERROR                                   (-1)

#define CS10600T070P                            (1)
#define CS10600F070P                            (2)
#define LOCAL                                   (3)

#ifndef IPC_TYPE
    #define IPC_TYPE                            CS10600F070P
#endif


#if IPC_TYPE == CS10600F070P
    #define SERIAL_DEVICE_NAME                  "/dev/ttymxc2"
#elif IPC_TYPE == CS10600T070P
    #define SERIAL_DEVICE_NAME                  "/dev/ttyO1"
#elif IPC_TYPE == LOCAL
    #define SERIAL_DEVICE_NAME                  "/dev/ttyUSB0"
#endif




#define BAUDRATE                                B115200
#define BAUDRATE_ERROR                          B57600
#define BAURATE_NAME(x)                         (x == BAUDRATE ? "115200" : "57600")

#define TEST_STRING                             "The quick brown fox jumps over the lazy dog"
#define NUMBER_OF_MESSAGES                      (1000)


char testData[]=TEST_STRING;
int testDataSize = sizeof(testData) / sizeof(testData[0]) - 1;

#define SYNC_CHAR                               testData[0]
#define END_CHAR                                testData[testDataSize - 1]




void setBaud(int fd, speed_t baud)
{
    //Configure port for 8N1 transmission
    struct termios options;
    tcgetattr(fd, &options);					//Gets the current options for the port
    cfsetispeed(&options, baud);			    //Sets the Input Baud Rate
    cfsetospeed(&options, baud);			    //Sets the Output Baud Rate
    options.c_cflag |= CLOCAL;		            //? all these set options for 8N1 serial operations    options.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
    options.c_cflag |= CS8;						//?
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    options.c_cc[VTIME] = 0;                    /* inter-character timer unused */
    options.c_cc[VMIN]  = 1;                    /* blocking read until 1 chars received */
    tcsetattr(fd, TCSANOW, &options);			//Set the new options for the port "NOW"
}



int openPort(char * port)
{
	int fd = open(port, O_RDWR | O_NOCTTY |O_NDELAY );
    if (fd <0)
    {
        return ERROR;
    }
    fcntl(fd, F_SETFL, 0);

    setBaud(fd, BAUDRATE);
    return fd;
}



#endif //#ifndef __INCLUDE_GENERAL__
