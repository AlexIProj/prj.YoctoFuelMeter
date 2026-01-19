#ifndef SERIAL_COM_H
#define SERIAL_COM_H

int SerialCom_Init(const char* device, int baudrate);
void SerialCom_SendString(int serial_fd, const char* str);
void SerialCom_Close(int serial_fd);

#endif