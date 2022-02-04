#include <Arduino.h>

#include "libs/arduinoWebSockets/WebSocketsServer.h"
#include "servoUtils.h"

WebSocketsServer webSocket = WebSocketsServer(8081);
bool webSocketStarted = false;
bool webSocketConnected = false;

bool webSocketIsConnected(void) {
    return webSocketConnected;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
				
				// send message to client
				webSocket.sendTXT(num, "Connected");
                webSocketConnected = true;
            }
            break;
        case WStype_TEXT:
            servoSetPositionInChars((char*)payload);
            // Serial.printf("[%u] get Text: %s\n", num, payload);
            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            // USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
            // hexdump(payload, length);

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
    }

}

void webSocketBroadcastBin(const unsigned char *payload, unsigned int len) {
    if (webSocketStarted) {
        webSocket.broadcastBIN(payload, len);
    }
}

void webSocketBroadcastTXT(const char *payload) {
    if (webSocketStarted) {
        webSocket.broadcastTXT(payload);
    }
}

void webSocketInit() {
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    webSocketStarted = true;
}

void webSocketLoop() {
    webSocket.loop();
}
