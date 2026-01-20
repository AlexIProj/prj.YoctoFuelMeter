#ifndef CONFIG_H
#define CONFIG_H

#include "Arduino.h"

// Speed (Pin 9 = PB1) and Flow (Pin 8 = PB0) share PORTB
#define SIM_PORT_DDR    DDRB
#define SIM_PORT_OUT    PORTB
#define SIM_PORT_PIN    PINB
#define PIN_SPEED_BIT   1   // Arduino Pin 9 = PB1
#define PIN_FLOW_BIT    0   // Arduino Pin 8 = PB0

// Button on Pin 7 = PD7
#define BTN_PORT_DDR    DDRD
#define BTN_PORT_OUT    PORTD
#define PIN_BTN_BIT     7   // Arduino Pin 7 = PD7

// LCD I2C Address
#define LCD_I2C_ADDRESS (0x27 << 1)

#endif