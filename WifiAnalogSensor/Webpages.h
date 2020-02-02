#pragma once

#ifndef Webpages_h
#define Webpages_h

#ifndef COMPANY
#define COMPANY ""
#endif // !COMPANY

#include "arduino.h"
#include <ESP8266WiFi.h>

//mincss.com
const char WEBPAGES_HEAD1[] PROGMEM = R"=====(
	<HTML>
	<HEAD>
		<TITLE>)=====";
const char WEBPAGES_HEAD2[] PROGMEM = R"=====( Analog Sensor</TITLE>)=====";

const char WEBPAGES_STYLE[] PROGMEM = R"======(
		<style>
			body,textarea,input,select{background:0;border-radius:0;font:16px sans-serif;margin:0}.smooth{transition:all .2s}button,.btn,.nav a{text-decoration:none}.container{margin:0 20px;width:auto}label>*{display:inline}form>*{display:block;margin-bottom:10px}button,.btn{background:#999;border-radius:6px;border:0;color:#fff;cursor:pointer;display:inline-block;margin:2px 0;padding:12px 30px 14px}button:hover,.btn:hover{background:#888}button:active,button:focus,.btn:active,.btn:focus{background:#777}.btn-a{background:#0ae}.btn-a:hover{background:#09d}.btn-a:active,.btn-a:focus{background:#08b}.btn-b{background:#3c5}.btn-b:hover{background:#2b4}.btn-b:active,.btn-b:focus{background:#2a4}.btn-c{background:#d33}.btn-c:hover{background:#c22}.btn-c:active,.btn-c:focus{background:#b22}.btn-sm{border-radius:4px;padding:10px 14px 11px}.row{margin:1% 0;overflow:auto}.col{float:left}.table,.c12{width:100%}.c11{width:91.66%}.c10{width:83.33%}.c9{width:75%}.c8{width:66.66%}.c7{width:58.33%}.c6{width:50%}.c5{width:41.66%}.c4{width:33.33%}.c3{width:25%}.c2{width:16.66%}.c1{width:8.33%}h1{font-size:3em}.btn,h2{font-size:2em}.ico{font:33px Arial Unicode MS,Lucida Sans Unicode}.addon,.btn-sm,.nav,textarea,input,select{outline:0;font-size:14px}textarea,input,select{padding:4px;margin:8px;border:1px solid #ccc}textarea:focus,input:focus,select:focus{border-color:#5ab}textarea,input[type=text]{-webkit-appearance:none;width:20em}.addon{padding:8px 12px;box-shadow:0 0 0 1px #ccc}.nav,.nav .current,.nav a:hover{background:#094;color:#fff}.nav{height:24px;padding:11px 0 15px}.nav a{color:#beb;padding-right:1em;position:relative;top:-1px}.nav .pagename{font-size:22px;top:1px}.btn.btn-close{background:#000;float:right;font-size:25px;margin:-54px 7px;display:none}@media(min-width:1310px){.container{margin:auto;width:1270px}}@media(max-width:870px){.row .col{width:100%}}@media(max-width:500px){.btn.btn-close{display:block}.nav{overflow:hidden}.pagename{margin-top:-11px}.nav:active,.nav:focus{height:auto}.nav div:before{background:#000;border-bottom:10px double;border-top:3px solid;content:'';float:right;height:4px;position:relative;right:3px;top:14px;width:20px}.nav a{padding:.5em 0;display:block;width:50%}}.table th,.table td{padding:.5em;text-align:left}.table tbody>:nth-child(2n-1){background:#ddd}.msg{padding:1.5em;background:#def;border-left:5px solid #59d}
			.hero {background:#efe;padding:20px;border-radius:10px;margin-top:1em;}.hero h1{margin-top:0;margin-bottom:0.3em;}.c4{padding:10px;box-sizing: border-box;}.c4 h3{margin-top:0;}.c4 a{margin-top:10px;display:inline-block;}
		</style>
	)======";

const char WEBPAGES_HEAD_END[] PROGMEM = R"======(
	</HEAD>
	<BODY>
	)======";

const char WEBPAGES_MENU1[] PROGMEM = R"======(
		<nav class="nav" tabindex="-1" onclick="this.focus()">
			<div class="container">
				<a class="pagename current" href="#">)======";
const char WEBPAGES_MENU2[] PROGMEM = R"======( Analog Sensor</a>
				<a href="/">Home</a>
				<a href="/update">Update</a> 
				<a href="/config">Network</a>
				<a href="/restart">Restart</a>
			</div>
		</nav>
		<div class="container">
	)======";

const char WEBPAGES_MAIN[] PROGMEM = R"=====(
			<div class="hero">
				<h1>Sensor</h1>
				<h3>Reading: <span id="r"></span></h3>
				<p>Time: <span id="t"></span></p>
			</div>
			<div class="row">
				<div class="col c4"><h3>Board info</h3>{dev}</div>
				<div class="col c4"><h3>Metrics</h3>{upl}</div>
				<div class="col c4"><h3>Network</h3>{net}</div>
			</div>
			<script>
				var xhttp = new XMLHttpRequest(); 
				xhttp.onreadystatechange = function() {
					if (this.readyState == 4 && this.status == 200) {
						obj = JSON.parse(this.responseText);
						document.getElementById("r").innerHTML = obj.read;
						document.getElementById("t").innerHTML = dt(obj.millis - obj.frame.time);
						document.getElementById("q").innerHTML = obj.queue;
						document.getElementById("ls").innerHTML = dt(obj.millis);
						if(obj.lastgood > 0) document.getElementById("su").innerHTML = dt(obj.millis - obj.lastgood);
						document.getElementById("et").innerHTML = dt(obj.millis - obj.lasterr.time);
						document.getElementById("eh").innerHTML = obj.lasterr.code;
						document.getElementById("ed").innerHTML = obj.lasterr.data;
					} 
				};
				setInterval(function(){ 
					xhttp.open("GET", "/data", true);
					xhttp.send(); 
				}, 1000);
				function dt(millis){ return (new Date(Date.now() - millis)).toLocaleString(); }
			</script>
	)=====";

const char WEBPAGES_REBOOT[] PROGMEM = R"=====(
			<h3>Restart this device?</h3>
			<form action="/restart?r" method="POST">
			<p><input type="checkbox" id="clearwifi" name="clearwifi">
			<label for="clearwifi">Clear WiFi settings</label></p><br>
			<button type="submit" class="btn btn-a btn-sm smooth">Restart now</a>
			</form>
	)=====";

const char WEBPAGES_REBOOTING[] PROGMEM = R"=====(
			<h3>Restarting...</h3>
	)=====";

const char WEBPAGES_FOOT[] PROGMEM = R"=====(
		</div>
	</BODY>
	</HTML>
	)=====";

class WebpagesClass {
public:
	WebpagesClass();
	String getDeviceInfo();
	String getNetworkInfo();
	String getUploadInfo();

	static String getStdHeader() { return String(FPSTR(WEBPAGES_HEAD1)) + COMPANY + String(FPSTR(WEBPAGES_HEAD2))
		+ FPSTR(WEBPAGES_STYLE) + FPSTR(WEBPAGES_HEAD_END) + FPSTR(WEBPAGES_MENU1) + COMPANY + FPSTR(WEBPAGES_MENU2); }
	static String getStdFooter() { return FPSTR(WEBPAGES_FOOT); }
};

extern WebpagesClass Webpages;
#endif // !Webpages_h