// #include <Arduino_GFX_Library.h>

// // PIN MAPPING (Using the ones we discussed)
// #define TFT_SCK  7
// #define TFT_MOSI 6
// #define TFT_MISO -1 
// #define TFT_CS   5
// #define TFT_DC   4
// #define TFT_RST  15

// // Software SPI Bus - This bypasses the hardware that causes the "Panic"
// Arduino_DataBus *bus = new Arduino_SWSPI(
//     TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO
// );

// // ST7796 Driver for the 4.0" TN Panel
// Arduino_GFX *gfx = new Arduino_ST7796(
//     bus, TFT_RST, 1 /* Rotation */, false /* IPS */
// );

// void setup() {
//   Serial.begin(115200);
//   delay(3000);
//   Serial.println("Starting Software SPI - No Hardware Hooks");

//   if (!gfx->begin()) {
//     Serial.println("gfx->begin() failed!");
//   } else {
//     Serial.println("SUCCESS! Code is running.");
//     gfx->fillScreen(0x07E0); // Bright Green
//   }
// }

// void loop() {
//   gfx->fillScreen(0xF800); // Red
//   delay(1000);
//   gfx->fillScreen(0x001F); // Blue
//   delay(1000);
// }
