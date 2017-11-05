#include "Settings.h"
#include <Arduino.h>


extern int exINT;
extern float exFloat;
extern boolean exBoolean;
extern long exLong;
extern boolean exBooleanArray[];

String settingValueArray[8];



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
// 1 = true
// 0 = false
boolean toBoolean(String settingValue) {
    if(settingValue.toInt()==1){
        return true;
    } else {
        return false;
    }
}

const int chipSelect = BUILTIN_SDCARD;
//bool filePosValue[8] = {true, true, true, true, true, true, true, true};
File recallFile;

void SD_init() {
    Serial.print("Initializing SD card...");
    if (!SD.begin(chipSelect)) {
        Serial.println("initialization failed!");
        return;
    }
    Serial.println("initialization done.");
    
    /* if (SD.exists("recall.txt")) {
        SD.remove("recall.txt");
        Serial.println("removed recall.txt");
    } */
}

void SD_readSettings(){
    char character;
    String settingName;
    String settingValue;
    
    recallFile = SD.open("settings.txt");
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
            //Serial.print("debuggg names __ ");
            //Serial.println(settingName);
            if (settingName.length() == 0) {
                Serial.println("--> halla --> String settingName is empty !");
            } // break , goto close file
            
            if (character == '{') {
                character = recallFile.read();
                //Serial.println("i found a {");
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
                if (settingName != "exBooleanArray") {
                    Serial.print("Name:");
                    Serial.println(settingName);
                    Serial.print("Value :");
                    Serial.println(settingValue);
                }
                
                if (settingName == "exBooleanArray") {
                    for (int j = 0; j < 8; ++j) {
                        Serial.print("settingValueArray: ");
                        Serial.println(settingValueArray[j]);
                        Serial.print("exBooleanArray: ");
                        Serial.println(exBooleanArray[j]);
                        
                    }
                } // debugging end
                
                // Apply the value to the parameter
                applySetting(settingName,settingValue);
                // Reset Strings
                settingName = "";
                settingValue = "";
            }
        }
        // close the file:
        recallFile.close();
        Serial.println("closed file");
    } else {
        // if the file didn't open, print an error:
        Serial.println("error opening settings.txt");
    }
}

void SD_readAllSettings2Monitor() {
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

/* Apply the value to the parameter by searching for the parameter name
Using String.toInt(); for Integers
toFloat(string); for Float
toBoolean(string); for Boolean
toLong(string); for Long
*/
void applySetting(String settingName, String settingValue) {
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
    if(settingName == "exBooleanArray") {
        for (unsigned int i = 0; i < 8; ++i) {
            exBooleanArray[i] = toBoolean(settingValueArray[i]);
        }
    }
    Serial.print("applied ");
    Serial.print(settingName);
    Serial.print(" ");
    Serial.println("setting");
}


void SD_writeSettings(unsigned int i) {
    
    //exBooleanArray[i] = doStep[i];
    if (doStep[i] != exBooleanArray[i]) {
        //Serial.println("__________ something changed");
        
        // Delete the old one
        SD.remove("settings.txt");
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
        recallFile.print("exBooleanArray=");
        recallFile.print("{");
        for (int i = 0; i < 8; ++i) {
            recallFile.print(exBooleanArray[i]);
            if (i < 7) {
                recallFile.print(",");
            }
            if (i == 7) {
                recallFile.print("}");
            }
        }
        recallFile.println("]");
        //
        //SD_readSettings(); // read new settings into flaskepost
        // close the file:
        recallFile.close();
        //Serial.println("############# Writing done.");
        exBooleanArray[i] = doStep[i];
    }
}



// void SD_write(unsigned int i) {        
    //     // open and set mode FILE_WRITE (if no recallFile exits, create it)
    //     recallFile = SD.open("recall.txt", FILE_WRITE);
    //     if (recallFile) {
        //         // if change present, then write
        //         if (doStep[i] != filePosValue[i]) {
            //             recallFile.seek(i+1);
            //             recallFile.println(doStep[2]);
            //             recallFile.close();
            //             Serial.println("done writing change.");
            //         }
            //     } else {
                //         // if the file didn't open, print an error:
                //         Serial.println("write error opening recall.txt");
                //     }
                // }
                
                // void SD_read(unsigned int i) {
                    //     // open the file:
                    //     recallFile = SD.open("recall.txt");
                    //     if (recallFile) { // get settings from before powerdown
                        
                        //         recallFile.seek(i);
                        //         filePosValue[i] = recallFile.peek();
                        
                        //         // read from the file until there's nothing else in it: (and print it to monitor)
                        //         Serial.println("recall.txt:");
                        //         while (recallFile.available()) {
                            //             Serial.write(recallFile.read());
                            //         }
                            //         // close the file:
                            //         recallFile.close();
                            //     } else {
                                //         // if the file didn't open, print an error:
                                //         Serial.println("read error opening recall.txt");
                                //     }
                                // }
                                
                                
                                // only works with max 120MHz teensy 3.6
                                // void WRITE2EEPROM(unsigned int i) { //only write if it is different
                                    //     if (doStep[i] != EEPROM.read(i)) {
                                        //         EEPROM.write(i, doStep[i]);
                                        //     }
                                        // }
                                        
                                        
                                        
                                        