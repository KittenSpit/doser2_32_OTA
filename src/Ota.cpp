#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "Config.h"
#include "Ota.h"

void otaSetup(){
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  if (strlen(OTA_PASSWORD) > 0) ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA
    .onStart([](){
      String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
      Serial.printf("[OTA] Start updating %s\n", type.c_str());
    })
    .onEnd([](){
      Serial.println("\n[OTA] End");
    })
    .onProgress([](unsigned int progress, unsigned int total){
      Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error){
      Serial.printf("[OTA] Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
  Serial.println("[OTA] Ready");
  Serial.print("[OTA] Hostname: "); Serial.println(OTA_HOSTNAME);
}

void otaHandle(){
  ArduinoOTA.handle();
}
