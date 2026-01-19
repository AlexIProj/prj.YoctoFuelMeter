#include "serial_com.h"
#include "config.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>

int SerialCom_Init(const char *device, int baudrate) {
    int serial_fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd == -1) {
        LOG("Error opening serial port %s\n", device);
        return -1;
    }

    struct termios options;
    tcgetattr(serial_fd, &options);

    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    options.c_lflag &=(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    tcsetattr(serial_fd, TCSANOW, &options);

    tcflush(serial_fd, TCIFLUSH);

    return serial_fd;
}

void SerialCom_SendString(int serial_fd, const char* str) {
    if(serial_fd < 0)
        return;
    int len = strlen(str);
    (void)write(serial_fd, str, len);
    (void)write(serial_fd, "\n", 1);
}

void SerialCom_Close(int serial_fd){
    if(serial_fd >= 0){
        close(serial_fd);
    }
}