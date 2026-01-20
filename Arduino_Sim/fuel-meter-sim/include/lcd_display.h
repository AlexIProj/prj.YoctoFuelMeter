#ifndef DISPLAY_LCD_H
#define DISPLAY_LCD_H

#include <Arduino.h>
#include "config.h"

class DisplayLCD {
    public:
        DisplayLCD();

        void Setup(void);
        void SetSegment(int digit, char segment, bool state);
        void ShowChar(int digit, char c, bool dot);
        void Update(void);

        void clear(void);

    private:
        uint8_t _digitState[4];

        void I2C_Init(void);
        void I2C_Start(void);
        void I2C_Stop(void);
        void I2C_Write(uint8_t data);

        void LCD_Pulse(uint8_t nibble, uint8_t mode);
        void LCD_Send(uint8_t value, uint8_t mode);
        void LCD_Command(uint8_t cmd);
        void LCD_Data(uint8_t data);
        void LCD_Create_Char(uint8_t location, const uint8_t* chrmap);
};

#endif