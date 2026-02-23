#include "display.h"
#include <U8g2lib.h>
#include <SPI.h>

// 🔧 CHANGE THESE to match your wiring
static const int PIN_SCK  = 11;  // OLED D0
static const int PIN_MOSI = 10;  // OLED D1
static const int PIN_CS   = 7;
static const int PIN_DC   = 8;
static const int PIN_RES  = 9; 

// Try SSD1306 first
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI 
u8g2(U8G2_R0, PIN_CS, PIN_DC, PIN_RES);

// If screen is blank, switch to:
// U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI 
// u8g2(U8G2_R0, PIN_CS, PIN_DC, PIN_RES);

void display_init()
{
    SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);
    u8g2.begin();
}

void display_drawHello()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 20, "ESP32-S3 Ready");
    u8g2.drawStr(0, 40, "Melissa OS v1 :)");
    u8g2.sendBuffer();
}
