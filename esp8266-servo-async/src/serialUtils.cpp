#include <Arduino.h>
#include "configUtils.h"
#include "webSocketsUtils.h"

#define MAX_BUF_LEN 100
unsigned char serial_idx = 0;
char serial_buf[MAX_BUF_LEN];
unsigned long serial_time = 0;

void set_wifi_access_point(char *buf) {
    bool ok_flag = false;
    bool pass_flag = false;
    // sample buf: {"w":"wifiaccesspoint","p":"wifipassword"}
    unsigned char idx = 6;
    String ssid = "";
    String password = "";
    while (buf[idx] != 0 && idx < MAX_BUF_LEN) {
        if (buf[idx] == '"') {
            if (MAX_BUF_LEN - idx) {
                if (buf[idx+1] == '}') {
                    ok_flag = true;
                    break;
                }
                if (MAX_BUF_LEN - idx > 8) {
                    if (buf[idx+3] = 'p') {
                        pass_flag = true;
                        idx+=7;
                    }
                } else {
                    break;
                }
            }
        }
        
        if (pass_flag) {
            password += buf[idx];
        } else {
            ssid += buf[idx];
        }

        idx++;
    }
    if (ok_flag) {
        Serial.println("Setting wifi ssid:");
        Serial.println(ssid);
        Serial.println(password);
        
        snprintf(configGet()->wifissid, 20, "%s",ssid.c_str());
        snprintf(configGet()->wifipassword, 20, "%s",password.c_str());

        Serial.println("restarting");
        configSave();
        ESP.restart();
    }
}

void parse_buf(char *buf) {
    switch(buf[2]) {
        case 'w':
            set_wifi_access_point(buf);
            break;
        default:
            break;
    }
}

void serialInit() {
    Serial.begin(115200);
    Serial.println("Serial started!\r\n");
}

void serialLoop() {
    if (webSocketIsConnected()) return;

    if (Serial.available()) {
        char c = Serial.read();
        if (c == '{') {
            serial_idx = 0;
            serial_time = millis();
        }
        serial_buf[serial_idx] = c;
        serial_idx++;
        if (c == '}' && serial_idx > 0) {
            parse_buf(serial_buf);
            serial_idx = 0;
            serial_buf[0] = 0;
        }
    }

    if (serial_idx > 0 && (millis() - serial_time > 2000)) {
        Serial.println("serial timeout");
        serial_buf[0] = 0;
        serial_idx = 0;
    }
}
