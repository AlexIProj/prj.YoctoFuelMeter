#include "sensors.h"
#include <gpiod.h>

// ------------------ GLOBALS ------------------
volatile int pulses_speed = 0;
volatile int pulses_flow = 0;

void* Thread_CountSpeed(void* arg) {
    struct gpiod_line_request* request = (struct gpiod_line_request*)arg;
    struct gpiod_edge_event_buffer* buffer = gpiod_edge_event_buffer_new(1);

    while(1){
        if (gpiod_line_request_wait_edge_events(request, -1) > 0) {
            gpiod_line_request_read_edge_events(request, buffer, 1);
            pulses_speed++;
        }
    }
    return NULL;
}

void* Thread_CountFlow(void* arg) {
    struct gpiod_line_request* request = (struct gpiod_line_request*)arg;
    struct gpiod_edge_event_buffer* buffer = gpiod_edge_event_buffer_new(1);

    while(1) {
        if (gpiod_line_request_wait_edge_events(request, -1) > 0) {
            gpiod_line_request_read_edge_events(request, buffer, 1);
            pulses_flow++;
        }
    }

    return NULL;
}

void GetAndResetCounts(int* s, int* f) {
    *s = pulses_speed;
    pulses_speed = 0;
    *f = pulses_flow;
    pulses_flow = 0;
}