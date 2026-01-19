#ifndef SERIAL_COM_H
#define SERIAL_COM_H

int SerialCom_Init(void);
void SerialCom_SendData(int fd, double val);

#endif