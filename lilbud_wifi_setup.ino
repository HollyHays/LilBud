#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Wi-Fi Fo Fum";
const char* password = "sbkbst310";
//const char* ssid = "iPhone(3)";
//const char* password = "Albus1010$";

const char* DEVICE_ID = "lilbud_02";
//hotspot
//const char* SERVER_IP = "172.20.10.9";
//at home
const char* SERVER_IP = "10.0.0.189";// laptop IP; later change to cloud server so it supports different wifis
const int SERVER_PORT = 5000;

void setup() {
  Serial.begin(115200);
  delay(2000);

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String("http://") + SERVER_IP + ":" + SERVER_PORT +
                 "/inbox?device_id=" + DEVICE_ID;

    http.begin(url);
    int code = http.GET();

    if (code == 200) {
      String payload = http.getString();
      Serial.println("DISPLAY:");
      Serial.println(payload);
    }

    http.end();
  }

  delay(3000);
}
