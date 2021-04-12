#include <MD_Parola.h>
#include <SPI.h>
#include <MD_MAX72xx.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ElegantOTA.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include "JF_Font_Data.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN   D5
#define DATA_PIN  D7
#define CS_PIN    D8

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
time_t ntptime;
//uint8_t  nightsymbol[] = {8, 0x10, 0x30, 0x60, 0x61, 0x73, 0x3e, 0x1c, 0x80};
uint8_t  nightsymbol[] = {10, 0x2, 0x27, 0x42, 0xc0, 0xc1, 0x67, 0x7e, 0x3c, 0x1, 0x83};
uint8_t  daysymbol[] = {10, 0x12, 0x49, 0x24, 0x92, 0x48, 0x23, 0x87, 0x4f, 0x1f, 0x9f};

void setup() {
  Serial.begin(115200);
  P.begin();
  P.addChar('<', nightsymbol);
  P.addChar('>', daysymbol);
  P.setFont(jF_Custom);
  P.setInvert(false);
  P.print(".con");
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  WiFiManager wm;
  wm.setConfigPortalTimeout(120); // auto close configportal after n seconds
  if (!wm.autoConnect("NodeMCU Clock LED Matrix")) {
    P.print(".res");
    delay(3500);
    ESP.reset();
  }
  P.print(".con_ok");
  delay(2500);
  server.on("/", []() {
    server.send(200, "text/plain", "NodeMCU LED Matrix\nGo to 'http://<IPAddress>/update' to perform OTA update");
  });
  ElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  P.print(".upd");
  delay(2500);
  timeClient.begin();
  timeClient.setTimeOffset(7200);
  timeClient.forceUpdate();
  setTime(timeClient.getEpochTime());
  P.setIntensity(1);
}

void loop() {
  if (millis() > 7200000) {
    WiFi.mode(WIFI_OFF);
  }
  String strhour, strminute;
  char dayornight, separator;
  if (hourFormat12() < 10) {
    strhour = "0" + String(hourFormat12());
  }
  else {
    strhour = String(hourFormat12());
  }
  if (minute() < 10) {
    strminute = "0" + String(minute());
  }
  else {
    strminute = String(minute());
  }
  if (isAM()) {
    dayornight = '>';
  }
  if (isPM()) {
    dayornight = '<';
  }
  if (second() % 2) {
    separator = ':';
  }
  else {
    separator = ' ';
  }
  P.print(strhour + separator + strminute + dayornight);
  server.handleClient();
}
