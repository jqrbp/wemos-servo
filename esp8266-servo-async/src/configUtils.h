#ifndef __CONFIG_UTILS_H__
#define __CONFIG_UTILS_H__

typedef struct {
  char version[4];
  char hostname[20];
  char wifissid[20];
  char wifipassword[20];
} configuration_t;

configuration_t *configGet();
void configInit();
void configSave();
#endif
