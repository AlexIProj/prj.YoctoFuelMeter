#ifndef CONFIG_H
#define CONFIG_H

#include "Arduino.h"

#define SIM_PORT_DDR    DDRB
#define SIM_PORT_OUT    PORTB
#define SIM_PORT_PIN    PINB
#define PIN_SPEED_BIT   1
#define PIN_FLOW_BIT    0

#define BTN_PORT_DDR    DDRD
#define BTN_PORT_OUT    PORTD
#define PIN_BTN_BIT     7

#define LCD_I2C_ADDRESS (0x27 << 1)

#define DEBUG_ENABLED   10

#endif