#ifndef SENSORS_H
#define SENSORS_H

void* Thread_CountSpeed(void* arg);
void* Thread_CountFlow(void* arg);

void GetAndResetCounts(int* s, int* f);

#endif