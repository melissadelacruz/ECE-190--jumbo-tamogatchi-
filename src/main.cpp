#include <Arduino.h>
#include "display/display.h"
#include "sensors/imu.h"
#define WINDOW 10

//pedometer
float buffer[WINDOW];
int bufIndex = 0;

int stepCount = 0;
bool stepDetected = false;

float baseline = 29.0; // ~9.8 * 3 (L1 norm baseline)
float ax, ay, az;
float gx, gy, gz;

// character display
int happiness = 0;
unsigned long lastStepTime = 0;


void setup() {
    //pedometer
    Serial.begin(115200);
    delay(1500);
    Serial.println("Booting IMU test...");
    imuSetup();
    Serial.println("IMU setup complete");

    //display
    display_init();
    setup_buttons();
}

void loop() {

    imuRead(ax, ay, az, gx, gy, gz);

    // 1. L1 norm
    float l1 = abs(ax) + abs(ay) + abs(az);

    // 2. Moving average
    buffer[bufIndex] = l1;
    bufIndex = (bufIndex + 1) % WINDOW;

    float avg = 0;
    for (int i = 0; i < WINDOW; i++) {
        avg += buffer[i];
    }
    avg /= WINDOW;

    // 3. Detrend (remove gravity)
    float dt = avg - baseline;

    // 4. Step detection (PEAK DETECTION)
    if (dt > 2.0 && !stepDetected) {
        stepCount++;
        stepDetected = true;
        lastStepTime = millis();

        Serial.print("Steps: ");
        Serial.println(stepCount);
        
        if (stepCount % 15 == 0 && stepCount != 0) {
            happiness++;
            Serial.println("💖 Pet is happier!");
        }
    }

    if (dt < 1.0) {
        stepDetected = false;
    }

    // ---- DISPLAY + BUTTONS ----
    check_buttons();

    // TODO: replace later
    display_Homepage();

}

