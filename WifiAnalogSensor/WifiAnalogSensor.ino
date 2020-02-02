/*
 Name:		WifiAnalogSensor.ino
 Created:	10/22/2019 9:38:01 PM
 Author:	Sam


!!! ON UPDATE IotWebConf
-> comment out MDNS (we don't use it)
   //#define IOTWEBCONF_CONFIG_USE_MDNS
-> password doesn't need te be 8 chars
  //if ((0 < l) && (l < 8))
  if (false)
  }
*/

#include <ArduinoJson.hpp>
#define BUFFER_SIZE 500

#include <Ticker.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>			//https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include "IotWebConf.h"				//https://github.com/prampec/IotWebConf
#include <ArduinoJson.h>			//https://github.com/bblanchon/ArduinoJson
#include <RingBufHelpers.h>			//https://github.com/wizard97/Embedded_RingBuf_CPP
#include <RingBufCPP.h>
#include "Webpages.h"
#include <EEPROM.h>

Ticker ticker1;
Ticker ticker2;
Ticker ticker3;
DNSServer dnsServer;
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
IotWebConf iotWebConf("sensor", &dnsServer, &httpServer, "holder");

//default values
char url[200] = "http://server.lan/site/page.php?additionalpar=A";
char synctime[5] = "5";
char shottime[5] = "100";

struct Frame {
	unsigned int min;
	unsigned int mean;
	unsigned int max;
	unsigned long time;
};
RingBufCPP<Frame, BUFFER_SIZE> framebuffer;

//general variables
Frame lastFrame;
unsigned int lastRawRead;
unsigned long lastSuccessfulUploadTime;
unsigned long lastUploadErrorTime;
int lastUploadErrorHttpCode;
char lastUploadErrorReturnData[400] = "";
unsigned int samplesInFrame;
unsigned int samplesCounter;
unsigned int frameMin;
unsigned int frameMax;
float frameMean;

// -- Callback method declarations.
void configSaved();
void wifiConnected();
boolean connectAp(const char* apName, const char* password);
void connectWifi(const char* ssid, const char* password);
IotWebConfParameter custom_url = IotWebConfParameter("URL to call", "custom_url", url, 200, "text", url);
IotWebConfParameter custom_synctime = IotWebConfParameter("Seconds between URL calls", "custom_synctime", synctime, 5, "number","5", synctime , "min='1' max='99999' step='1'");
IotWebConfParameter custom_shottime = IotWebConfParameter("Milliseconds between samples", "custom_shottime", shottime, 5, "number", "100", shottime, "min='1' max='99999' step='1'");

//flags
bool needsUpload = false;
bool needsReading = false;
bool needsRestart = false;

// -- This is an OOP technique to override behaviour of the existing
// IotWebConfHtmlFormatProvider. Here two method are overriden from
// the original class. See IotWebConf.h for all potentially overridable
// methods of IotWebConfHtmlFormatProvider .
class CustomHtmlFormatProvider : public IotWebConfHtmlFormatProvider
{
protected:
	String getStyle() override
	{
		return String(FPSTR(WEBPAGES_STYLE));
	}
	String getBodyInner() override
	{
		return String(FPSTR(WEBPAGES_MENU1)) + COMPANY + String(FPSTR(WEBPAGES_MENU2));
	}
};
// -- An instance must be created from the class defined above.
CustomHtmlFormatProvider customHtmlFormatProvider;

void blink()
{
	//toggle state
	int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
	digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

void setup() {
	Serial.begin(115200);
	pinMode(BUILTIN_LED, OUTPUT);
	ticker3.attach_ms(100, blink);

	//ap parameters
	iotWebConf.addParameter(&custom_url);
	iotWebConf.addParameter(&custom_synctime);
	iotWebConf.addParameter(&custom_shottime);
	iotWebConf.setConfigSavedCallback(&configSaved);
	iotWebConf.setWifiConnectionCallback(&wifiConnected);
	iotWebConf.setApConnectionHandler(&connectAp);
	iotWebConf.setWifiConnectionHandler(&connectWifi);
	iotWebConf.setHtmlFormatProvider(&customHtmlFormatProvider);
	iotWebConf.getApPasswordParameter()->label = "Configuration password";
	iotWebConf.skipApStartup();
	//TODO: configpage password?

	// -- Set up required URL handlers on the web server.
	httpUpdater.setup(&httpServer);
	httpServer.on("/", handleRoot);
	httpServer.on("/restart", handleRestart);
	httpServer.on("/data", handleData);
	httpServer.on("/config", [] { iotWebConf.handleConfig(); });
	httpServer.onNotFound([]() { iotWebConf.handleNotFound(); });
	httpServer.begin();

	// -- Initializing the configuration.
	iotWebConf.init();

	//ticker1.attach_ms(atoi(shottime), []() { needsReading = true;});
	ticker1.attach_ms(atoi(shottime), readSensor);
	ticker2.attach(atoi(synctime), []() { needsUpload = true;});
	samplesInFrame = atoi(synctime) * 1000 / atoi(shottime);
	samplesCounter = 0;
	frameMin = UINT_MAX;
	frameMean = 0;
	frameMax = 0;

}

void loop() {
	iotWebConf.doLoop();
	if (needsReading) {
		readSensor();
		needsReading = false;
	}
	if (needsUpload) {
		uploadData();
		needsUpload = false;
	}
	if (needsRestart) {
		doRestart();
		needsRestart = false;
	}
	//update led according to state
	switch (iotWebConf.getState()) {
	case IOTWEBCONF_STATE_BOOT:
		ticker3.attach_ms(100, blink);
		break;
	case IOTWEBCONF_STATE_AP_MODE:
	case IOTWEBCONF_STATE_NOT_CONFIGURED:
		ticker3.attach_ms(600, blink);
		break;
	case IOTWEBCONF_STATE_CONNECTING:
		ticker3.attach_ms(300, blink);
		break;
	case IOTWEBCONF_STATE_ONLINE:
		ticker3.detach();
		//stop the self-config AP if nobody is connected
		if(WiFi.softAPgetStationNum() < 1) WiFi.softAPdisconnect(true);	
		break;
	default:
		ticker3.attach_ms(600, blink);
		break;
	}
}

void readSensor() {
	lastRawRead = analogRead(A0);
	if (lastRawRead < frameMin) frameMin = lastRawRead;
	if (lastRawRead > frameMax) frameMax = lastRawRead;
	frameMean += (float)lastRawRead / samplesInFrame;
	samplesCounter++;
	if (samplesCounter >= samplesInFrame) {
		//add frame to queue for upload
		Serial.println(F("Adding frame"));
		struct Frame frame = { frameMin, frameMean, frameMax, millis() };
		framebuffer.add(frame);
		lastFrame = frame;
		//reset vars
		samplesCounter = 0;
		frameMin = UINT_MAX;
		frameMean = 0;
		frameMax = 0;
	}
}

void uploadData() {
	digitalWrite(LED_BUILTIN, LOW);
	HTTPClient http;						//Declare object of class HTTPClient

	for (int i = 0; i < framebuffer.numElements(); i++) {
		http.begin(url);					//Specify request destination
		http.addHeader(F("Content-Type"), F("text/plain"));  //Specify content-type header
		http.addHeader(F("Accept"), F("text/plain"));
		Serial.println(F("Uploading frame"));

		Frame frame;
		String data;
		DynamicJsonDocument json(256);

		framebuffer.pull(&frame);
		JsonObject jsonFrame = json.createNestedObject("frame");
		jsonFrame["min"] = frame.min;
		jsonFrame["mean"] = frame.mean;
		jsonFrame["max"] = frame.max;
		jsonFrame["time"] = frame.time;
		json["millis"] = millis();
		serializeJson(json, data);
		int httpCode = http.POST(data);		//Send the request
		char payload[400];
		http.getString().substring(0, 398).toCharArray(payload, 400, 0);	//Get the response payload
		http.end();							//Close connection

		Serial.println(httpCode);			//Print HTTP return code
		if (httpCode != 200 || !(payload[0] == 'o' && payload[1] =='k') ) {
			//Upload failed, add frame back to buffer
			Serial.println(F("Upload failed"));
			framebuffer.add(frame);
			lastUploadErrorHttpCode = httpCode;
			lastUploadErrorTime = millis();
			strcpy(lastUploadErrorReturnData, payload);
			break;
		}
		else {
			lastSuccessfulUploadTime = millis();
			Serial.println(payload);
		}
		
	}

	digitalWrite(LED_BUILTIN, HIGH);
}

void handleRoot() {
	digitalWrite(LED_BUILTIN, LOW);
	// -- Let IotWebConf test and handle captive portal requests.
	if (iotWebConf.handleCaptivePortal())
	{
		// -- Captive portal request were already served.
		return;
	}
	String fullpage = Webpages.getStdHeader();
	String mainpage = FPSTR(WEBPAGES_MAIN);
	mainpage.replace(F("{dev}"), Webpages.getDeviceInfo());
	mainpage.replace(F("{net}"), Webpages.getNetworkInfo());
	mainpage.replace(F("{upl}"), Webpages.getUploadInfo());
	mainpage.replace(F("{ql}"), (String)framebuffer.numElements());
	fullpage += mainpage;
	fullpage += Webpages.getStdFooter();
	httpServer.send(200, F("text/html"), fullpage);
	digitalWrite(LED_BUILTIN, HIGH);
}

void handleRestart() {
	digitalWrite(LED_BUILTIN, LOW);
	String fullpage = Webpages.getStdHeader();
	if (httpServer.hasArg("r")) {
		fullpage += FPSTR(WEBPAGES_REBOOTING);
	}
	else {
		fullpage += FPSTR(WEBPAGES_REBOOT);
	}
	fullpage += Webpages.getStdFooter();
	httpServer.send(200, F("text/html"), fullpage);
	digitalWrite(LED_BUILTIN, HIGH);
	if (httpServer.hasArg("r")) {
		if (httpServer.hasArg("clearwifi")) {
			//invalidate the config version
			for (byte t = 0; t < IOTWEBCONF_CONFIG_VERSION_LENGTH; t++)
			{
				EEPROM.write(IOTWEBCONF_CONFIG_START + t, 0);
			}
			EEPROM.commit();
		}
		needsRestart = true;
	}
}

void doRestart() {
	Serial.println(F("Rebooting..."));
	ticker1.once(1, []() {ESP.restart();});
	ticker2.detach();
	ticker3.attach_ms(50, blink);
}

void handleData() {
	digitalWrite(LED_BUILTIN, LOW);
	String data;
	DynamicJsonDocument json(256);
	json["read"] = lastRawRead;
	JsonObject jsonFrame = json.createNestedObject("frame");
	jsonFrame["min"] = lastFrame.min;
	jsonFrame["mean"] = lastFrame.mean;
	jsonFrame["max"] = lastFrame.max;
	jsonFrame["time"] = lastFrame.time;
	JsonObject jsonError = json.createNestedObject("lasterr");
	jsonError["time"] = lastUploadErrorTime;
	jsonError["data"] = lastUploadErrorReturnData;
	jsonError["code"] = lastUploadErrorHttpCode;
	json["millis"] = millis();
	json["lastgood"] = lastSuccessfulUploadTime;
	json["queue"] = framebuffer.numElements();
	serializeJson(json, data);
	httpServer.send(200, F("text/plain"), data);
	digitalWrite(LED_BUILTIN, HIGH);
}

void configSaved() {
	Serial.println(F("Configuration was updated."));
	doRestart();
}

void wifiConnected() {
	//if you get here you have connected to the WiFi
	Serial.print(F("Local ip: "));
	Serial.println(WiFi.localIP());
	//keep LED off
	digitalWrite(BUILTIN_LED, HIGH);
}

boolean connectAp(const char* apName, const char* password) {
	Serial.println(F("Entered config mode"));
	return WiFi.softAP(apName);
}

void connectWifi(const char* ssid, const char* password) {
	Serial.print(F("Connecting to WiFi: "));
	Serial.println(ssid);
	WiFi.begin(ssid, password);
}
