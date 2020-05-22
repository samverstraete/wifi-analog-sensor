/*
 Name:		WifiAnalogSensor.ino
 Created:	10/22/2019 9:38:01 PM
 Author:	Sam


*/

#define BUFFER_SIZE 500
#define ADMIN_USERNAME "admin"
#define LED_BUILTIN 2 //NodeMCU v3 only

#include <ArduinoJson.hpp>
#include <Ticker.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>			//https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>			//https://github.com/bblanchon/ArduinoJson
#include <RingBufHelpers.h>			//https://github.com/wizard97/Embedded_RingBuf_CPP
#include <RingBufCPP.h>
#include <EEPROM.h>
#include "Config.h"					//https://github.com/msraynsford/APConfig
#include "FirmwareReset.h"
#include "Webpages.h"

Ticker ticker1;
Ticker ticker2;
Ticker ticker3;
Ticker ticker4;
DNSServer dnsServer;
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

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

//flags
bool needsUpload = false;
bool needsReading = false;
bool needsRestart = false;
bool reconfigMode = false;

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

	Serial.println(F("WifiAnalogSensor"));

	reconfigMode = checkResetFlag();

	Config.InitConfig();
	Config.LoadConfig();
	Config.PrintConfig();
	yield();

	//Start the wifi
	WiFi.mode(WIFI_AP_STA);
	WiFi.enableSTA(false);
	//Check to see if the flag is still set from the previous boot
	if (reconfigMode) WiFi.softAP(Config.GetOwnSSID(), "");	

	/* Setup the DNS server redirecting all the domains to the apIP */
	dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer.start(53, "*", WiFi.softAPIP());

	//Set up required URL handlers on the web server.
	httpUpdater.setup(&httpServer);
	if (Config.config.adminpass[0] != '\0' && !reconfigMode) httpUpdater.updateCredentials(ADMIN_USERNAME, Config.config.adminpass);
	httpServer.on("/", handleRoot);
	httpServer.on("/restart", handleRestart);
	httpServer.on("/data", handleData);
	httpServer.on("/config", handleAdmin);
	httpServer.onNotFound([]() { Webpages.handleNotFound(&httpServer); });
	httpServer.begin();

	
	if (reconfigMode) {
		//Try to connect after 10 minutes 
		ticker4.once(600, []() {
			connectWifi();
		});
	} else {
		//Try to connect to the AP now
		connectWifi();
	}

	ticker1.attach_ms(atoi(Config.config.shot), readSensor);
	ticker2.attach(atoi(Config.config.sync), []() { needsUpload = true;});
	samplesInFrame = atoi(Config.config.sync) * 1000 / atoi(Config.config.shot);
	samplesCounter = 0;
	frameMin = UINT_MAX;
	frameMean = 0;
	frameMax = 0;

}

void loop() {
	dnsServer.processNextRequest();
	httpServer.handleClient();
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
	//check WiFi state
	switch (WiFi.status()) {
	case WL_IDLE_STATUS:
		ticker3.attach_ms(100, blink);
		break;
	case WL_NO_SSID_AVAIL:  //not found
	case WL_CONNECT_FAILED: //password incorrect
		ticker3.attach_ms(600, blink);
		break;
	case WL_CONNECTION_LOST://signal lost
	case WL_DISCONNECTED:   //not configured in station mode
		ticker3.attach_ms(300, blink);
		break;
	case WL_CONNECTED:
		ticker3.detach();
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
		http.begin(Config.config.url);					//Specify request destination
		http.addHeader(F("Content-Type"), F("text/plain"));  //Specify content-type header
		http.addHeader(F("Accept"), F("text/plain"));

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

		if (httpCode != 200 || !(payload[0] == 'o' && payload[1] =='k') ) {
			//Upload failed, add frame back to buffer
			Serial.print(F("Upload failed, HTTP "));
			Serial.println(httpCode);
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
	// -- Test and handle captive portal requests.
	if (Webpages.handleCaptivePortal(&httpServer))
	{
		// -- Captive portal request were already served.
		return;
	}
	httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
	httpServer.send(200, F("text/html"), Webpages.getStdHeader());
	String mainpage = FPSTR(WEBPAGES_MAIN);
	mainpage.replace(F("{dev}"), Webpages.getDeviceInfo());
	mainpage.replace(F("{net}"), Webpages.getNetworkInfo());
	mainpage.replace(F("{upl}"), Webpages.getUploadInfo());
	mainpage.replace(F("{ql}"), (String)framebuffer.numElements());
	httpServer.sendContent(mainpage);
	httpServer.sendContent(Webpages.getStdFooter());
	digitalWrite(LED_BUILTIN, HIGH);
}

void handleAdmin() {
	digitalWrite(LED_BUILTIN, LOW);
	// -- Authenticate
	if (!reconfigMode && Config.config.adminpass[0] != '\0' && !httpServer.authenticate(ADMIN_USERNAME, Config.config.adminpass)) {
		Serial.println(F("Auth required"));
		httpServer.requestAuthentication();
		return;
	}
	Webpages.serveAdmin(&httpServer, &needsRestart);

	digitalWrite(LED_BUILTIN, HIGH);
}

void handleRestart() {
	digitalWrite(LED_BUILTIN, LOW);
	if (!reconfigMode && httpServer.hasArg("clear") && Config.config.adminpass[0] != '\0' && !httpServer.authenticate(ADMIN_USERNAME, Config.config.adminpass)) {
		Serial.println(F("Auth required"));
		httpServer.requestAuthentication();
		return;
	}
	httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
	httpServer.send(200, F("text/html"), Webpages.getStdHeader());
	if (httpServer.hasArg("r")) {
		httpServer.sendContent(FPSTR(WEBPAGES_REBOOTING));
	}
	else {
		httpServer.sendContent(FPSTR(WEBPAGES_REBOOT));
	}
	httpServer.sendContent(Webpages.getStdFooter());
	digitalWrite(LED_BUILTIN, HIGH);
	if (httpServer.hasArg("r")) {
		if (httpServer.hasArg("clear")) {
			Config.ResetConfig();
		}
		needsRestart = true;
	}
}

void doRestart() {
	Serial.println(F("Rebooting..."));
	ticker1.once(1, []() {
		WiFi.disconnect(true); 
		ESP.restart();
	});
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

void connectWifi() {
	if (Config.GetOwnSSID() != Config.config.ssid) {
		Serial.println(F("STA trying to connect"));
		WiFi.enableSTA(true);
		WiFi.begin(Config.config.ssid, Config.config.pass);
	}
}

void onStationModeGotIP(WiFiEventStationModeGotIP event) {
	//if you get here you have connected to the WiFi
	Serial.print(F("Local ip: ")); 
	Serial.println(event.ip);
	Serial.print(F("Gateway: "));
	Serial.println(event.gw);
	//keep LED off
	digitalWrite(BUILTIN_LED, HIGH);
	//stop the self-config AP if nobody is connected
	if (WiFi.softAPgetStationNum() < 1) {
		Serial.println(F("Stopping AP"));
		dnsServer.stop();
		WiFi.softAPdisconnect(true);
	}
}

void onStationModeConnected(WiFiEventStationModeConnected event) {
	Serial.print(F("Connected to WiFi: "));
	Serial.println(event.ssid);
}

void onStationModeDisconnected(WiFiEventStationModeDisconnected event) {
	Serial.print(F("Disconnected from WiFi, reason: "));
	Serial.println(event.reason);
	if (event.reason == 201) {
		//wait some time before trying to reconnect, to allow reliable softAP connections
		WiFi.enableSTA(false);
		ticker4.once(10, []() {
			connectWifi();
		});
	}
}

void onStationModeDHCPTimeout() {
	Serial.println(F("DHCP timeout"));
}

// -- Callback method declarations.
WiFiEventHandler _onStationModeConnectedHandler = WiFi.onStationModeConnected(onStationModeConnected);
WiFiEventHandler _onStationModeDisconnectedHandler = WiFi.onStationModeDisconnected(onStationModeDisconnected);
WiFiEventHandler _onStationModeDHCPTimeoutHandler = WiFi.onStationModeDHCPTimeout(onStationModeDHCPTimeout);
WiFiEventHandler _onStationModeGotIPHandler = WiFi.onStationModeGotIP(onStationModeGotIP);
