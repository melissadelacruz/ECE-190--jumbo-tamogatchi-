# ESP32-S3 SPI OLED Test Branch

This branch contains initial firmware testing for:

- ESP32-S3 Mini Dev Board
- 0.96" SPI OLED (SSD1306/SH1106)
- PlatformIO project structure

Pin mapping used:

D0 (SCK)  → GPIO 11  
D1 (MOSI) → GPIO 10  
CS        → GPIO 7  
DC        → GPIO 8  
RES       → GPIO 9  

Library: U8g2  
Framework: Arduino (PlatformIO)

Purpose:
Initial display bring-up and modular firmware structure test.
