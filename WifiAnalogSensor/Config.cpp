#include "Config.h"

ConfigClass::ConfigClass() {}

String ConfigClass::GetOwnSSID() {
    char t[33] = "";
    const int size = sprintf(t, "%s%06X", SSID_DEFAULT, ESP.getChipId());
    return String(t);
}

// Declare an area of EEPROM where the variables are stored
void ConfigClass::InitConfig() {
#ifdef USESERIAL
    Serial.print(F("Init "));
    Serial.println(sizeof(config));
#endif 
    EEPROM.begin(sizeof(config));
}

// Writes the config values back to EEPROM
void ConfigClass::SaveConfig() {
#ifdef USESERIAL
    Serial.println(F("Save Config"));
#endif
    // Store the new settings to EEPROM
    EEPROM_writeAnything(0, config);
    EEPROM.commit();
}

// Restores the values if the EEPROM has been corrupted
void ConfigClass::ResetConfig() {
#ifdef USESERIAL
    Serial.println(F("Reset Config"));
#endif

    strcpy(config.ssid, GetOwnSSID().c_str());
    sprintf(config.pass, PASS_DEFAULT);
    sprintf(config.name, NAME_DEFAULT);
    sprintf(config.adminpass, ADMINPASS_DEFAULT);
    sprintf(config.url, URL_DEFAULT);
    sprintf(config.sync, SYNC_DEFAULT);
    sprintf(config.shot, SHOT_DEFAULT);

    SaveConfig();
}

// Checks all of the bytes in the string array to make sure they are valid characters
bool ConfigClass::ValidateString(char* value, config_valid_t type, u_short minlength) {
    if (strlen(value) < minlength) return false;
    if (strlen(value) > UINT16_MAX) return false;

    bool valid = true;
    //Check each character in the string to make sure it is alphanumeric or space
    for (uint16_t i = 0; i < strlen(value); i++) {
        switch (type)
        {
        case ConfigClass::ALL:
            if (!isPrintable(value[i])) valid = false;
            break;
        case ConfigClass::SSID:
            if (!isGraph(value[i])) valid = false;
            if (value[i] == '+' || value[i] == ']' || value[i] == '/' || value[i] == '"') valid = false;
            //first character not !#; not checked
            break;
        case ConfigClass::URL:
            if (!isGraph(value[i])) valid = false;
            break;
        case ConfigClass::NUMBER:
            if (!isDigit(value[i])) valid = false;
            break;
        default:
            if (!isPrintable(value[i])) valid = false;
            break;
        }
    }
    return valid;
}

// Loads the config values from EEPROM, leaves the default values if the EEPROM hasn't been set yet
void ConfigClass::LoadConfig() {
    bool eepromValid = true;

    // Load the config variables from EEPROM
    config_t eepromConfig;
    EEPROM_readAnything(0, eepromConfig);

    //Check to see if the config variables loaded from eeprom are valid
    eepromValid &= ValidateString(eepromConfig.ssid, ConfigClass::SSID, 1);
    eepromValid &= ValidateString(eepromConfig.pass, ConfigClass::ALL);
    eepromValid &= ValidateString(eepromConfig.name, ConfigClass::ALL, 1);
    eepromValid &= ValidateString(eepromConfig.adminpass, ConfigClass::ALL);
    eepromValid &= ValidateString(eepromConfig.url, ConfigClass::URL, 8);
    eepromValid &= ValidateString(eepromConfig.sync, ConfigClass::NUMBER, 1);
    eepromValid &= ValidateString(eepromConfig.shot, ConfigClass::NUMBER, 1);

    if (eepromValid) {
#ifdef USESERIAL
        Serial.println(F("Invalid EEPROM"));
#endif
        strcpy(config.ssid, eepromConfig.ssid);
        strcpy(config.pass, eepromConfig.pass);
        strcpy(config.name, eepromConfig.name);
        strcpy(config.adminpass, eepromConfig.adminpass);
        strcpy(config.url, eepromConfig.url);
        strcpy(config.sync, eepromConfig.sync);
        strcpy(config.shot, eepromConfig.shot);
    }
    else {
        // If the EEROM isn't valid then create a unique name for the wifi
        ResetConfig();
        SaveConfig();
    }
}

// Sends a copy of the config values out to the serial port
void ConfigClass::PrintConfig() {
#ifdef USESERIAL
    Serial.println(String(F("SSID: ")) + config.ssid);
    Serial.println(String(F("Pass: ")) + config.pass);
    Serial.println(String(F("Name: ")) + config.name);
    Serial.println(String(F("URL:  ")) + config.url);
    Serial.println(String(F("Sync: ")) + config.sync);
    Serial.println(String(F("Shot: ")) + config.shot);
#endif
}

ConfigClass Config;