#include "imu.h"

Adafruit_MPU6050 mpu;

void imuSetup() {
    Wire.begin(16, 15); // SDA, SCL
    
    if (!mpu.begin()) {
        Serial.println("MPU6050 not found");
        while (1) delay(10);
    }
}

void imuRead(float &ax, float &ay, float &az,
             float &gx, float &gy, float &gz) {

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    ax = a.acceleration.x;
    ay = a.acceleration.y;
    az = a.acceleration.z;

    gx = g.gyro.x;
    gy = g.gyro.y;
    gz = g.gyro.z;
}
