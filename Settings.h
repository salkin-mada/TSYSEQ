#ifndef SETTINGS_H
#define SETTINGS_H

#include <SPI.h>
#include <SD.h>
// #include <EEPROM.h>

extern bool doStep[5][16];
extern int flagAbuttonWasPressed;

extern unsigned int trackStepIterationTrigDevider[5];
extern bool trackMuteLatch[5];
extern int selectedTrack;
extern int devisionValue;
extern int interruptCountToStepResetValue;
extern bool poolDirection;

void SD_init();
void SD_checkTypeAndListRoot();
void SD_writeSettings(unsigned int i);
void SD_readSettings();
void SD_readAllSettings2Monitor();
void applySetting(String settingName, String settingValue);
//void SD_write(unsigned int i);
//void SD_read(unsigned int i);
//void WRITE2EEPROM(unsigned int i);

#endif