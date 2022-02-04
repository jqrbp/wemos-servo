# wemos-servo
Servo Control using Wemos D1 R1 board (esp8266) Arduino via websocket with an embedded webpage.

## Libraries
1. Web Server: [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
2. Async TCP (required by the web server): [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP)
3. Websocket Server: [arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets)
4. Embedded Web Page Interface / Framework: [materializecss](https://materializecss.com)

## Notes
- Arduino file, esp8266-servo-async.ino, can be compiled and uploaded with all source codes and libraries are already included in src folder.
- Source codes for the embedded web page can be found in html folder. The sources are minimized and compressed (gzip) by using a minifier that is written in golang.

## Demo
- Board: WEMOS D1 R1
- Servo: FUTABA S3003
- Video: [wemos-servo](https://youtu.be/KA7A3oNB134)