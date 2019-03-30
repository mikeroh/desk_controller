/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef APSSID
#define APSSID "Desk Controller"
#define APPSK  "pantspants"
#endif

#define UP_PIN 5
#define DOWN_PIN 13

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

ESP8266WebServer server(80);

enum desk_direction {
  still = 0,
  down,
  up
};

const char * web_page ="<!DOCTYPE html>"
"<html>"
"<head>"
"<style>"

"a.blue_button {"
"display: block;"
"background-color: #1da1f2;"
"width: 95%;"
"height: 95px;"
"border: 5px solid #333;"
"-webkit-border-radius: 10px;"
"-moz-border-radius: 10px;"
"border-radius: 10px;"
"color: black;"
"padding: 80px 0px;"
"text-align: center;"
"cursor: pointer;"
"font-size: 75px;"
"text-decoration: none;"
"}"

"a.red_button {"
"display: block;"
"background-color: #ff3300;"
"width: 95%;"
"height: 100px;"
"border: 5px solid #333;"
"-webkit-border-radius: 10px;"
"-moz-border-radius: 10px;"
"border-radius: 10px;"
"color: black;"
"padding: 80px 0px;"
"text-align: center;"
"cursor: pointer;"
"font-size: 75px;"
"text-decoration: none;"
"}"

"a.green_button {"
"display: block;"
"background-color: #33cc33;"
"width: 95%;"
"height: 100px;"
"border: 5px solid #333;"
"-webkit-border-radius: 10px;"
"-moz-border-radius: 10px;"
"border-radius: 10px;"
"color: black;"
"padding: 80px 0px;"
"text-align: center;"
"cursor: pointer;"
"font-size: 75px;"
"text-decoration: none;"
"}"

"</style>"
"</head>"
"<body>"
"<p><font size=\"30\">Welcome to the desk controller V0.1</font></p>"
"<p><a href=\"/up\" class=\"blue_button\">Move Up</a></p>"
"<p><a href=\"/down\" class=\"blue_button\">Move Down</a></p>"
"<p><a href=\"/creep\" class=\"green_button\">Start Creep Up</a></p>"
"<p><a href=\"/stop\" class=\"red_button\">Stop Creep</a></p>"

"</body>"
"</html>";

desk_direction dir = still;

bool creep = false;
unsigned long long next_creep = 0;
unsigned long long move_timeout = 0;

void handleRoot() {
  server.send(200, "text/html", web_page);
}

void deskUp(){
  server.send(200, "text/html", "<meta http-equiv=\"refresh\" content=\"0; url=/\" />");
  move_timeout = millis() + 500;
  digitalWrite(UP_PIN, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
}

void deskDown(){
  server.send(200, "text/html", "<meta http-equiv=\"refresh\" content=\"0; url=/\" />");
  move_timeout = millis() + 500;
  digitalWrite(DOWN_PIN, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
}

void startCreep(){
  server.send(200, "text/html", "<meta http-equiv=\"refresh\" content=\"0; url=/\" />");
  creep = true;
  next_creep = millis();
}

void stopCreep(){
  server.send(200, "text/html", "<meta http-equiv=\"refresh\" content=\"0; url=/\" />");
  creep = false;
  move_timeout = millis();
}

void setup() {
  delay(1000);
  WiFi.mode(WIFI_AP);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();

  
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/up", deskUp);
  server.on("/down", deskDown);
  server.on("/creep", startCreep);
  server.on("/stop", stopCreep);
  
  if (!MDNS.begin("desk")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  
  server.begin();
  Serial.println("HTTP server started");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(UP_PIN, OUTPUT);
  pinMode(DOWN_PIN, OUTPUT);
  digitalWrite(UP_PIN, LOW);
  digitalWrite(DOWN_PIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);

  MDNS.addService("http", "tcp", 80);
}

void loop() {
  unsigned long long current_time = millis();
    if(current_time >= move_timeout) {
      digitalWrite(UP_PIN, LOW);
      digitalWrite(DOWN_PIN, LOW);
      digitalWrite(LED_BUILTIN, HIGH);
    }

    if(creep &&
      current_time >= next_creep){
      digitalWrite(UP_PIN, HIGH);
      digitalWrite(LED_BUILTIN, LOW);
      move_timeout = current_time + 5;
      next_creep = current_time + 600000; //10 minutes
    }
  server.handleClient();
  MDNS.update();
}
