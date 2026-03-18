#include <Arduino.h>
#include "display/display.h"
#include "sensors/imu.h"

float ax, ay, az;
float gx, gy, gz;

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("Booting IMU test...");
    imuSetup();
    Serial.println("IMU setup complete");
}

void loop() {
    imuRead(ax, ay, az, gx, gy, gz);

    Serial.print("AX: "); Serial.print(ax);
    Serial.print(" AY: "); Serial.print(ay);
    Serial.print(" AZ: "); Serial.println(az);

    delay(50);
}

// void setup() {
//     display_init();
//     setup_buttons();
// }

// void loop() {
//     check_buttons();
//     display_drawHello();
//     delay(10);
// }
