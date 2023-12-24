#include <Wire.h>
#include <ESP8266WiFi.h>
#include <TM1637Display.h>
#include <DHT.h>

// WiFi配置
const char *ssid = "HUAWEI-104AU3";
const char *password = "321222lee";

// TM1637配置
#define CLK_PIN_TEMP 13  // 连接到TM1637的CLK引脚（温度）
#define DIO_PIN_TEMP 16  // 连接到TM1637的DIO引脚（温度）
TM1637Display tm1637Temp(CLK_PIN_TEMP, DIO_PIN_TEMP);

#define CLK_PIN_HUMID 5  // 连接到TM1637的CLK引脚（湿度）
#define DIO_PIN_HUMID 2  // 连接到TM1637的DIO引脚（湿度）
TM1637Display tm1637Humid(CLK_PIN_HUMID, DIO_PIN_HUMID);

#define CLK_PIN_TIME 12  // 连接到TM1637的CLK引脚（时间）
#define DIO_PIN_TIME 14  // 连接到TM1637的DIO引脚（时间）
TM1637Display tm1637Time(CLK_PIN_TIME, DIO_PIN_TIME);

// DHT22配置
#define DHT_PIN 4
DHT dht(DHT_PIN, DHT22);

#define LDR_PIN A0

void setup() {
  Serial.begin(115200);

  // 连接到WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // 初始化TM1637
  tm1637Temp.setBrightness(4);
  tm1637Humid.setBrightness(4);
  tm1637Time.setBrightness(4);

  tm1637Time.clear();
  tm1637Humid.clear();
  tm1637Temp.clear();

  pinMode(LDR_PIN, INPUT);
  dht.begin();
}

void loop() {
  Time();
  Temp();
  SetBri();
}

void Time() {
  // 获取网络时间
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  // 显示时间在TM1637上，点亮冒号
  int hour = timeinfo.tm_hour;
  int minute = timeinfo.tm_min;
  tm1637Time.showNumberDecEx(hour * 100 + minute, 0b11100000, true); // 第二个参数表示是否点亮冒号
  //delay(1000);
}


void Temp() {
  // 获取温湿度
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  // 显示温度和湿度在TM1637上
  //Serial.println(temperature);
  int temp = (int)(temperature * 100);
  int dots1 = 0b00010000; // 0x10
  tm1637Temp.showNumberDecEx(temp, 0b11100000);
  //delay(1000);

  //Serial.println(humidity);
  int hum = (int)(humidity * 100);
  int dots2 = 0b00010000; // 0x10
  tm1637Humid.showNumberDecEx(hum, 0b11100000);
  delay(300000);
}

void SetBri() {
  int ldrValue = analogRead(LDR_PIN);
  //Serial.println(ldrValue);

  // 根据亮度调整TM1637的亮度
  int brightness = map(ldrValue, 1023, 0, 1, 4);
  tm1637Temp.setBrightness(brightness);
  tm1637Humid.setBrightness(brightness);
  tm1637Time.setBrightness(brightness);
  //Serial.println(brightness);
  delay(60000);
}
