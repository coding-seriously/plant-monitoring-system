#include <DHTesp.h>

#include <WiFi.h>
#include <WebServer.h>
#include <DHTesp.h>                // ✅ BeeGee Tokyo DHT Library
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// ===== WiFi Configuration =====
const char* ssid = "CHENAB";
const char* password = "44zMf3QqdU&KC3Mv";

// ===== Sensor Pins =====
#define DHTPIN 4
#define SOIL_PIN 36      // VP
#define LDR_PIN 39       // VN
#define RELAY_PIN 27

// ===== DHT Sensor Type =====
#define DHTTYPE DHTesp::DHT11     // ✅ Adjust if using DHT22 or DHT11

// ===== I2C LCD =====
#define I2C_ADDR 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_ROWS);

// ===== Sensor Objects =====
DHTesp dht;
WebServer server(80);

// ===== Calibration & Control Parameters =====
const int SOIL_DRY = 3500;   // Value when sensor is in dry soil
const int SOIL_WET = 1000;   // Value when sensor is in water
const int soilThresholdPercent = 30; // Pump turns on if below this %
const int pumpDurationSec = 10;
const int minPumpIntervalMin = 30;
const int maxPumpRunsPerDay = 20;

// ===== LDR Calibration =====
const int LDR_DARK = 500;     // Adjust after testing in dark
const int LDR_BRIGHT = 3000;  // Adjust after testing in bright light

// ===== Variables =====
bool pumpState = false;
unsigned long pumpOnTime = 0;
unsigned long lastWaterTime = 0;
int pumpRunsToday = 0;

// Manual override
bool manualOverride = false;
unsigned long manualOverrideTime = 0;
const unsigned long overrideDuration = 15000; // 15 sec

// ===== Helper Functions =====

// Convert raw soil reading to percent (0 = dry, 100 = wet)
float soilRawToPercent(int raw) {
  int r = constrain(raw, SOIL_WET, SOIL_DRY);
  float pct = 100.0 * (float)(SOIL_DRY - r) / (float)(SOIL_DRY - SOIL_WET);
  return constrain(pct, 0.0, 100.0);
}

// Average soil readings for stability
float readSoilPercent() {
  const int samples = 5;
  long total = 0;
  for (int i = 0; i < samples; i++) {
    total += analogRead(SOIL_PIN);
    delay(10);
  }
  int avg = total / samples;
  return soilRawToPercent(avg);
}

// Convert raw LDR reading to light percentage (0 = dark, 100 = bright)
float ldrRawToPercent(int raw) {
  int r = constrain(raw, LDR_DARK, LDR_BRIGHT);
  float pct = 100.0 * (float)(r - LDR_DARK) / (float)(LDR_BRIGHT - LDR_DARK);
  return constrain(pct, 0.0, 100.0);
}

// Average LDR readings for stability
float readLightPercent() {
  const int samples = 5;
  long total = 0;
  for (int i = 0; i < samples; i++) {
    total += analogRead(LDR_PIN);
    delay(10);
  }
  int avg = total / samples;
  return ldrRawToPercent(avg);
}

// Pump control
void setPump(bool on) {
  digitalWrite(RELAY_PIN, on ? LOW : HIGH);  // Active-Low relay
  pumpState = on;
  if (on) {
    pumpOnTime = millis();
    pumpRunsToday++;
    lastWaterTime = millis();
  }
}

// ===== Web Handlers =====
void handleRoot() {
  String html = "<html><body><h2>ESP32-S Plant Monitor</h2>";
  html += "<p><a href='/json'>View JSON Data</a><br>";
  html += "<a href='/toggle'>Toggle Pump</a></p></body></html>";
  server.send(200, "text/html", html);
}

void handleJSON() {
  TempAndHumidity dhtData = dht.getTempAndHumidity();
  float temp = dhtData.temperature ;
  float hum  = dhtData.humidity ;
  float soilPct = readSoilPercent();
  float lightPct = readLightPercent();

  String json = "{";
  json += "\"temperature\":" + String(temp, 1) + ",";
  json += "\"humidity\":" + String(hum, 1) + ",";
  json += "\"soil\":" + String(soilPct, 1) + ",";
  json += "\"light\":" + String(lightPct, 1) + ",";
  json += "\"pump\":" + String(pumpState ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void handleToggle() {
  Serial.println("Toggle request received");
  manualOverride = true;
  manualOverrideTime = millis();
  setPump(!pumpState);
  Serial.print("New Pump State: ");
  Serial.println(pumpState ? "ON" : "OFF");
  server.send(200, "text/plain", pumpState ? "Pump ON" : "Pump OFF");
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  Wire.begin();

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Plant Monitor");
  lcd.setCursor(0,1);
  lcd.print("Connecting WiFi...");

  // Relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // OFF at boot

  // DHT
  dht.setup(DHTPIN, DHTTYPE);
  Serial.println("DHT sensor initialized.");

  // WiFi
  WiFi.begin(ssid, password);
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(500);
    Serial.print(".");
    attempt++;
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected: " + WiFi.localIP().toString());
    lcd.print("WiFi OK IP:");
    lcd.setCursor(0,1);
    lcd.print(WiFi.localIP());
  } else {
    WiFi.softAP("ESP32S_PlantMonitor");
    Serial.println("\nAP Mode: " + WiFi.softAPIP().toString());
    lcd.print("AP Mode Started");
  }

  // Web server
  server.on("/", handleRoot);
  server.on("/json", handleJSON);
  server.on("/toggle", handleToggle);
  server.begin();
  delay(2000);
  lcd.clear();
}

// ===== Loop =====
void loop() {
  server.handleClient();

  // === 1-second loop interval ===
  delay(1000);

  // Read sensors
  TempAndHumidity dhtData = dht.getTempAndHumidity();
  float temp = !isnan(dhtData.temperature) ? dhtData.temperature : 0;
  float hum  = !isnan(dhtData.humidity) ? dhtData.humidity : 0;
  float soilPct = readSoilPercent();
  float lightPct = readLightPercent();

  Serial.printf("T=%.1fC H=%.1f%% Soil=%.1f%% Light=%.1f%% Pump=%s\n",
                temp, hum, soilPct, lightPct, pumpState ? "ON" : "OFF");

  // === Reset LCD each loop ===
  lcd.noDisplay();
  delay(10); // brief pause before reset
  lcd.display();
  lcd.clear();

  // LCD Line 1: Temperature & Humidity
  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.print(temp,1);
  lcd.print("C H:");
  lcd.print(hum,0);
  lcd.print("%");

  // LCD Line 2: Soil, Light, Pump
  lcd.setCursor(0,1);
  lcd.print("S:");
  lcd.print(soilPct,0);
  lcd.print("% ");
  lcd.print(lightPct < 30 ? "Dark" : "Light");
  lcd.print(" ");
  lcd.print(pumpState ? "ON" : "OFF");

  // === Auto-watering logic ===
  if (!pumpState && soilPct <= soilThresholdPercent) {
    unsigned long sinceLast = millis() - lastWaterTime;
    if (!manualOverride &&
        sinceLast > (unsigned long)minPumpIntervalMin * 60000UL &&
        pumpRunsToday < maxPumpRunsPerDay) {
      Serial.println("Soil dry -> Pump ON");
      setPump(true);
    }
  }

  // Turn off pump after duration
  if (pumpState && millis() - pumpOnTime >= pumpDurationSec * 1000UL) {
    setPump(false);
    Serial.println("Pump OFF after duration");
  }

  // Reset manual override
  if (manualOverride && millis() - manualOverrideTime > overrideDuration) {
    manualOverride = false;
  }
}
