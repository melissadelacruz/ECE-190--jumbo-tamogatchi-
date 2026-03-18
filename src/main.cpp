#include <Arduino.h>
#include "display/display.h"
#include "sensors/imu.h"
#include <WiFi.h>
#include <time.h>

#define WINDOW 10

// ===== PEDOMETER =====
float buffer[WINDOW];
int bufIndex = 0;

int stepCount = 0;
bool stepDetected = false;

float baseline = 29.0;
float ax, ay, az;
float gx, gy, gz;

// ===== CHARACTER =====
int happiness = 0;
unsigned long lastStepTime = 0;

// ===== WIFI =====
const char* ssid = "SpectrumSetup-3022";
const char* password = "nationalregister925";

static const unsigned long WIFI_CONNECT_TIMEOUT_MS = 10000;
static const unsigned long WIFI_RETRY_INTERVAL_MS = 10000;

unsigned long lastWiFiRetry = 0;

// ===== SETUP =====
void setup() {
    Serial.begin(115200);
    delay(1500);

    Serial.println("Booting IMU...");
    imuSetup();

    display_init();
    setup_buttons();

    // ---- WiFi Connect ----
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    unsigned long wifiStart = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - wifiStart < WIFI_CONNECT_TIMEOUT_MS) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());

        // ---- TIME SETUP ----
        configTime(-8 * 3600, 3600, "pool.ntp.org"); // PST + DST auto

    } else {
        Serial.println("\nWiFi failed, continuing offline.");
    }
}

// ===== LOOP =====
void loop() {

    // ===== TIME UPDATE =====
    struct tm timeinfo;

    if (WiFi.status() == WL_CONNECTED && getLocalTime(&timeinfo)) {

        int hours24 = timeinfo.tm_hour;
        int minutes = timeinfo.tm_min;

        bool isPM = hours24 >= 12;
        int hours12 = hours24 % 12;
        if (hours12 == 0) hours12 = 12;

        display_setTime(hours12, minutes, isPM);

        int month = timeinfo.tm_mon + 1;
        int date  = timeinfo.tm_mday;

        display_setDate(month, date);

        // DEBUG (optional)
        Serial.printf("Time: %02d:%02d | Date: %02d/%02d\n",
                      hours24, minutes, month, date);
    }
    else if (millis() - lastWiFiRetry >= WIFI_RETRY_INTERVAL_MS) {
        lastWiFiRetry = millis();
        Serial.println("Retrying WiFi...");
        WiFi.disconnect();
        WiFi.begin(ssid, password);
    }

    // ===== IMU READ =====
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

    // 3. Detrend
    float dt = avg - baseline;

    // 4. Step detection
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

    // ===== UI =====
    check_buttons();
    display_Homepage();
}
