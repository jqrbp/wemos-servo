#include "src/serialUtils.h"
#include "src/wifiUtils.h"
#include "src/webUtils.h"
#include "src/configUtils.h"
#include "src/servoUtils.h"

void setup() {
    serialInit();
    configInit();
    servoInit();
    wifiInit();
    webInit();
}

void loop() {
    serialLoop();
    webLoop();
    servoLoop();
}
