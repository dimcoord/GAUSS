#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>

#define RX 16
#define TX 17
HardwareSerial neogps(1);
TinyGPSPlus gps;
HTTPClient http;
WiFiClient client;

#define sensorPin 34
#define buttonPin 12
#define sensorPower 14
const char* ssid = "";
const char* password = "";
const char* serverUrl = "";
const char* token = "";
float longitude;
float latitude;
int satValue;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, LOW);
  Serial.begin(115200);
  neogps.begin(9600, SERIAL_8N1, RX, TX);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi network...");
  }
  Serial.println("Connected!");
}

void loop() {
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (neogps.available())
    {
      if (gps.encode(neogps.read()))
      {
        newData = true;
      }
    }
  }

  if (newData)
  {
    newData = false;
    satValue = gps.satellites.value();
    longitude = gps.location.lng();
    latitude = gps.location.lat();
    Serial.println("Sat: ");
    Serial.println(satValue);
    Serial.println("Longitude: ");
    Serial.println(longitude);
    Serial.println("Latitude: ");
    Serial.println(latitude);
  }
  else
  {
    Serial.println("No new data is received.");
  }

  digitalWrite(sensorPower, HIGH);
  int moisture = analogRead(sensorPin);
  digitalWrite(sensorPower, LOW);
  Serial.println(moisture);

  if (moisture != 0 && digitalRead(buttonPin) == LOW){
    DynamicJsonDocument doc(1024);
    doc["long"] = longitude;
    doc["lat"] = latitude;
    doc["tingker"] = moisture;
    doc["token"] = token;
    String json;
    serializeJson(doc, json);
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(json);
    if (httpCode == HTTP_CODE_OK) {
      String response = http.getString();
      Serial.println(response);
    } else {
      Serial.printf("HTTP error: %d\n", httpCode);
    }

    http.end();
  }

delay(500);

}
