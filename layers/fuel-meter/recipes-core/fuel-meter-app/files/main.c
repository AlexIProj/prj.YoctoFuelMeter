#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#include "config.h"
#include "calcs.h"
#include "serial_com.h"
#include "gpio_driver.h"
#include "sensors.h"

int main(){
    struct gpiod_line_request* req_speed = ConfigLineInput(GPIO_CHIP_PATH, PIN_SPEED_SEN, "speed_sensor", 1);
    struct gpiod_line_request* req_flow = ConfigLineInput(GPIO_CHIP_PATH, PIN_FLOW_SEN, "flow_sensor", 1);
    struct gpiod_line_request* req_button = ConfigLineInput(GPIO_CHIP_PATH, PIN_SWITCH_BTN, "switch_button", 0);

    if (!req_speed || !req_flow) {
        fprintf(stderr, "Failed to configure GPIO lines\n");
        return 1;
    }

    pthread_t speed_thread, flow_thread;
    pthread_create(&speed_thread, NULL, Thread_CountSpeed, req_speed);
    pthread_create(&flow_thread, NULL, Thread_CountFlow, req_flow);

    int serial_fd = SerialCom_Init();
    if (serial_fd < 0)  LOG("Warn: Serial not found\n");

    int is_metric = 1;
    int last_btn_state = 1;

    LOG("Fuel Meter App Started\n");

    while(1) {
        int current_btn_state = gpiod_line_request_get_value(req_button, PIN_SWITCH_BTN);
        if (current_btn_state == 0 && last_btn_state == 1)
            is_metric = !is_metric;
        last_btn_state = current_btn_state;

        int p_speed, p_flow;
        GetAndResetCounts(&p_speed, &p_flow);

        double res = CalculateFuelConsumption(p_speed, p_flow, is_metric);
        LOG("\r[DEBUG] MODE: %s | SPEED PULSES: %d | FLOW PULSES: %d | FUEL CONS: %.1f %s/L",
            is_metric ? "METRIC" : "IMPERIAL",
            p_speed,
            p_flow,
            res,
            is_metric ? "L" : "G"
        );
        #ifdef DEBUG
            fflush(stdout);
        #endif

        if (serial_fd >= 0) {
            SerialCom_SendData(serial_fd, res);
        }
        else {
            serial_fd = SerialCom_Init();
        }
        usleep(250000);
    }

    return 0;
}