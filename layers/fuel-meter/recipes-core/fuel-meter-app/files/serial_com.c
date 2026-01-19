#include "serial_com.h"
#include "config.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

int serial_fd = -1;

int SerialCom_Init(const char *device, int baudrate) {
    serial_fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd == -1) {
        LOG("Error opening serial port %s\n", device);
        return -1;
    }

    struct termios options;
    tcgetattr(serial_fd, &options);


}

void SerialCom_SendData(int fd, double val) {
    if (fd < 0)
        return;

    char buff[16];
    int len = sprintf(buff, "%04.1f\n", val);
    write(fd, buff, len);
}