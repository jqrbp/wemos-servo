#ifndef __SERVO_UTILS_H__
#define __SERVO_UTILS_H__

typedef struct {
  int pulsemin;
  int pulsemax;
  int valuemin;
  int valuemax;
  int valuein;
} servo_params_t;

servo_params_t *servoParamsGet();
void servoInit();
void servoSetPositionInChars(char *chrs);
void servoSetPulse(int min, int max, int vmin, int vmax);
void servoLoop();
#endif
