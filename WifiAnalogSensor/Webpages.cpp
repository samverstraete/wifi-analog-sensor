#include "Webpages.h"

WebpagesClass::WebpagesClass() {}

String WebpagesClass::getDeviceInfo() {
	String html = "";
	html += F("<dl>");
	html += F("<dt>Chip ID</dt><dd>");
	html += ESP.getChipId();
	html += F("</dd>");
	html += F("<dt>Flash Chip Vendor / ID</dt><dd>");
	html += ESP.getFlashChipVendorId();
	html += F(" / ");
	html += ESP.getFlashChipId();
	html += F("</dd>");
	html += F("<dt>Compiler Flash Chip Size</dt><dd>");
	html += ESP.getFlashChipSize() / 1024;
	html += F(" kB</dd>");
	html += F("<dt>Real Flash Size</dt><dd>");
	html += ESP.getFlashChipRealSize() / 1024;
	html += F(" kB</dd>");
	html += F("<dt>Heap Free</dt><dd>");
	html += ESP.getFreeHeap() / 1024;
	html += F(" kB</dd>");
	html += F("<dt>Heap Fragmentation</dt><dd>");
	html += ESP.getHeapFragmentation(); // in %
	html += F(" %</dd>");
	html += F("<dt>Stack Free</dt><dd>");
	html += ESP.getFreeContStack();
	html += F(" bytes</dd>");
	html += F("<dt>Version</dt><dd>");
	html += ESP.getFullVersion();
	html += F("</dd>");
	html += F("<dt>Boot Version / Mode</dt><dd>");
	html += ESP.getBootVersion();
	html += F(" / ");
	html += ESP.getBootMode();
	html += F("</dd>");
	html += F("<dt>CPU Frequency</dt><dd>");
	html += ESP.getCpuFreqMHz();
	html += F(" MHz</dd>");
	html += F("<dt>Sketch Size / Free</dt><dd>");
	html += ESP.getSketchSize() / 1024;
	html += F(" kB / ");
	html += ESP.getFreeSketchSpace() / 1024;
	html += F(" kB</dd>");
	/*html += F("<dt>Spiffs Size</dt><dd>");
	html += String(SpiffsTotalBytes() / 1024);
	html += F(" kB</dd>");
	html += F("<dt>Spiffs Free</dt><dd>");
	html += String(SpiffsFreeSpace() / 1024);
	html += F(" kB</dd>");*/
	html += F("</dl>");
	return html;
}

String WebpagesClass::getNetworkInfo() {
	String html = "";
	html += F("<dl>");
	html += F("<dt>IP</dt><dd>");
	html += WiFi.localIP().toString();
	html += F("</dd>");
	html += F("<dt>Subnet</dt><dd>");
	html += WiFi.subnetMask().toString();
	html += F("</dd>");
	html += F("<dt>Gateway</dt><dd>");
	html += WiFi.gatewayIP().toString();
	html += F("</dd>");
	html += F("<dt>DNS</dt><dd>");
	html += WiFi.dnsIP(0).toString();
	html += F("</dd>");
	html += F("<dt>SSID</dt><dd>");
	html += WiFi.SSID();
	html += F("</dd>");
	html += F("<dt>Channel</dt><dd>");
	html += String(WiFi.channel());
	html += F("</dd>");
	html += F("<dt>MAC</dt><dd>");
	html += WiFi.macAddress();
	html += F("</dd>");
	html += F("<dt>Wireless mode</dt><dd>");

# if defined(ESP8266)
	byte PHYmode = wifi_get_phy_mode();
# endif // if defined(ESP8266)
# if defined(ESP32)
	byte PHYmode = 3; // wifi_get_phy_mode();
# endif // if defined(ESP32)

	switch (PHYmode)
	{
	case 1:
		html += F("802.11b");
		break;
	case 2:
		html += F("802.11g");
		break;
	case 3:
		html += F("802.11n");
		break;
	}

	html += F("</dd>");
	html += F("<dt>RSSI</dt><dd>");
	html += String(WiFi.RSSI());
	html += F("</dd>");

	html += F("</dl>");
	return html;
}

String WebpagesClass::getUploadInfo() {
	String html = "";
	//TODO
	html += F("<p>System boot time: <span id=\"ls\"></span></p>");
	html += F("<p>Current queue length: <span id=\"q\">{ql}</span></p>");
	html += F("<p>Last successfull upload: <span id=\"su\"></span></p>");
	html += F("<div class=\"msg\"><p><strong>Last error</strong></p>");
	html += F("<p>Time: <span id=\"et\"></span></p>");
	html += F("<p>HTTP code: <span id=\"eh\"></span></p>");
	html += F("<p>Returned data: <span id=\"ed\"></span></p>");
	html += F("</div>");
	return html;
}

//Creates a webpage that allows the user to change the SSID and Password from the browser
void WebpagesClass::serveAdmin(ESP8266WebServer* webServer, bool* needsRestart) {

	// Check to see if we've been sent any arguments and instantly return if not
	if (webServer->args() == 0) {
		webServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
		webServer->send(200, F("text/html"), getStdHeader());
		String adminpage = FPSTR(WEBPAGES_ADMIN);
		adminpage.replace(F("{ssid}"), Config.config.ssid);
		adminpage.replace(F("{appwd}"), Config.config.pass);
		adminpage.replace(F("{tname}"), Config.config.name);
		adminpage.replace(F("{pwd}"), Config.config.adminpass);
		adminpage.replace(F("{url}"), Config.config.url);
		adminpage.replace(F("{shot}"), Config.config.shot);
		adminpage.replace(F("{sync}"), Config.config.sync);
		webServer->sendContent(adminpage);
		webServer->sendContent(getStdFooter());
	}
	else {
		// Create a string containing all the arguments, send them out to the serial port
		// Check to see if there are new values (also doubles to check the length of the new value is long enough)
#ifdef USESERIAL
		Serial.println(F("==ARGS=="));
		for (int i = 0; i < webServer->args(); i++) {
			Serial.println(webServer->argName(i) + ": " + webServer->arg(i));
		}
#endif
		String error = "";

		if ((webServer->arg("ssid").length() >= 1) && (webServer->arg("ssid").length() < MAX_STR_LEN)) {
			webServer->arg("ssid").toCharArray(Config.config.ssid, sizeof(Config.config.ssid));
		}
		else {
			error += F("SSID length wrong<br>");
		}

		if ((webServer->arg("appwd").length() >= MIN_STR_LEN) && (webServer->arg("appwd").length() < MAX_STR_LEN)) {
			webServer->arg("appwd").toCharArray(Config.config.pass, sizeof(Config.config.pass));
		}
		else {
			error += F("SSID password length wrong<br>");
		}

		if ((webServer->arg("tname").length() >= 1) && (webServer->arg("tname").length() < MAX_STR_LEN)) {
			webServer->arg("tname").toCharArray(Config.config.name, sizeof(Config.config.name));
		}
		else {

			error += F("Name length wrong<br>");
		}

		if (webServer->arg("pwd").length() < MAX_STR_LEN) {
			webServer->arg("pwd").toCharArray(Config.config.adminpass, sizeof(Config.config.adminpass));
		} else {
			error += F("Admin password length wrong<br>");
		}

		if ((webServer->arg("url").length() >= MIN_STR_LEN) && (webServer->arg("url").length() < MAX_URL_LEN)) {
			webServer->arg("url").toCharArray(Config.config.url, sizeof(Config.config.url));
		} else {
			error += F("URL length wrong<br>");
		}

		if ((webServer->arg("shot").length() < MAX_TIME_LEN)) {
			webServer->arg("shot").toCharArray(Config.config.shot, sizeof(Config.config.shot));
		} else {
			error += F("Shot time length wrong<br>");
		}

		if ((webServer->arg("sync").length() < MAX_TIME_LEN)) {
			webServer->arg("sync").toCharArray(Config.config.sync, sizeof(Config.config.sync));
		} else {
			error += F("Sync time length wrong<br>");
		}

		// Store the new settings to EEPROM
		Config.SaveConfig();
		Config.PrintConfig();

		if (error == "") {
			error = F("New settings will take effect after restart");
			*needsRestart = true;
		}

		String message;
		message = F("<html><head><meta http-equiv='refresh' content='10;url=/config' /></head><body>");
		message += error;
		message += F("<br/>Redirecting in 10 seconds...</body></html>");
		webServer->sendHeader(F("Content-Length"), String(message.length()));
		webServer->send(200, F("text/html"), message);
	}
}

boolean WebpagesClass::handleCaptivePortal(ESP8266WebServer* webServer)
{
	String host = webServer->hostHeader();
	String thingName = String(Config.config.name);
	thingName.toLowerCase();
	if (!isIp(host) && !host.startsWith(thingName))
	{
#ifdef USESERIAL
		Serial.print(F("Request for "));
		Serial.print(host);
		Serial.print(F(" redirected to "));
		Serial.println(webServer->client().localIP());
#endif
		String gotoPage = "/";
		if (Config.config.adminpass[0] == '\0') gotoPage = F("/config");
		webServer->sendHeader(F("Location"), String(F("http://")) + toStringIp(webServer->client().localIP()) + gotoPage, true);
		webServer->send(302, F("text/plain"), ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
		webServer->client().stop(); // Stop is needed because we sent no content length
		return true;
	}
	return false;
}

void WebpagesClass::handleNotFound(ESP8266WebServer* webServer)
{
	if (this->handleCaptivePortal(webServer))
	{
		// If captive portal redirect instead of displaying the error page.
		return;
	}
#ifdef USESERIAL
	Serial.print(F("404 "));
	Serial.println(webServer->uri());
#endif
	String message = F("404 Not Found\n");

	webServer->sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
	webServer->sendHeader(F("Pragma"), F("no-cache"));
	webServer->sendHeader(F("Expires"), F("-1"));
	webServer->send(404, F("text/plain"), message);
}

/** Is this an IP? */
boolean WebpagesClass::isIp(String str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		int c = str.charAt(i);
		if (c != '.' && (c < '0' || c > '9'))
		{
			return false;
		}
	}
	return true;
}

/** IP to String? */
String WebpagesClass::toStringIp(IPAddress ip)
{
	String res = "";
	for (int i = 0; i < 3; i++)
	{
		res += String((ip >> (8 * i)) & 0xFF) + ".";
	}
	res += String(((ip >> 8 * 3)) & 0xFF);
	return res;
}

WebpagesClass Webpages;