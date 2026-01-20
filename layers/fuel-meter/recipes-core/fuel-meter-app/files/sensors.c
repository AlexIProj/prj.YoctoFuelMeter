#include "sensors.h"
#include <gpiod.h>
#include <stdatomic.h>

// ------------------ GLOBALS ------------------
static atomic_int pulses_speed = 0;
static atomic_int pulses_flow = 0;

void* Thread_CountSpeed(void* arg) {
    struct gpiod_line_request* request = (struct gpiod_line_request*)arg;
    struct gpiod_edge_event_buffer* buffer = gpiod_edge_event_buffer_new(1);

    while(1){
        if (gpiod_line_request_wait_edge_events(request, -1) > 0) {
            gpiod_line_request_read_edge_events(request, buffer, 1);
            atomic_fetch_add(&pulses_speed, 1);
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
            atomic_fetch_add(&pulses_flow, 1);
        }
    }

    return NULL;
}

void GetAndResetCounts(int* s, int* f) {
    *s = atomic_exchange(&pulses_speed, 0);
    *f = atomic_exchange(&pulses_flow, 0);
}