#include "display.h"
#include <U8g2lib.h>
#include <SPI.h>

// pin definitions
static const int PIN_SCK  = 11;  // OLED D0
static const int PIN_MOSI = 10;  // OLED D1
static const int PIN_RES  = 9;
static const int PIN_DC   = 8;
static const int PIN_CS   = 7;

//button pins
static const int BTN_FEED = 4;
static const int BTN_PLAY = 5;
static const int BTN_BED  = 6;

// animation variables
int petYOffset = 0;
bool goingUp = true;
unsigned long lastAnim = 0;

// global variables
bool showHeart = false;
bool isSleeping = false;
bool isWalking = false;
bool playMode = false;
unsigned long heartTimer = 0;
int hungerLevel = 4; // Start with some hunger
unsigned long walkTimer = 0;

extern int stepCount;
extern int happiness;


U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI
u8g2(U8G2_R0, PIN_CS, PIN_DC, PIN_RES);

// bitmaps
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


//heart display
static const uint8_t HEART_BITMAP[] = {
    0b01100110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00000000
};

//functions
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

    u8g2.setDrawColor(0); //black holes for face
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


void setup_buttons() {
    pinMode(BTN_FEED, INPUT_PULLUP);
    pinMode(BTN_PLAY, INPUT_PULLUP);
    pinMode(BTN_BED,  INPUT_PULLUP);
}

void check_buttons() {
    // Play Button -> Heart
    // if (digitalRead(BTN_PLAY) == LOW) {
    //     showHeart = true;
    //     heartTimer = millis();
    //     delay(200); 
    // }
    if (digitalRead(BTN_PLAY) == LOW) {
    Serial.println("PLAY MODE");

    playMode = true;        // 👈 switch UI
    isWalking = true;
    walkTimer = millis();

    showHeart = true;
    heartTimer = millis();
}

    // Feed Button -> Increase Hunger Bar
    if (digitalRead(BTN_FEED) == LOW) {
        if (hungerLevel < 6) hungerLevel++;
    }

    // Sleep Button -> Turn off screen for a couple of seconds and display "zzz"
    if (digitalRead(BTN_BED) == LOW) {
        isSleeping = !isSleeping; // Toggle sleep on/off
    }
}

void updatePetAnimation(bool isWalking) {
    if (!isWalking) {
        petYOffset = 0;
        return;
    }

    if (millis() - lastAnim > 150) {
        lastAnim = millis();

        if (goingUp) {
            petYOffset = -2;
            goingUp = false;
        } else {
            petYOffset = 0;
            goingUp = true;
        }
    }
}

void display_Homepage() {
    char hungerBar[8];
    for (int i = 0; i < 6; ++i) {
        hungerBar[i] = (i < hungerLevel) ? '#' : '-';
    }
    hungerBar[6] = '\0';

    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, 128, 64);
    
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(4, 11, "Hunger:");
    u8g2.drawStr(48, 11, hungerBar);

    if (playMode) {
        // Steps taken by user
        char stepStr[20];
        sprintf(stepStr, "Steps: %d", stepCount);
        u8g2.drawStr(4, 25, stepStr);

        // Happiness meter
        u8g2.drawStr(4, 40, "Mood:");

        for (int i = 0; i < happiness && i < 5; i++) {
            u8g2.drawStr(50 + i*10, 40, "<3");
        }
    }

    updatePetAnimation(isWalking);
    drawPetSprite(48, 18 + petYOffset);

    if (!playMode) {
        u8g2.drawStr(7, 61, "Feed");
        u8g2.drawStr(50, 61, "Play");
        u8g2.drawStr(95, 61, "Bed");
    }

    if (showHeart) {
        u8g2.drawXBMP(82, 16, 8, 8, HEART_BITMAP);
        if (millis() - heartTimer > 1000) showHeart = false; 
    }

    if (isSleeping) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(50, 32, "Zzz..."); 
    u8g2.sendBuffer();
    return; // Skip drawing the rest of the UI
}

    u8g2.sendBuffer();
}
