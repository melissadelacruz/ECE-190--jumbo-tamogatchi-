#include "network.h"
#include "../display/display.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

static const char* weatherApiKey = WEATHER_API_KEY;

const char* wifiStatusName(wl_status_t status) {
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


// WiFi credentials
const char* ssid = "Galaxy Z Flip8";
const char* password = "10306090";

static const unsigned long WIFI_CONNECT_TIMEOUT_MS = 10000;
static const unsigned long WIFI_RETRY_INTERVAL_MS = 10000;
unsigned long lastWiFiRetry = 0;
unsigned long lastWeatherFetch = 0;
const unsigned long WEATHER_INTERVAL = 60000;


void networkSetup() {
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
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        configTime(-8 * 3600, 3600, "pool.ntp.org");
        fetchTemperature();
    } else {
        Serial.print("WiFi unavailable; continuing offline: ");
        Serial.println(wifiStatusName(WiFi.status()));
    }
}

void networkUpdate() {
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
    
}
