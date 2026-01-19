#include "lcd_display.h"

static const uint8_t SEG_A[8] = {0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t SEG_BC[8] = {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18};
static const uint8_t SEG_D[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F};
static const uint8_t SEG_EF[8] = {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};
static const uint8_t SEG_G[8] = {0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t SEG_DP[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03};
static const uint8_t SEG_GD[8] = {0x1F, 0X1F, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F};
static const uint8_t SEG_CDP[8]= {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1B, 0x1B};

enum {
    SLOT_TOP    = 0,
    SLOT_RIGHT  = 1,
    SLOT_BOTTOM = 2,
    SLOT_LEFT   = 3,
    SLOT_MID    = 4,
    SLOT_COMBI  = 5,
    SLOT_DP     = 6,
    SLOT_CDP    = 7
};

#define LCD_BL 0x08
#define LCD_EN 0x04

DisplayLCD::DisplayLCD() {
    memset(_digitState, 0, 4);
}

void DisplayLCD::I2C_Init(void){
    TWSR = 0x00;
    TWBR = (1 << TWBR6) | (1 << TWBR3);
    TWCR = (1 << TWEN);
}

void DisplayLCD::I2C_Start(void){
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while(!(TWCR & (1 << TWINT)));
}

void DisplayLCD::I2C_Stop(void){
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void DisplayLCD::I2C_Write(uint8_t data){
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while(!(TWCR & (1 << TWINT)));
}

void DisplayLCD::LCD_Pulse(uint8_t nibble, uint8_t mode) {
    uint8_t data = (nibble & 0xF0) | mode | LCD_BL;

    I2C_Start();
    I2C_Write(LCD_I2C_ADDRESS);
    I2C_Write(data | LCD_EN);
    I2C_Write(data & ~LCD_EN);
    I2C_Stop();
    delayMicroseconds(50);
}

void DisplayLCD::LCD_Send(uint8_t val, uint8_t mode){
    LCD_Pulse(val, mode);
    LCD_Pulse(val << 4, mode);
}

void DisplayLCD::LCD_Command(uint8_t cmd)   { LCD_Send(cmd, 0x00); }

void DisplayLCD::LCD_Data(uint8_t data) { LCD_Send(data, 0x01); }

void DisplayLCD::LCD_Create_Char(uint8_t location, const uint8_t* chrmap) {
    LCD_Command(0x40 | (location << 3));
    for(int i = 0; i < 8; i++)
        LCD_Data(chrmap[i]);
}

void DisplayLCD::Setup(void) {
    I2C_Init();
    delay(50);

    LCD_Pulse(0x30, 0x00);
    delay(5);
    LCD_Pulse(0x30, 0x00);
    delayMicroseconds(150);
    LCD_Pulse(0x30, 0x00);
    LCD_Pulse(0x20, 0x00);

    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Command(0x01);
    delay(2);

    LCD_Create_Char(SLOT_TOP,   SEG_A);
    LCD_Create_Char(SLOT_RIGHT, SEG_BC);
    LCD_Create_Char(SLOT_BOTTOM,SEG_D);
    LCD_Create_Char(SLOT_LEFT,  SEG_EF);
    LCD_Create_Char(SLOT_MID,   SEG_G);
    LCD_Create_Char(SLOT_COMBI, SEG_GD);
    LCD_Create_Char(SLOT_DP,    SEG_DP);
    LCD_Create_Char(SLOT_CDP,   SEG_CDP);

    LCD_Command(0x01);
}

void DisplayLCD::SetSegment(int digit, char segment, bool state) {
    if (digit < 0 || digit > 3)
        return;
    
    int bit = -1; 
    segment = toupper(segment);
    switch(segment) {
        case 'A': bit = 0; break;
        case 'B': bit = 1; break;
        case 'C': bit = 2; break;
        case 'D': bit = 3; break;
        case 'E': bit = 4; break;
        case 'F': bit = 5; break;
        case 'G': bit = 6; break;
        case 'P': bit = 7; break;
        default: return;
    }
    if (bit != -1) {
        if (state)
            _digitState[digit] |= (1 << bit);
        else
            _digitState[digit] &= ~(1 << bit);
    }
}

void DisplayLCD::Update(void){
    for(int i =0; i < 4; i++) {
        int x = i * 4;
        uint8_t state = _digitState[i];

        LCD_Command(0x80 | x);
        LCD_Data((state & (1 << 5)) ? SLOT_LEFT : ' ');
        LCD_Data((state & (1 << 0)) ? SLOT_TOP: ' ');
        LCD_Data((state & (1 << 1)) ? SLOT_RIGHT: ' ');

        LCD_Command(0xC0 | x);
        LCD_Data((state & (1 << 4)) ? SLOT_LEFT: ' ');

        bool g = state & (1 << 6);
        bool d = state & (1 << 3);

        if (g && d)
            LCD_Data(SLOT_COMBI);
        else if (g)
            LCD_Data(SLOT_MID);
        else if (d)
            LCD_Data(SLOT_BOTTOM);
        else
            LCD_Data(' ');

        bool c = state & (1 << 2);
        bool p = state & (1 << 7);

        if (c && p)
            LCD_Data(SLOT_CDP);
        else if (p)
            LCD_Data(SLOT_DP);
        else if (c)
            LCD_Data(SLOT_RIGHT);
        else
            LCD_Data(' ');
    }
    delay(1);
}

void DisplayLCD::ShowChar(int digit, char c, bool dot) {
    // Clear all segments (A-G and P) before setting new pattern
    _digitState[digit] = 0;

    uint8_t pattern = 0;

    if (c >= '0' && c <= '9'){
        const uint8_t digits[] = {
            0x3F,
            0x06,
            0x5B,
            0x4F,
            0x66,
            0x6D,
            0x7D,
            0x07,
            0x7F,
            0x6F
        };
        pattern = digits[c - '0'];
    }
    else {
        switch(toupper(c)){
            case 'L': pattern = 0x38; break;  // segments: E,F,D
            case 'G': pattern = 0x3D; break;  // segments: A,C,D,E,F
            case '-': pattern = 0x40; break;  // segment: G
            case ' ': pattern = 0x00; break;  // no segments
            case 'E': pattern = 0x79; break;  // segments: A,D,E,F,G
            case 'F': pattern = 0x71; break;  // segments: A,E,F,G
            case 'U': pattern = 0x3E; break;  // segments: B,C,D,E,F
        }
    }

    SetSegment(digit, 'A', pattern & (1 << 0));
    SetSegment(digit, 'B', pattern & (1 << 1));
    SetSegment(digit, 'C', pattern & (1 << 2));
    SetSegment(digit, 'D', pattern & (1 << 3));
    SetSegment(digit, 'E', pattern & (1 << 4));
    SetSegment(digit, 'F', pattern & (1 << 5));
    SetSegment(digit, 'G', pattern & (1 << 6));

    if (dot)
        SetSegment(digit, 'P', true);
}

void DisplayLCD::clear(void){
    memset(_digitState, 0, 4);
    Update();
}

void DisplayLCD::TEST(void){
    for(int i = 0; i < 4; i++)
        _digitState[i] = 0xFF;
    Update();
}