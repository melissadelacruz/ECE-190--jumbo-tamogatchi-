#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;
const int LIGHT_PIN = 1; // ADC1 channel on S3

void setup() {
  Serial.begin(115200);
  
  // Custom I2C pins for ESP32-S3
  Wire.begin(7, 8); 

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) { delay(10); }
  }
  
  // Print Header for CSV
  Serial.println("timestamp,ax,ay,az,gx,gy,gz,light");
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  int lightVal = analogRead(LIGHT_PIN);

  // CSV Output
  Serial.print(millis()); Serial.print(",");
  Serial.print(a.acceleration.x); Serial.print(",");
  Serial.print(a.acceleration.y); Serial.print(",");
  Serial.print(a.acceleration.z); Serial.print(",");
  Serial.print(g.gyro.x); Serial.print(",");
  Serial.print(g.gyro.y); Serial.print(",");
  Serial.print(g.gyro.z); Serial.print(",");
  Serial.println(lightVal);

  delay(50); // Record at 20Hz
}