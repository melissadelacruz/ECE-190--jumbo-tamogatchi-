#ifndef IMU_H
#define IMU_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

extern Adafruit_MPU6050 mpu;

void imuSetup();
void imuRead(float &ax, float &ay, float &az,
             float &gx, float &gy, float &gz);

#endif