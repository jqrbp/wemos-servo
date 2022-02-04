//
// A simple server implementation showing how to:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s
//

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include "libs/ESPAsyncTCP/ESPAsyncTCP.h"
#endif
#include "libs/ESPAsyncWebServer/ESPAsyncWebServer.h"

#include "webSocketsUtils.h"
#include "configUtils.h"
#include "servoUtils.h"

#include "html_h/main_html_min_gz.h"
#include "html_h/main_js_min_gz.h"
#include "html_h/main_css_min_gz.h"
#include "html_h/wshandler_js_min_gz.h"
#include "html_h/favicon_png_min_gz.h"
#include "html_h/materialize_min_css_min_gz.h"
#include "html_h/materialize_min_js_min_gz.h"

AsyncWebServer server(8080);

static const char GZIP[] PROGMEM ="gzip";
static const char CONTENT_ENC[] PROGMEM = "Content-Encoding";
static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char TEXT_HTML[] PROGMEM = "text/html";
static const char TEXT_CSS[] PROGMEM = "text/css";
static const char APP_JSON[] PROGMEM = "application/json";
static const char APP_JS[] PROGMEM = "application/javascript";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

void handleNotFound(AsyncWebServerRequest *request) {
  String message = "Handle Not Found\n\n";
  message += "URL: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";
  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  request->send(404, "text/plain", message);
}

void handleGzChars(AsyncWebServerRequest *request, const char *type, const unsigned char *contents, unsigned int len) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, type, contents, len);
    response->addHeader(FPSTR(CONTENT_ENC), FPSTR(GZIP));
    request->send(response);
}

void handleSaveConfig(AsyncWebServerRequest *request) {
  configuration_t *curConfig = configGet();
  String ssid;
  String password;
  String hostname;
  if (request->hasParam("ssid", true) && request->hasParam("password", true) && request->hasParam("hostname", true)) {
      ssid = request->getParam("ssid", true)->value();
      password = request->getParam("password", true)->value();
      hostname = request->getParam("hostname", true)->value();
      snprintf(curConfig->wifissid, 20, "%s",ssid.c_str());
      snprintf(curConfig->wifipassword, 20, "%s",password.c_str());
      snprintf(curConfig->hostname, 20, "%s",hostname.c_str());
      request->send(200, "text/plain", "Hello, POST: Changing Configuration. Restarting...");
      configSave();
      ESP.restart();
  } else {
      request->send(200, "text/plain", "Hello, POST: Failed to save config");
  }
}

void handleSetPulse(AsyncWebServerRequest *request) {
  int pulse_min, pulse_max, value_min, value_max;
  String min_str;
  String max_str;
  String vmin_str;
  String vmax_str;
  String out_str;
  if (request->hasParam("min", true) && 
        request->hasParam("max", true) &&
        request->hasParam("vmin", true) &&
        request->hasParam("vmax", true)
    ) {
      min_str = request->getParam("min", true)->value();
      max_str = request->getParam("max", true)->value();
      vmin_str = request->getParam("vmin", true)->value();
      vmax_str = request->getParam("vmax", true)->value();
      out_str = "Pulse: " + min_str + "-" + max_str + ", Value: " + vmin_str + " - "+ vmax_str;
      pulse_min = atoi(min_str.c_str());
      pulse_max = atoi(max_str.c_str());
      value_min = atoi(vmin_str.c_str());
      value_max = atoi(vmax_str.c_str());
      servoSetPulse(pulse_min, pulse_max, value_min, value_max);
      request->send(200, "text/plain", "Setting Servo  " + out_str);
  } else {
    request->send(200, "text/plain", "Bad Request");
  }
}

void handleGetPulse(AsyncWebServerRequest *request) {
  char out[100];

  servo_params_t* servoParams = servoParamsGet();
  snprintf(out, 100, "{\"min\":%d,\"max\":%d,\"vmin\":%d,\"vmax\":%d,\"vin\":%d}",
    servoParams->pulsemin,
    servoParams->pulsemax,
    servoParams->valuemin,
    servoParams->valuemax,
    servoParams->valuein);

  request->send(200, "application/json", out);
}

void webInit(void) {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        handleGzChars(request, "text/html", main_html_min_gz, main_html_min_gz_len);
    });
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
        handleGzChars(request, "image/png", favicon_png_min_gz, favicon_png_min_gz_len);
    });
    server.on("/main.js", HTTP_GET,  [](AsyncWebServerRequest *request){
        handleGzChars(request, "application/javascript", main_js_min_gz, main_js_min_gz_len);
    });
    server.on("/main.css", HTTP_GET,  [](AsyncWebServerRequest *request){
        handleGzChars(request, "text/css", main_css_min_gz, main_css_min_gz_len);
    });
    server.on("/wshandler.js", HTTP_GET,  [](AsyncWebServerRequest *request){
        handleGzChars(request, "application/javascript", wshandler_js_min_gz, wshandler_js_min_gz_len);
    });
    server.on("/materialize.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
        handleGzChars(request, "text/css", materialize_min_css_min_gz, materialize_min_css_min_gz_len);
    });
    server.on("/materialize.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        handleGzChars(request, "application/javascript", materialize_min_js_min_gz, materialize_min_js_min_gz_len);
    });

    server.on("/saveconfig", HTTP_POST, handleSaveConfig);
    server.on("/setpulse", HTTP_POST, handleSetPulse);
    server.on("/getpulse", HTTP_GET, handleGetPulse);

    server.onNotFound(handleNotFound);

    server.begin();
    webSocketInit();
}

void webLoop(void) {
  webSocketLoop();
}
