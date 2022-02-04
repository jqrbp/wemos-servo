#include <Servo.h>
#include <string.h>
#include "webSocketsUtils.h"
#include "servoUtils.h"

#define SERVO_PIN 5

Servo myservo;

static servo_params_t SERVOPARAMS = {
    500,
    2400,
    10,
    340,
    20
};

int curPosition = 20;
bool newPosFlag = false;
bool servoSetFlag = false;

unsigned char servoBuf[10];

void servoInit() {
    pinMode(SERVO_PIN, OUTPUT);
    myservo.attach(SERVO_PIN, SERVOPARAMS.pulsemin, SERVOPARAMS.pulsemax);
}

servo_params_t* servoParamsGet() {
    return &SERVOPARAMS;
}

void servoSetPositionInChars(char *chrs) {
    servoSetFlag = true;
    int valin = atoi(chrs);

    if (valin < SERVOPARAMS.valuemin) {
        valin = SERVOPARAMS.valuemin;
    } 
    
    if (valin > SERVOPARAMS.valuemax) {
        valin = SERVOPARAMS.valuemax;
    }

    SERVOPARAMS.valuein = valin;

    newPosFlag = true;
    servoSetFlag = false;
}

void servoSetPulse(int min, int max, int vmin, int vmax) {
    servoSetFlag = true;
    SERVOPARAMS.pulsemin = min;
    SERVOPARAMS.pulsemax = max;
    SERVOPARAMS.valuemin = vmin;
    SERVOPARAMS.valuemax = vmax;
    Serial.print("Setting Servo Pulse:");Serial.print(SERVOPARAMS.pulsemin, DEC);Serial.print(" - ");Serial.println(SERVOPARAMS.pulsemax, DEC);
    Serial.print("Setting Servo Range:");Serial.print(SERVOPARAMS.valuemin, DEC);Serial.print(" - ");Serial.println(SERVOPARAMS.valuemax, DEC);
    myservo.detach();
    myservo.attach(SERVO_PIN, SERVOPARAMS.pulsemin, SERVOPARAMS.pulsemax);
    servoSetFlag = false;
}

void servoLoop() {
    if (servoSetFlag) return;

    curPosition = (curPosition * 7 + SERVOPARAMS.valuein * 1) / 8;
    if (newPosFlag) {
        memcpy(servoBuf,&curPosition,2);
        memcpy(servoBuf + 2, &SERVOPARAMS.valuein,2);
        memcpy(servoBuf + 4, &SERVOPARAMS.valuemin, 2);
        memcpy(servoBuf + 6, &SERVOPARAMS.valuemax, 2);
        webSocketBroadcastBin(servoBuf,8);
        newPosFlag = false;
    }
    myservo.write(curPosition);
}
