#include "display.h"
#include <U8g2lib.h>
#include <SPI.h>

// CHANGE THESE to match your wiring
static const int PIN_SCK  = 11;  // OLED D0
static const int PIN_MOSI = 10;  // OLED D1
static const int PIN_CS   = 7;
static const int PIN_DC   = 8;
static const int PIN_RES  = 9;

// Try SSD1306 first
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI
u8g2(U8G2_R0, PIN_CS, PIN_DC, PIN_RES);

// EDITABLE ORANGE CHARACTER (32x32)
// This binary grid matches your cute orange reference image!
static const uint32_t ORANGE_SHAPE[] = {
    0b00000000000000000000000000000000, 
    0b00000000000000000000111000000000, // Leaf top
    0b00000000000000000011111100000000, // Leaf
    0b00000000000000000011111100000000, 
    0b00000000001111111100111000000000, // Head Start
    0b00000001111111111111000000000000,
    0b00000111111111111111110000000000,
    0b00001111111111111111111000000000,
    0b00011111111111111111111100000000,
    0b00011111111111111111111100000000,
    0b00111111111111111111111110000000,
    0b00111111111111111111111110000000,
    0b00111111111111111111111110000000,
    0b00111111111111111111111110000000,
    0b00111111111111111111111110000000,
    0b00111111111111111111111110000000,
    0b00111111111111111111111110000000,
    0b00111111111111111111111110000000,
    0b00111111111111111111111110000000,
    0b00011111111111111111111100000000,
    0b00011111111111111111111100000000,
    0b00001111111111111111111000000000,
    0b00000111111111111111110000000000,
    0b00000001111111111111000000000000,
    0b00000000011100001110000000000000, // Feet
    0b00000000011100001110000000000000  
};

static void drawPetSprite(int x, int y)
{
    // 1. Draw the white body silhouette
    u8g2.setDrawColor(1);
    for (int row = 0; row < 26; ++row) // Adjusted for actual sprite height
    {
        for (int col = 0; col < 32; ++col)
        {
            if ((ORANGE_SHAPE[row] >> (31 - col)) & 1)
            {
                u8g2.drawPixel(x + col, y + row);
            }
        }
    }

    // 2. Punch holes for eyes and mouth (Draw in Black)
    u8g2.setDrawColor(0);
    u8g2.drawBox(x + 9, y + 14, 2, 3);  // Left Eye
    u8g2.drawBox(x + 17, y + 14, 2, 3); // Right Eye
    
    u8g2.drawPixel(x + 12, y + 18);     // Smile Left
    u8g2.drawPixel(x + 13, y + 19);     // Smile Bottom
    u8g2.drawPixel(x + 14, y + 18);     // Smile Right
    
    u8g2.setDrawColor(1); // Reset back to white
}

void display_init()
{
    SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);
    u8g2.begin();
    u8g2.setContrast(255);
}

void display_drawHello()
{
    const int hungerLevel = 4; 
    char hungerBar[8];

    // Build the hunger bar string
    for (int i = 0; i < 6; ++i)
    {
        hungerBar[i] = (i < hungerLevel) ? '#' : '-';
    }
    hungerBar[6] = '\0';

    u8g2.clearBuffer();

    // UI Frame
    u8g2.drawFrame(0, 0, 128, 64);

    // Header Status
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(4, 11, "Hunger:");
    u8g2.drawStr(48, 11, hungerBar);

    // Centered Orange Character
    // X=48 centers a 32px sprite on a 128px screen
    drawPetSprite(48, 18);

    // Footer Menu
    u8g2.drawStr(7, 61, "Feed");
    u8g2.drawStr(50, 61, "Play");
    u8g2.drawStr(95, 61, "Bed");

    u8g2.sendBuffer();
}