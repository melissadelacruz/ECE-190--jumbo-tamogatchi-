<<<<<<< HEAD
**ECE 190 Project**


This is my repo for my ECE 190 project. I will be integrating things such as machine learning, DSP, data analysis, and etc to make this project. I want to be off of of social media and to do so I will focus on creating a physical device that I can use when I have digital fatigue.
Below is a Google drive link that includes other types of documentation for this project. 

Thanks! :D

https://drive.google.com/drive/folders/1MEKUAmXFv2e_WvpG6FI-sjovKOeH6pkQ?usp=drive_link
=======
# ESP32-S3 SPI OLED Test Branch

This branch contains initial firmware testing for:

- ESP32-S3 Mini Dev Board
- 0.96" SPI OLED (SSD1306/SH1106)
- PlatformIO project structure

Pin mapping used:

Screen:  
D0 (SCK)  → GPIO 11  
D1 (MOSI) → GPIO 10  
CS        → GPIO 7  
DC        → GPIO 8  
RES       → GPIO 9  
VCC       → 3.3V

Buttons:  
One End → GND  
Other End → GPIO 5  

One End → GND  
Other End GPIO 4



Library: U8g2  
Framework: Arduino (PlatformIO)

Purpose:
Initial display bring-up and modular firmware structure test.

## Firmware navigation

- `src/main.cpp`: Arduino entry points; calls the application.
- `src/app/app.cpp`: startup, sensor updates, and the main update loop.
- `src/app/state.h`: the shared runtime `PetState` and meter limits.
- `src/app/pet.cpp`: pet meter updates and saved state.
- `src/app/network.cpp`: Wi-Fi, time, and weather.
- `src/display/`: OLED drawing and button handling.
- `src/sensors/`: IMU, pedometer, and shake detection.

The application, pet logic, and display currently share one `petState` object.
The step count and three meters are saved using ESP32 Preferences. Sensor debug
readings continue to appear in the serial monitor at 115200 baud.
