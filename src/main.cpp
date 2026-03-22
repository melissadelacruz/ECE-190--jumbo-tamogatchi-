#include <Arduino.h>
#include "display/display.h"
#include "sensors/imu.h"
#include "sensors/pedometer.h"
#include "sensors/shake.h"
#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Create sensor objects
Pedometer pedometer;
ShakeDetector shakeDetector;

// Global variables for display (defined here)
int stepCount = 0;
int happiness = 0;
int fullnessLevel = 4;
int energyLevel = 0;
int hungerLevel = 4;

// Mode flags
bool playMode = false;
bool feedMode = false;
bool isSleeping = false;
bool isWalking = false;

unsigned long heartTimer = 0;
unsigned long lastShakeTime = 0;
unsigned long lastEnergyUpdate = 0;

// WiFi credentials
const char* ssid = "SpectrumSetup-3022";
const char* password = "nationalregister925";
const char* weatherApiKey = WEATHER_API_KEY;

static const unsigned long WIFI_CONNECT_TIMEOUT_MS = 10000;
static const unsigned long WIFI_RETRY_INTERVAL_MS = 10000;
unsigned long lastWiFiRetry = 0;
unsigned long lastWeatherFetch = 0;
const unsigned long WEATHER_INTERVAL = 60000;

void fetchTemperature() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return;
    }

    Serial.println("Fetching weather...");
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String url = "https://api.openweathermap.org/data/2.5/weather?zip=92130,US&units=imperial&appid=" + String(weatherApiKey);
    
    http.begin(client, url);
    int httpCode = http.GET();

    if (httpCode == 200) {
        String payload = http.getString();
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error && doc["main"]["temp"]) {
            int displayTemp = (int)doc["main"]["temp"];
            display_setTemp(displayTemp);
            Serial.print("Temp: ");
            Serial.println(displayTemp);
        }
    }
    http.end();
}

void setup() {
    Serial.begin(115200);
    delay(1500);

    Serial.println("Initializing IMU...");
    imuSetup();
    
    // Initialize sensors
    pedometer.begin();
    shakeDetector.setThreshold(2.5);
    shakeDetector.setCooldown(500);
    shakeDetector.setSamplesRequired(3);

    display_init();
    setup_buttons();
    
    // WiFi Connect
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    unsigned long wifiStart = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < WIFI_CONNECT_TIMEOUT_MS) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected!");
        configTime(-8 * 3600, 3600, "pool.ntp.org");
        fetchTemperature();
    } else {
        Serial.println("\nWiFi failed, continuing offline.");
    }
}

void loop() {
    // Time Update
    struct tm timeinfo;
    if (WiFi.status() == WL_CONNECTED && getLocalTime(&timeinfo)) {
        int hours24 = timeinfo.tm_hour;
        int minutes = timeinfo.tm_min;
        bool isPM = hours24 >= 12;
        int hours12 = hours24 % 12;
        if (hours12 == 0) hours12 = 12;
        
        display_setTime(hours12, minutes, isPM);
        display_setDate(timeinfo.tm_mon + 1, timeinfo.tm_mday);
    } else if (millis() - lastWiFiRetry >= WIFI_RETRY_INTERVAL_MS) {
        lastWiFiRetry = millis();
        WiFi.disconnect();
        WiFi.begin(ssid, password);
    }
    
    // Weather Update
    if (millis() - lastWeatherFetch > WEATHER_INTERVAL) {
        lastWeatherFetch = millis();
        fetchTemperature();
    }
    
    // IMU Read
    float ax, ay, az, gx, gy, gz;
    imuRead(ax, ay, az, gx, gy, gz);
    
    // PEDOMETER - Only in PLAY mode
    if (playMode) {
        if (pedometer.update(ax, ay, az)) {
            stepCount = pedometer.getStepCount();
            happiness = pedometer.getHappiness();
            isWalking = true;
        }
    } else {
        isWalking = false;
    }
    
    // SHAKE DETECTOR - Only in FEED mode
    if (feedMode) {
        if (shakeDetector.update(ax, ay, az)) {
            if (fullnessLevel < 6) {
                fullnessLevel++;
                if (happiness < 5) {
                    happiness++;
                    pedometer.setHappiness(happiness);
                }
            }
            lastShakeTime = millis();
        }
    }
    
    // ENERGY UPDATE - Only in BED mode
    if (isSleeping) {
        if (millis() - lastEnergyUpdate > 30000) {
            lastEnergyUpdate = millis();
            if (energyLevel < 5) {
                energyLevel++;
            }
        }
    }
    
    // UI
    check_buttons();
    display_Home();
    delay(50);
}
