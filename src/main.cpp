#include <Arduino.h>
#include "display/display.h"
#include "sensors/imu.h"
#include "sensors/pedometer.h"
#include "sensors/shake.h"
#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// Create sensor objects
Pedometer pedometer;
ShakeDetector shakeDetector;
Preferences preferences;

// Global variables for display (defined here)
int stepCount = 0;
int playHearts = 0;
int happiness = 0;
int fullnessLevel = 0;
int energyLevel = 0;

// Mode flags
bool playMode = false;
bool feedMode = false;
bool isSleeping = false;
bool isWalking = false;

unsigned long heartTimer = 0;
unsigned long lastEnergyUpdate = 0;

// WiFi credentials
const char* ssid = "melissa";
const char* password = "10306090";
const char* weatherApiKey = WEATHER_API_KEY;

static const unsigned long WIFI_CONNECT_TIMEOUT_MS = 10000;
static const unsigned long WIFI_RETRY_INTERVAL_MS = 10000;
static const unsigned long FULLNESS_DECAY_INTERVAL_MS = 10UL * 60UL * 1000UL;
static const unsigned long ENERGY_DECAY_INTERVAL_MS = 15UL * 60UL * 1000UL;
static const unsigned long PLAY_DECAY_INTERVAL_MS = 20UL * 60UL * 1000UL;
static const int MAX_PLAY_HEARTS = 5;
static const int MAX_FULLNESS = 6;
static const int MAX_ENERGY = 5;
unsigned long lastWiFiRetry = 0;
unsigned long lastWeatherFetch = 0;
unsigned long lastFullnessDecay = 0;
unsigned long lastEnergyDecay = 0;
unsigned long lastPlayDecay = 0;
const unsigned long WEATHER_INTERVAL = 60000;

static const char* wifiStatusName(wl_status_t status) {
    switch (status) {
        case WL_IDLE_STATUS: return "idle";
        case WL_NO_SSID_AVAIL: return "SSID not available";
        case WL_SCAN_COMPLETED: return "scan completed";
        case WL_CONNECTED: return "connected";
        case WL_CONNECT_FAILED: return "connection failed";
        case WL_CONNECTION_LOST: return "connection lost";
        case WL_DISCONNECTED: return "disconnected";
        default: return "unknown";
    }
}

static void printAvailableNetworks() {
    Serial.println("Scanning for WiFi networks...");
    int networkCount = WiFi.scanNetworks();

    if (networkCount == 0) {
        Serial.println("No WiFi networks found.");
        return;
    }

    Serial.print("Networks found: ");
    Serial.println(networkCount);

    for (int i = 0; i < networkCount; i++) {
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" | RSSI: ");
        Serial.print(WiFi.RSSI(i));
        Serial.print(" dBm | Channel: ");
        Serial.print(WiFi.channel(i));
        Serial.print(" | Encryption: ");
        Serial.println(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "secured");
    }
}

// persistence section
static int clampMeter(int value, int minValue, int maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

static int decayMeter(int value, int amount = 1) {
    value -= amount;
    return value < 0 ? 0 : value;
}

static void loadPetState() {
    preferences.begin("tama-pet", false);

    stepCount = preferences.getInt("steps", 0);
    playHearts = clampMeter(preferences.getInt("play", 0), 0, MAX_PLAY_HEARTS);
    fullnessLevel = clampMeter(preferences.getInt("full", 0), 0, MAX_FULLNESS);
    energyLevel = clampMeter(preferences.getInt("energy", 0), 0, MAX_ENERGY);

    pedometer.setStepCount(stepCount);
    pedometer.setHappiness(playHearts);

    unsigned long now = millis();
    lastFullnessDecay = now;
    lastEnergyDecay = now;
    lastPlayDecay = now;
}

static void savePetStateIfChanged() {
    static int savedStepCount = -1;
    static int savedPlayHearts = -1;
    static int savedFullness = -1;
    static int savedEnergy = -1;

    if (stepCount != savedStepCount) {
        preferences.putInt("steps", stepCount);
        savedStepCount = stepCount;
    }
    if (playHearts != savedPlayHearts) {
        preferences.putInt("play", playHearts);
        savedPlayHearts = playHearts;
    }
    if (fullnessLevel != savedFullness) {
        preferences.putInt("full", fullnessLevel);
        savedFullness = fullnessLevel;
    }
    if (energyLevel != savedEnergy) {
        preferences.putInt("energy", energyLevel);
        savedEnergy = energyLevel;
    }
}

static void updateOverallHappiness() {
    int fullMeters = 0;
    if (playHearts >= MAX_PLAY_HEARTS) fullMeters++;
    if (fullnessLevel >= MAX_FULLNESS) fullMeters++;
    if (energyLevel >= MAX_ENERGY) fullMeters++;
    happiness = fullMeters;
}

static void decayPetMetersIfNeeded() {
    unsigned long now = millis();

    while (now - lastFullnessDecay >= FULLNESS_DECAY_INTERVAL_MS) {
        fullnessLevel = decayMeter(fullnessLevel);
        lastFullnessDecay += FULLNESS_DECAY_INTERVAL_MS;
    }

    while (now - lastPlayDecay >= PLAY_DECAY_INTERVAL_MS) {
        playHearts = decayMeter(playHearts);
        pedometer.setHappiness(playHearts);
        lastPlayDecay += PLAY_DECAY_INTERVAL_MS;
    }

    if (!isSleeping) {
        while (now - lastEnergyDecay >= ENERGY_DECAY_INTERVAL_MS) {
            energyLevel = decayMeter(energyLevel);
            lastEnergyDecay += ENERGY_DECAY_INTERVAL_MS;
        }
    } else {
        lastEnergyDecay = now;
    }
}

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

    // Calibrate pedometer baseline
    Serial.println("Calibrating pedometer - stand still for 3 seconds...");
    delay(3000);
    
    float sum = 0;
    for (int i = 0; i < 100; i++) {
        float ax, ay, az, gx, gy, gz;
        imuRead(ax, ay, az, gx, gy, gz);
        float l1 = abs(ax) + abs(ay) + abs(az);
        sum += l1;
        delay(10);
    }
    float calibratedBaseline = sum / 100;
    Serial.print("Calibrated baseline: ");
    Serial.println(calibratedBaseline);
    
    pedometer.begin();
    pedometer.setBaseline(calibratedBaseline);
    loadPetState();

    // shake detector
    shakeDetector.setThreshold(25.0); //delta value set from data recorded based on hand shake
    shakeDetector.setCooldown(2000);
    shakeDetector.setSamplesRequired(3);

    display_init();
    setup_buttons();
    
    // WiFi Connect
    WiFi.mode(WIFI_STA);
    printAvailableNetworks();
    Serial.print("Connecting to WiFi SSID: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    unsigned long wifiStart = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < WIFI_CONNECT_TIMEOUT_MS) {
        delay(500);
        Serial.print("WiFi status: ");
        Serial.println(wifiStatusName(WiFi.status()));
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        configTime(-8 * 3600, 3600, "pool.ntp.org");
        fetchTemperature();
    } else {
        Serial.print("\nWiFi failed, continuing offline. Final status: ");
        Serial.println(wifiStatusName(WiFi.status()));
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
        Serial.print("Retrying WiFi. Current status: ");
        Serial.println(wifiStatusName(WiFi.status()));
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


    // shake feature for FEED MODE (printing delta for debugging)
    if (feedMode) {
    // Calculate delta for debugging
    static float prevAx, prevAy, prevAz;
    static bool first = true;
    if (!first) {
        float delta = sqrt(pow(ax - prevAx, 2) + pow(ay - prevAy, 2) + pow(az - prevAz, 2));
        if (delta > 2.0) {  // Only print when something happens
            Serial.print("Delta: ");
            Serial.println(delta);
        }
    }
    prevAx = ax; prevAy = ay; prevAz = az;
    first = false;
}

    // Add this to see raw acceleration when in play mode (for debugging pedometer)
    if (playMode) {
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint > 2000) {
            lastPrint = millis();
            Serial.printf("Raw - ax: %.2f, ay: %.2f, az: %.2f\n", ax, ay, az);
        }
    }
    
    // PEDOMETER - Only in PLAY mode
    if (playMode) {
        if (pedometer.update(ax, ay, az)) {
            stepCount = pedometer.getStepCount();
            playHearts = pedometer.getHappiness();
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
            }
            lastShakeTime = millis();
            Serial.println("SHAKE DETECTED! YUM!");
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

    decayPetMetersIfNeeded();
    updateOverallHappiness();
    savePetStateIfChanged();
    
    // UI
    check_buttons();
    display_Home();
    delay(50);
}
