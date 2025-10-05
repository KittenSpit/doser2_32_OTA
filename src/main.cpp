#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include "Config.h"
#include "Timekeeper.h"
#include "Pump.h"
#include "Scheduler.h"
#include "WebServerSetup.h"
#include "Ota.h"

void setup(){
  Serial.begin(115200);
  LittleFS.begin(true);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t0=millis(); while(WiFi.status()!=WL_CONNECTED && millis()-t0<20000){ delay(200); Serial.print("."); }
  Serial.println(); Serial.print("IP: "); Serial.println(WiFi.localIP());

  timeSetup();
  webSetup();
  otaSetup();
}

uint32_t lastWS=0;

void loop(){
  pumpLoop();
  schedulerLoop();
  otaHandle();
  if(millis()-lastWS>1000){
    wsBroadcastStatus();
    lastWS=millis();
  }
}
