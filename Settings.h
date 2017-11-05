#ifndef SETTINGS_H
#define SETTINGS_H

#include <SPI.h>
#include <SD.h>
// #include <EEPROM.h>
extern int exINT;
extern float exFloat;
extern boolean exBoolean;
extern long exLong;

extern bool doStep[];
extern int flagAbuttonHaveBeenPressed;

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