#include "Settings.h"
#include <Arduino.h>

String settingValueArray[8];
bool flagSettingsHaveBeenRead = false;


// converting string to Float
float toFloat(String settingValue){
    char floatbuf[settingValue.length()+1];
    settingValue.toCharArray(floatbuf, sizeof(floatbuf));
    float f = atof(floatbuf);
    return f;
}
// converting string to Long
long toLong(String settingValue){
    char longbuf[settingValue.length()+1];
    settingValue.toCharArray(longbuf, sizeof(longbuf));
    long l = atol(longbuf);
    return l;
}

// Converting String to integer and then to boolean
boolean toBoolean(String settingValue) {
    if(settingValue.toInt()==1){
        return true; // 1 = true
    } else {
        return false; // 0 = false
    }
}

const int chipSelect = BUILTIN_SDCARD;
File recallFile;

/* ################################## INIT ################################## */
/* ################################### SD ################################### */

void SD_init() {
    // Serial.print("Initializing SD card...");
    if (!SD.begin(chipSelect)) {
        // Serial.println("initialization failed!");
        return;
    }
    
    // Serial.println("initialization done.");
    
    /* if (SD.exists("SETTINGS.TXT") == true) {
        if (SD.remove("SETTINGS.TXT") == true); {
            Serial.println("removed SETTINGS.TXT");
        }
    } */
    // make sure settingValueArray is doStep incase not settingsfile exists
    for (int i = 0; i < 8; ++i) {
        settingValueArray[i] = doStep[i];    
    }
    
    delay(200);
}

/* ################################## CHECK ################################## */
/* ################################### ROOT ################################## */

// denne er farlig; bliver ikke lukket ordentligt, og jeg kan derfor ikke skrive til kortet...
void SD_checkTypeAndListRoot() {
    Sd2Card card;
    SdVolume volume;
    SdFile root;
    if (!card.init(SPI_HALF_SPEED, chipSelect)) {
        Serial.println("initialization failed. Things to check:");
        Serial.println("* is a card inserted?");
        Serial.println("* is your wiring correct?");
        Serial.println("* did you change the chipSelect pin to match your shield or module?");
        return;
    } else {
        Serial.println("Wiring is correct and a card is present.");
    }
    delay(200);
    
    // print the type of card
    Serial.print("\nCard type: ");
    switch (card.type()) {
        case SD_CARD_TYPE_SD1:
        Serial.println("SD1");
        break;
        case SD_CARD_TYPE_SD2:
        Serial.println("SD2");
        break;
        case SD_CARD_TYPE_SDHC:
        Serial.println("SDHC");
        break;
        default:
        Serial.println("Unknown");
    }
    delay(200);
    
    // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
    if (!volume.init(card)) {
        Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
        return;
    }
    
    
    // print the type and size of the first FAT-type volume
    uint32_t volumesize;
    Serial.print("\nVolume type is FAT");
    Serial.println(volume.fatType(), DEC);
    Serial.println();
    
    volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
    volumesize *= volume.clusterCount();       // we'll have a lot of clusters
    volumesize *= 512;                            // SD card blocks are always 512 bytes
    Serial.print("Volume size (bytes): ");
    Serial.println(volumesize);
    Serial.print("Volume size (Kbytes): ");
    volumesize /= 1024;
    Serial.println(volumesize);
    Serial.print("Volume size (Mbytes): ");
    volumesize /= 1024;
    Serial.println(volumesize);
    
    
    Serial.println("\nFiles found on the card (name, date and size in bytes): ");
    root.openRoot(volume);
    
    // list all files in the card with date and size
    root.ls(LS_R | LS_DATE | LS_SIZE);
    
}

/* ################################## READ ################################### */
/* ################################ SETTINGS ################################# */

void SD_readSettings(){
    if (SD.exists("settings.txt") == true) {
        char character;
        String settingName;
        String settingValue;
        
        recallFile = SD.open("settings.txt", FILE_READ);
        if (recallFile) {int i = 0;
            while (recallFile.available()) {
                character = recallFile.read();
                while((recallFile.available()) && (character != '[')){
                    character = recallFile.read();
                }
                character = recallFile.read();
                while((recallFile.available()) && (character != '=')){
                    settingName = settingName + character;
                    character = recallFile.read();
                }
                character = recallFile.read();
                while( (recallFile.available()) && (character != ']') && (character != '{') ){
                    settingValue = settingValue + character;
                    character = recallFile.read();
                }
                
                // debug names
                // Serial.print("debuggg names __ ");
                // Serial.println(settingName);
                if (settingName.length() == 0) {
                    // Serial.println("--> halla --> String settingName is empty !");
                } // break , goto close file
                
                if (character == '{') {
                    character = recallFile.read();
                    // Serial.println("i found a {");
                    while(recallFile.available() && (character != '}')) {
                        settingValue = settingValue + character;
                        character = recallFile.read();
                        if (character == ',') {
                            character = recallFile.read();
                            // Serial.println("i found a ,");
                            // Serial.println(settingValue);
                            settingValueArray[i] = settingValue;
                            //reset string
                            settingValue = "";
                            ++i;
                        }
                    }
                    if (character == '}') {
                        character = recallFile.read();
                        // Serial.println("i found a }");
                        // Serial.println(settingValue);
                        settingValueArray[i] = settingValue;
                    }
                }
                
                if(character == ']'){
                    
                    //Debugging
                    /* if (settingName != "doStep") {
                        Serial.print("Name:");
                        Serial.println(settingName);
                        Serial.print("Value :");
                        Serial.println(settingValue);
                    }
                    
                    if (settingName == "doStep") {
                        for (int j = 0; j < 8; ++j) {
                            Serial.print("settingValueArray: ");
                            Serial.println(settingValueArray[j]);
                            Serial.print("doStep: ");
                            Serial.println(doStep[j]);
                            
                        }
                    } */ // debugging end
                    
                    // Apply the value to the parameter
                    flagSettingsHaveBeenRead = true;
                    applySetting(settingName,settingValue);
                    // Reset Strings
                    settingName = "";
                    settingValue = "";
                }
            }
            // close the file:
            recallFile.close();
            // Serial.println("closed file");
        } else {
            // if the file didn't open, print an error:
            Serial.println("error opening settings.txt");
        }
    } else {
        Serial.println("SD_read could not find settings.txt");
    }
}

/* ################################# APPLY ################################### */
/* ################################ SETTINGS ################################# */


void applySetting(String settingName, String settingValue) {
    if (flagSettingsHaveBeenRead == true) {
        if(settingName == "exINT") {
            exINT=settingValue.toInt();
        }
        if(settingName == "exFloat") {
            exFloat=toFloat(settingValue);
        }
        if(settingName == "exBoolean") {
            exBoolean=toBoolean(settingValue);
        }
        if(settingName == "exLong") {
            exLong=toLong(settingValue);
        }
        if(settingName == "doStep") {
            for (unsigned int i = 0; i < 8; ++i) {
                doStep[i] = toBoolean(settingValueArray[i]);
            }
            Serial.println("after apllying toBoolean(settingValueArray[i]) --> doStep[i]");
            for (int j = 0; j < 8; ++j) {
                Serial.print("settingValueArray: ");
                Serial.println(settingValueArray[j]);
                Serial.print("doStep: ");
                Serial.println(doStep[j]);
            }
            /* Apply the value to the parameter by searching for the parameter name
            Using String.toInt(); for Integers
            toFloat(string); for Float
            toBoolean(string); for Boolean
            toLong(string); for Long
            */
            Serial.print("applied ");
            Serial.print(settingName);
            Serial.print(" ");
            Serial.println("setting");
        }
    }
}

/* ################################## WRITE ################################## */
/* ################################ SETTINGS ################################# */
void SD_writeSettings(unsigned int i) {
    
    if (flagAbuttonHaveBeenPressed == 1) {
        
            Serial.println("__________ something changed");
            
            // Delete the old one
            if (SD.exists("settings.txt")) {
                SD.remove("settings.txt");
                Serial.println("settings removed by SD_writeSettings");       ;     
            }
            // Create a fresh empty one
            recallFile = SD.open("settings.txt", FILE_WRITE);
            
            recallFile.print("[");
            recallFile.print("exINT=");
            recallFile.print(exINT);
            recallFile.println("]");
            recallFile.print("[");
            recallFile.print("exFloat=");
            recallFile.print(exFloat,5);
            recallFile.println("]");
            recallFile.print("[");
            recallFile.print("exBoolean=");
            recallFile.print(exBoolean);
            recallFile.println("]");
            recallFile.print("[");
            recallFile.print("exLong=");
            recallFile.print(exLong);
            recallFile.println("]");
            recallFile.print("[");
            recallFile.print("doStep=");
            recallFile.print("{");
            for (int k = 0; k < 8; ++k) {
                recallFile.print(doStep[k]);
                if (k < 7) {
                    recallFile.print(",");
                }
                if (k == 7) {
                    recallFile.print("}");
                }
            }
            recallFile.println("]");
            
            Serial.println("settings created");
            //
            //SD_readSettings(); // read new settings into flaskepost
            // close the file:
            recallFile.close();
            Serial.println("############# Writing done and file closed.");
            for (int j = 0; j < 8; ++j) {
                settingValueArray[i] = doStep[i];                
            }
    }
}

void SD_readAllSettings2Monitor() {
    if (SD.exists("settings.txt")) {
        recallFile = SD.open("settings.txt");
        if (recallFile) {
            // read from the file until there's nothing else in it: (and print it to monitor)
            Serial.println("settings.txt:");
            while (recallFile.available()) {
                Serial.write(recallFile.read());
            }
            // close the file:
            recallFile.close();
        } else {
            // if the file didn't open, print an error:
            Serial.println("read all error opening settings.txt");
        }
    }
}


/* only works with max 120MHz teensy 3.6
void WRITE2EEPROM(unsigned int i) { //only write if it is different
    if (doStep[i] != EEPROM.read(i)) {
        EEPROM.write(i, doStep[i]);
    }
} */



