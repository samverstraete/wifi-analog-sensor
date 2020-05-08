// Handles a class of data for storing variables, and the reading/writing of those variables to EEPROM
// It is able to detect invalid data being read from EEPROM and restore some basic values in those circumstances

#ifndef Config_H
#define Config_H

#include "EEPROMAnything.h"

#define USESERIAL
#define USELED

#define MIN_STR_LEN 8
#define MAX_STR_LEN 33
#define MAX_URL_LEN 200
#define MAX_TIME_LEN 6

#define SSID_DEFAULT "SensorConfig"
#define PASS_DEFAULT ""
#define NAME_DEFAULT "Sensor"
#define ADMINPASS_DEFAULT ""
const static char URL_DEFAULT[] PROGMEM = "http://samverstraete.net/bit/upload.php?device=1";
#define SYNC_DEFAULT "5"
#define SHOT_DEFAULT "100"

class ConfigClass {
public:
    // Define a structure to hold all the variables that are going to be stored in EEPROM
    struct config_t {
        char ssid[MAX_STR_LEN] = SSID_DEFAULT;
        char pass[MAX_STR_LEN] = PASS_DEFAULT;
        char name[MAX_STR_LEN] = NAME_DEFAULT;
        char adminpass[MAX_STR_LEN] = ADMINPASS_DEFAULT;
        char url[MAX_URL_LEN] = "";
        char sync[MAX_TIME_LEN] = SYNC_DEFAULT;
        char shot[MAX_TIME_LEN] = SHOT_DEFAULT;
    } config;

    enum config_valid_t {  
        ALL = 0,
        SSID = 1,
        URL = 2,
        NUMBER = 3
    };

    ConfigClass();
    String GetOwnSSID();
    void InitConfig();
    void SaveConfig();
    void ResetConfig();
    bool ValidateString(char* value, config_valid_t type, u_short minlength = 0);
    void LoadConfig();
    void PrintConfig();
};

extern ConfigClass Config;

#endif