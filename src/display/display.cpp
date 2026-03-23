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
static const int BTN_BED  = 5;
static const int BTN_PLAY = 6;

// animation variables
int petYOffset = 0;
bool goingUp = true;
unsigned long lastAnim = 0;

// global variables
// Button edge state is local to the display/button module.
bool lastPlayState = HIGH;
bool lastFeedState = HIGH;
bool lastBedState = HIGH;
unsigned long lastShakeTime = 0;

extern int stepCount;
extern int happiness;

//time variables
int displayHours = 0;
int displayMinutes = 0;
bool displayPM = false;
int displayMonth = 0;
int displayDay = 0;

// weather variables
int displayTemp =0;


// display screen
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI
u8g2(U8G2_R0, PIN_CS, PIN_DC, PIN_RES);

// bitmaps

//pet
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


// heart
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

// Thunder bolt
static const uint8_t THUNDER_BITMAP[] = {
    0b00010000,
    0b00011000,
    0b00011100,
    0b11111110,
    0b01111111,
    0b00111000,
    0b00011000,
    0b00001000
};

//for fullness
static const uint8_t ICECREAM_BITMAP[] = {
    0b00111000,
    0b01111100,
    0b01111100,
    0b01111100,
    0b01111100,
    0b00111000,
    0b00010000,
    0b00010000
};

// functions
void display_setTemp(int t) {
    displayTemp = t;
}

void display_setDate(int m, int d) {
    displayMonth = m;
    displayDay = d;
}

void display_setTime(int h, int m, bool isPM) {
    displayHours = h;
    displayMinutes = m;
    displayPM = isPM;
}

// Modified drawPetSprite with size parameter (scale factor)
static void drawPetSprite(int x, int y, bool isSleeping = false, float scale = 1.0) {
    // Calculate scaled dimensions
    int width = 32;
    int height = 26;
    int scaledWidth = width * scale;
    int scaledHeight = height * scale;
    
    // Center the sprite when scaled
    int offsetX = (scaledWidth - width) / 2;
    int offsetY = (scaledHeight - height) / 2;
    
    u8g2.setDrawColor(1);
    
    // Draw scaled sprite
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            if ((ORANGE_SHAPE[row] >> (31 - col)) & 1) {
                // Draw scaled pixel (simple scaling)
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        u8g2.drawPixel(x + (col * scale) + sx - offsetX, 
                                      y + (row * scale) + sy - offsetY);
                    }
                }
            }
        }
    }

    u8g2.setDrawColor(0); // black holes for face
    
    // Adjust face positions based on scale
    int eyeOffset = scale * 9;
    int eyeYOffset = y + (scale * 14);
    
    if (isSleeping) {
        // Closed eyes
        u8g2.drawBox(x + (scale * 9), eyeYOffset, scale * 4, scale * 2);
        u8g2.drawBox(x + (scale * 17), eyeYOffset, scale * 4, scale * 2);
    } else {
        // Normal open eyes
        u8g2.drawBox(x + (scale * 9), eyeYOffset, scale * 2, scale * 3);
        u8g2.drawBox(x + (scale * 17), eyeYOffset, scale * 2, scale * 3);
        // Smile gets bigger with scale
        u8g2.drawPixel(x + (scale * 12), y + (scale * 18));
        u8g2.drawPixel(x + (scale * 13), y + (scale * 19));
        u8g2.drawPixel(x + (scale * 14), y + (scale * 18));
    }
    
    u8g2.setDrawColor(1);
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
    // Play Button
    bool currentPlayState = digitalRead(BTN_PLAY);
    if (lastPlayState == HIGH && currentPlayState == LOW) {
        // Enter/exit play mode and leave the other modes cleanly.
        if (!playMode) {
            feedMode = false;
            isSleeping = false;
        }

        playMode = !playMode;
        isWalking = playMode;
        heartTimer = millis();
    }
    lastPlayState = currentPlayState;

    // Feed Button
    bool currentFeedState = digitalRead(BTN_FEED);
    if (lastFeedState == HIGH && currentFeedState == LOW) {
        if (!feedMode) {
            playMode = false;
            isSleeping = false;
            isWalking = false;
        } 

        // Feed button acts as an enter/exit toggle for feed mode.
        feedMode = !feedMode;
    }
    lastFeedState = currentFeedState;

    // Sleep Button
    bool currentBedState = digitalRead(BTN_BED);
    if (lastBedState == HIGH && currentBedState == LOW) {
        playMode = false;
        feedMode = false;
        isSleeping = !isSleeping;
        isWalking = false;
    }
    lastBedState = currentBedState;
}

void updatePetAnimation(bool isWalking) {
    if (!isWalking) {
        petYOffset = 0;
        return;
    }

    if (millis() - lastAnim > 150) {
        lastAnim = millis();

        if (goingUp) {
            petYOffset = -16;
            goingUp = false;
        } else {
            petYOffset = 0;
            goingUp = true;
        }
    }
}

// Update display_Play() to show full play mode screen
void display_Play() {
    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.setFont(u8g2_font_6x10_tr);
    
    // Title
    u8g2.drawStr(4, 12, "PLAYING");
    
    // Steps taken by user
    char stepStr[20];
    sprintf(stepStr, "Steps: %d", stepCount);
    u8g2.drawStr(4, 28, stepStr);
    
    // Happiness meter
    u8g2.drawStr(4, 38, "Mood:");

    // draw bitmap hearts
    for (int i = 0; i < 5; i++) {
        if (i < happiness) {
            u8g2.drawXBMP(4 + i * 12, 46, 8, 8, HEART_BITMAP);
        }
    }
    
    // Show pet in play mode
    updatePetAnimation(isWalking);
    int petX = 80;
    int walkShift = (isWalking && millis() % 300 < 150) ? 1 : 0;
    drawPetSprite(petX + walkShift, 32 + petYOffset);
    
    u8g2.sendBuffer();
}

// Updated display_Feed function
void display_Feed() {
    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.setFont(u8g2_font_6x10_tr);
    
    // Title
    u8g2.drawStr(4, 12, "FEED MODE");
    
    // Instructions
    u8g2.drawStr(4, 24, "Shake to feed!");
    
    // Fullness bar
    u8g2.drawStr(4, 38, "Fullness:");
    
    // Draw bananas for fullness level (0-6)
    for (int i = 0; i < 6; i++) {
        int x = 4 + i * 12;
        int y = 42;
        
        if (i < fullnessLevel) {
            // Filled banana
            u8g2.drawXBMP(x, y, 8, 8, ICECREAM_BITMAP);
        } 
    }
    
    // Show fullness as text
    char fullnessStr[20];
    sprintf(fullnessStr, "%d/6", fullnessLevel);
    u8g2.drawStr(4, 60, fullnessStr);
    
    // Draw pet with size based on fullness level
    // Scale from 0.8 (skinny) to 1.5 (fat)
    float petScale = 0.7 + (fullnessLevel * 0.12);  // Min 0.7, Max 1.42
    if (petScale > 1.4) petScale = 1.4;
    if (petScale < 0.8) petScale = 0.8;
    
    // Position pet based on scale
    int petX = 80;
    int petY = 20;
    
    // Show walking animation in feed mode
    updatePetAnimation(isWalking);
    int walkShift = (isWalking && millis() % 300 < 150) ? 1 : 0;
    drawPetSprite(petX + walkShift, petY + petYOffset, false, petScale);    
    
    u8g2.sendBuffer();
}


// Update display_Bed() function with walking pet and energy bar
void display_Bed() {
    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, 128, 64);
    
    // Draw walking pet on the right with closed eyes (sleeping)
    updatePetAnimation(isWalking);
    int petX = 80;  // Position on the right
    int walkShift = (isWalking && millis() % 300 < 150) ? 1 : 0;
    drawPetSprite(petX + walkShift, 18 + petYOffset, true);  // true = sleeping (closed eyes)
    
    // Add sleeping Z's above pet
    u8g2.setFont(u8g2_font_6x10_tr);

    u8g2.drawStr(102, 10, "z");
    u8g2.drawStr(109, 15, "z");
    
    // Draw "Sleeping" text
    u8g2.drawStr(4, 12, "SLEEPING");
    
    // Draw energy bar - moved higher to better show thunder bolts
    u8g2.drawStr(4, 28, "Energy:");
    
    // Draw thunder bolts for energy level
    for (int i = 0; i < 5; i++) {
        int x = 4 + i * 12;
        int y = 32;  // Moved higher so bolts are clearly visible
        
        if (i < energyLevel) {
            // Filled thunder bolt
            u8g2.drawXBMP(x, y, 8, 8, THUNDER_BITMAP);
        } 
    }
    
    // Show energy level as text
    char energyStr[20];
    sprintf(energyStr, "%d/5", energyLevel);
    u8g2.drawStr(4, 50, energyStr);
    
    u8g2.sendBuffer();
}

// Update display_Home() to be the main dispatcher
void display_Home() {
    // Check states in priority order
    if (isSleeping) {
        display_Bed();
        return;
    }
    
    if (playMode) {
        display_Play();
        return;
    }

    if(feedMode) {
        display_Feed();
        return;
    }
    
    // Normal home screen (no special modes)
    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, 128, 64);
    
    u8g2.setFont(u8g2_font_6x10_tr);

    // Time UI
    char timeStr[20];
    sprintf(timeStr, "%d:%02d %s",
            displayHours,
            displayMinutes,
            displayPM ? "PM" : "AM");
    u8g2.drawStr(4, 12, timeStr);

    // Date UI
    char dateStr[20];
    sprintf(dateStr, "%02d/%02d", displayMonth, displayDay);
    u8g2.drawStr(4, 26, dateStr);

    // Temperature UI
    char tempStr[10];
    sprintf(tempStr, "%d%cF", displayTemp, 176);
    u8g2.drawStr(100, 12, tempStr);

    // Pet animation
    updatePetAnimation(isWalking);
    int petX = 48;
    int walkShift = (isWalking && millis() % 300 < 150) ? 1 : 0;
    drawPetSprite(petX + walkShift, 18 + petYOffset);

    // Button labels
    u8g2.drawStr(7, 61, "Feed");
    u8g2.drawStr(50, 61, "Play");
    u8g2.drawStr(100, 61, "Bed");
    
    
    u8g2.sendBuffer();
}
