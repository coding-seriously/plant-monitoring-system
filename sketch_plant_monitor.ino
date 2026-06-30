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
const int SOIL_DRY = 3500;
const int SOIL_WET = 1000;
const int soilThresholdPercent = 30;
const int pumpDurationSec = 10;
const int minPumpIntervalMin = 30;
const int maxPumpRunsPerDay = 20;

// ===== Variables =====
bool pumpState = false;
unsigned long pumpOnTime = 0;
unsigned long lastWaterTime = 0;
int pumpRunsToday = 0;

// Manual override for toggle
bool manualOverride = false;
unsigned long manualOverrideTime = 0;
const unsigned long overrideDuration = 15000; // 15 seconds

// For smooth LCD updates
float lastTemp = -1000, lastHum = -1000, lastSoil = -1;
int lastLDR = -1;
bool lastPumpState = false;

// ===== Helper Functions =====
float soilRawToPercent(int raw) {
  int r = constrain(raw, SOIL_WET, SOIL_DRY);
  float pct = 100.0 * (float)(r - SOIL_WET) / (float)(SOIL_DRY - SOIL_WET);
  return constrain(pct, 0.0, 100.0);
}

void setPump(bool on) {
  digitalWrite(RELAY_PIN, on ? LOW : HIGH);  // Active-Low
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
  float temp = !isnan(dhtData.temperature) ? dhtData.temperature : lastTemp;
  float hum  = !isnan(dhtData.humidity) ? dhtData.humidity : lastHum;
  float soilPct = soilRawToPercent(analogRead(SOIL_PIN));
  int ldrVal = analogRead(LDR_PIN);

  String json = "{";
  json += "\"temperature\":" + String(temp, 1) + ",";
  json += "\"humidity\":" + String(hum, 1) + ",";
  json += "\"soil\":" + String(soilPct, 1) + ",";
  json += "\"ldr\":" + String(ldrVal) + ",";
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
  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Plant Monitor");
  lcd.setCursor(0,1);
  lcd.print("Connecting WiFi...");

  // Relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // OFF at boot

  // DHT Sensor
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

  static unsigned long lastRead = 0;
  if (millis() - lastRead > 3000) {
    lastRead = millis();

    // Read all sensors
    TempAndHumidity dhtData = dht.getTempAndHumidity();
    float temp = !isnan(dhtData.temperature) ? dhtData.temperature : lastTemp;
    float hum  = !isnan(dhtData.humidity) ? dhtData.humidity : lastHum;
    float soilPct = soilRawToPercent(analogRead(SOIL_PIN));
    int ldrVal = analogRead(LDR_PIN);

    Serial.printf("T=%.1fC H=%.1f%% Soil=%.1f%% LDR=%d Pump=%s\n",
                  temp, hum, soilPct, ldrVal, pumpState?"ON":"OFF");

    // LCD update
    if (temp != lastTemp || hum != lastHum) {
      lcd.setCursor(0,0);
      lcd.print("T:");
      lcd.print(temp,1);
      lcd.print("C H:");
      lcd.print(hum,0);
      lcd.print("%  ");
      lastTemp = temp;
      lastHum = hum;
    }

    if (soilPct != lastSoil || ldrVal != lastLDR || pumpState != lastPumpState) {
      lcd.setCursor(0,1);
      lcd.print("S:");
      lcd.print(soilPct,0);
      lcd.print("% L:");
      lcd.print(ldrVal);
      lcd.print(pumpState?"ON ":"OFF");
      lastSoil = soilPct;
      lastLDR = ldrVal;
      lastPumpState = pumpState;
    }

    // Auto-watering logic
    if (!pumpState && soilPct >= soilThresholdPercent) {
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
}
