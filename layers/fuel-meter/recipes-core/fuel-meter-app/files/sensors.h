#ifndef SENSORS_H
#define SENSORS_H

extern volatile int pulses_speed;
extern volatile int pulses_flow;

void* Thread_CountSpeed(void* arg);
void* Thread_CountFlow(void* arg);

void GetAndResetCounts(int* s, int* f);

#endif