#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include "config.h"
#include "calcs.h"
#include "gpio_driver.h"
#include "serial_com.h"
#include "sensors.h"

// ------------------ GLOBALS ------------------
static volatile int running = 1;
static int serial_fd = -1;
static struct gpiod_line_request* speed_line_req = NULL;
static struct gpiod_line_request* flow_line_req = NULL;
static struct gpiod_line_request* button_line_req = NULL;

// ------------------ MOVING AVERAGE FILTER ------------------
#define FILTER_SIZE 5
static double filter_buffer[FILTER_SIZE] = {0};
static int filter_index = 0;
static int filter_count = 0;

static double ApplyMovingAverage(double new_value) {
    filter_buffer[filter_index] = new_value;
    filter_index = (filter_index + 1) % FILTER_SIZE;

    if (filter_count < FILTER_SIZE)
        filter_count++;

    double sum = 0.0;
    for (int i = 0; i < filter_count; i++) {
        sum += filter_buffer[i];
    }
    return sum / filter_count;
}

// ------------------ SIGNAL HANDLER ------------------
void signal_handler(int sig) {
    (void)sig;
    printf("\n[Main] Received termination signal. Shutting down...\n");
    running = 0;
}

// ------------------ INITIALIZATION ------------------
int InitializeHardware(void) {
    printf("[Init] Configuring GPIO lines...\n");

    speed_line_req = ConfigLineInput(GPIO_CHIP_PATH, PIN_SPEED_SEN, "speed_sensor", 1);
    if (!speed_line_req) {
        fprintf(stderr, "[Init] ERROR: Failed to configure speed sensor GPIO (Pin %d)\n", PIN_SPEED_SEN);
        return -1;
    }
    printf("[Init] Speed sensor configured on GPIO %d\n", PIN_SPEED_SEN);

    flow_line_req = ConfigLineInput(GPIO_CHIP_PATH, PIN_FLOW_SEN, "flow_sensor", 1);
    if (!flow_line_req) {
        fprintf(stderr, "[Init] ERROR: Failed to configure flow sensor GPIO (Pin %d)\n", PIN_FLOW_SEN);
        return -1;
    }
    printf("[Init] Flow sensor configured on GPIO %d\n", PIN_FLOW_SEN);

    button_line_req = ConfigLineInput(GPIO_CHIP_PATH, PIN_SWITCH_BTN, "toggle_button", 0);
    if (!button_line_req) {
        fprintf(stderr, "[Init] ERROR: Failed to configure toggle button GPIO (Pin %d)\n", PIN_SWITCH_BTN);
        return -1;
    }
    printf("[Init] Toggle button configured on GPIO %d\n", PIN_SWITCH_BTN);

    printf("[Init] Opening serial port %s at %d baud...\n", SERIAL_PORT, BAUD_RATE);
    serial_fd = SerialCom_Init(SERIAL_PORT, BAUD_RATE);
    if (serial_fd < 0) {
        fprintf(stderr, "[Init] ERROR: Failed to initialize serial communication\n");
        return -1;
    }
    printf("[Init] Serial communication initialized (fd=%d)\n", serial_fd);

    return 0;
}

// ------------------ CLEANUP ------------------
void CleanupHardware(void) {
    printf("[Cleanup] Releasing resources...\n");
    
    if (serial_fd >= 0) {
        SerialCom_Close(serial_fd);
        printf("[Cleanup] Serial port closed\n");
    }
    
    printf("[Cleanup] Done\n");
}

// ------------------ MAIN ------------------
int main(void) {
    pthread_t thread_speed, thread_flow;
    int pulse_speed = 0;
    int pulse_flow = 0;
    int is_metric = 1;
    int btn_prev_state = 0;
    double consumption = 0.0;
    char uart_buffer[32];

    printf("============================================\n");
    printf("    Raspberry Pi Fuel Meter Application\n");
    printf("============================================\n");
    printf("Sensor constants:\n");
    printf("  - Flow sensor: %.1f pulses/cm³\n", K_FLOW_SEN);
    printf("  - Speed sensor: %.1f pulses/mile\n", K_SPEED_SEN);
    printf("  - Metric constant: %.6f\n", GetMetricConstant());
    printf("  - Imperial constant: %.6f\n", GetImperialConstant());
    printf("--------------------------------------------\n");

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (InitializeHardware() != 0) {
        fprintf(stderr, "[Main] Hardware initialization failed. Exiting.\n");
        CleanupHardware();
        return EXIT_FAILURE;
    }

    printf("[Main] Starting pulse counting threads...\n");
    
    if (pthread_create(&thread_speed, NULL, Thread_CountSpeed, speed_line_req) != 0) {
        fprintf(stderr, "[Main] ERROR: Failed to create speed counting thread\n");
        CleanupHardware();
        return EXIT_FAILURE;
    }

    if (pthread_create(&thread_flow, NULL, Thread_CountFlow, flow_line_req) != 0) {
        fprintf(stderr, "[Main] ERROR: Failed to create flow counting thread\n");
        CleanupHardware();
        return EXIT_FAILURE;
    }

    printf("[Main] Application running. Press Ctrl+C to exit.\n");
    printf("[Main] Default mode: METRIC (L/100Km)\n");
    printf("--------------------------------------------\n");

    while (running) {
        int btn_state = GPIOD_GetLineValue(button_line_req, PIN_SWITCH_BTN);

        if (btn_state == 1 && btn_prev_state == 0) {
            is_metric = !is_metric;
            filter_count = 0;
            filter_index = 0;
            printf("[Main] >> MODE CHANGED: %s\n", is_metric ? "METRIC (L/100Km)" : "IMPERIAL (Gal/100mi)");
        }
        btn_prev_state = btn_state;

        GetAndResetCounts(&pulse_speed, &pulse_flow);

        double raw_consumption = CalculateFuelConsumption(pulse_speed, pulse_flow, is_metric);
        consumption = ApplyMovingAverage(raw_consumption);

        char unit_char = is_metric ? 'L' : 'G';
        snprintf(uart_buffer, sizeof(uart_buffer), "%c%04.1f", unit_char, consumption);

        SerialCom_SendString(serial_fd, uart_buffer);

        LOG("[Main] Pulses: S=%d F=%d | Consumption: %.1f %s | UART: %s\n",
            pulse_speed, pulse_flow, consumption,
            is_metric ? "L/100Km" : "Gal/100mi",
            uart_buffer);

        usleep(200000);
    }

    CleanupHardware();
    printf("[Main] Application terminated.\n");

    return EXIT_SUCCESS;
}