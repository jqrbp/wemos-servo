#include <EEPROM.h>
#include "configUtils.h"

#define CONFIG_START 8        // position in EEPROM where our first byte gets written
#define CONFIG_VERSION "001" // version string to let us compare current to whatever is in EEPROM

configuration_t CONFIGURATION = {
  CONFIG_VERSION,
  "esp8266servo",
  "wifiAPName",
  "wifiAPPassword"
};

bool EEPROMStartedFlag = false;

configuration_t* configGet() {
  return &CONFIGURATION;
}

void EEPROMStart() {
  if (!EEPROMStartedFlag) {
    EEPROM.begin(sizeof(configuration_t) + CONFIG_START);
    EEPROMStartedFlag = true;
  }
}

void EEPROMEnd() {
  if (EEPROMStartedFlag) {
    EEPROM.end();
    EEPROMStartedFlag = false;
  }
}

void configSave() {
  EEPROMStart();
  EEPROM.put(CONFIG_START, CONFIGURATION);
  EEPROM.commit();
  EEPROMEnd();
}

int configLoad() {
  // validate its the correct version (and therefore the same struct...)
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2]) {
    // and if so read configuration into struct
    EEPROM.get(CONFIG_START, CONFIGURATION);
    return 1;
  } else {
    configSave();
  }
  return 0;
}

void configInit() {
  EEPROMStart();
  configLoad();
  EEPROMEnd();
}
