#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include "time.h"
#include <WebServer.h>  // built-in ESP32 library

Adafruit_BMP280 bmp;  // I2C interface

// Replace with your WiFi credentials (kept out of the repository)
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// NTP server and offset
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800; // GMT+5:30 (India)
const int daylightOffset_sec = 0;

// Web server on port 80
WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Mount SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println(" SPIFFS Mount Failed!");
    while (1);
  }
  Serial.println(" SPIFFS Mounted.");

  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n WiFi connected.");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  // Init time via NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time synchronized via NTP.");

  // Initialize I2C explicitly
  Wire.begin(21, 22);

  // Initialize BMP280
  if (!bmp.begin(0x76)) {
    Serial.println(" BMP280 not found. Check wiring!");
    while (1);
  }
  Serial.println("BMP280 detected!");

  // Create CSV if it doesn't exist
  if (!SPIFFS.exists("/bmp280.csv")) {
    File file = SPIFFS.open("/bmp280.csv", FILE_WRITE);
    if (file) {
      file.println("timestamp,temperature_C,pressure_hPa");
      file.close();
      Serial.println(" Created bmp280.csv with header.");
    } else {
      Serial.println("Failed to create file.");
    }
  }

  // Setup web server route
  server.on("/bmp280.csv", HTTP_GET, []() {
    File file = SPIFFS.open("/bmp280.csv", "r");
    if (!file) {
      server.send(404, "text/plain", "File not found");
      return;
    }

    // Tell browser this is a CSV file for download
    server.streamFile(file, "text/csv");
    file.close();
  });

  server.begin();
  Serial.println("Web server started. Access file at: http://" + WiFi.localIP().toString() + "/bmp280.csv");
}

void loop() {
  // Handle web requests
  server.handleClient();

  // Read sensor values
  float t = bmp.readTemperature();
  float p = bmp.readPressure() / 100.0F;

  // Get formatted time
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    delay(10000);
    return;
  }

  char timeString[30];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);

  // Append to CSV
  File file = SPIFFS.open("/bmp280.csv", FILE_APPEND);
  if (file) {
    file.printf("%s,%.2f,%.2f\n", timeString, t, p);
    file.close();
    Serial.printf("Logged -> %s | T: %.2f C | P: %.2f hPa\n", timeString, t, p);
  } else {
    Serial.println(" Failed to open bmp280.csv for writing!");
  }

  delay(10000); // log every 10 seconds
}
