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

WebpagesClass Webpages;