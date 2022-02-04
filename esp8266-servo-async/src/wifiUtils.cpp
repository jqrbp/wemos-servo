#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include "libs/ESPAsyncTCP/ESPAsyncTCP.h"
#endif

#include "configUtils.h"

void wifiInit() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(configGet()->hostname);
  WiFi.begin(configGet()->wifissid, configGet()->wifipassword);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("\nSTA: Failed!\n");
    WiFi.disconnect(true);
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}
