#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <TM1637Display.h>
#include <DHT.h>

#ifndef STASSID
#define STASSID "HUAWEI-104AU3"
#define STAPSK "321222lee"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

unsigned long lastNtpSync = 0;
const unsigned long ntpSyncInterval = 3600000;  // 每小时同步一次时间

unsigned long lastSensorUpdate = 0;
const unsigned long sensorInterval = 5000;  // 每5秒更新一次传感器数据

// TM1637 配置
#define CLK_PIN_TEMP 13
#define DIO_PIN_TEMP 16
TM1637Display tm1637Temp(CLK_PIN_TEMP, DIO_PIN_TEMP);

#define CLK_PIN_HUMID 5
#define DIO_PIN_HUMID 2
TM1637Display tm1637Humid(CLK_PIN_HUMID, DIO_PIN_HUMID);

#define CLK_PIN_TIME 12
#define DIO_PIN_TIME 14
TM1637Display tm1637Time(CLK_PIN_TIME, DIO_PIN_TIME);

// DHT22 配置
#define DHT_PIN 4
DHT dht(DHT_PIN, DHT22);

#define LDR_PIN A0

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // 限制 WiFi 连接时间，15 秒超时
  unsigned long wifiStartTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStartTime < 15000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    ESP.restart();
  }

  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);  // 进入 WiFi 轻度休眠模式

  // 初始化显示屏
  tm1637Temp.setBrightness(4);
  tm1637Humid.setBrightness(4);
  tm1637Time.setBrightness(4);

  tm1637Time.clear();
  tm1637Humid.clear();
  tm1637Temp.clear();

  pinMode(LDR_PIN, INPUT);
  dht.begin();
  
  // 仅在启动时同步一次时间
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  lastNtpSync = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  ArduinoOTA.handle();  // OTA 仍然保持不变

  // 每小时同步一次时间
  if (currentMillis - lastNtpSync >= ntpSyncInterval) {
    Serial.println("Syncing time with NTP...");
    configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    lastNtpSync = currentMillis;
  }

  // 每 5 秒更新一次传感器数据
  if (currentMillis - lastSensorUpdate >= sensorInterval) {
    lastSensorUpdate = currentMillis;
    updateSensors();
  }

  displayTime();
}

void displayTime() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    int hour = timeinfo.tm_hour;
    int minute = timeinfo.tm_min;
    tm1637Time.showNumberDecEx(hour * 100 + minute, 0b11100000, true);
  }
}

void updateSensors() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (!isnan(temperature)) {
    int temp = (int)(temperature * 100);
    tm1637Temp.showNumberDecEx(temp, 0b11100000);
  }

  if (!isnan(humidity)) {
    int hum = (int)(humidity * 100);
    tm1637Humid.showNumberDecEx(hum, 0b11100000);
  }

  adjustBrightness();
}

void adjustBrightness() {
  int ldrValue = analogRead(LDR_PIN);
  int brightness = map(ldrValue, 1023, 0, 1, 5);
  tm1637Temp.setBrightness(brightness);
  tm1637Humid.setBrightness(brightness);
  tm1637Time.setBrightness(brightness);
}
