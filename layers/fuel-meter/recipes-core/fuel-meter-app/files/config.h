#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

// ------------------ RASPBERRY PI ------------------
#define GPIO_CHIP_PATH  "/dev/gpiochip0"
#define SERIAL_PORT     "/dev/ttyACM0"
#define PIN_SPEED_SEN   17
#define PIN_FLOW_SEN    27
#define PIN_SWITCH_BTN  22

#define BAUD_RATE       115200

// ------------------ CONSTANTS -------------------
#define K_FLOW_SEN      12.0    // pulses per cm3
#define K_SPEED_SEN     6000.0  // pulses per mile

#define KM_PER_MILES    1.609344
#define CM3_PER_LITER   1000.0
#define CM3_PER_GALLON  3785.41

// ------------------ DEBUG --------------------
#ifdef DEBUG
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...) do {} while (0)
#endif

#endif