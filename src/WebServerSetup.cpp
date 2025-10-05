#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "Model.h"
#include "Pump.h"
#include "Scheduler.h"
#include "Logger.h"

static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");

static PumpSettings s0, s1;

bool loadSettings(){
  if(!LittleFS.exists(SETTINGS_PATH)) return false;
  File f = LittleFS.open(SETTINGS_PATH,"r");
  DynamicJsonDocument d(2048); if(deserializeJson(d,f)!=DeserializationError::Ok) return false;
  s0.ml_per_sec = d["p0"]["mlps"]|1.0; s0.pwm = d["p0"]["pwm"]|180;
  s1.ml_per_sec = d["p1"]["mlps"]|1.0; s1.pwm = d["p1"]["pwm"]|180;
  return true;
}

void saveSettings(){
  DynamicJsonDocument d(2048);
  JsonObject o0 = d.createNestedObject("p0"); o0["mlps"]=s0.ml_per_sec; o0["pwm"]=s0.pwm;
  JsonObject o1 = d.createNestedObject("p1"); o1["mlps"]=s1.ml_per_sec; o1["pwm"]=s1.pwm;
  File f = LittleFS.open(SETTINGS_PATH,"w"); serializeJsonPretty(d,f);
}

void wifiConnect(){
  WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID,WIFI_PASS);
  uint32_t t0=millis(); while(WiFi.status()!=WL_CONNECTED && millis()-t0<20000){ delay(200); }
}

static void handleWS(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
  if(type==WS_EVT_CONNECT){
    // initial snapshot is pushed by wsBroadcastStatus() from main loop
  }
}

void wsBroadcastStatus(){
  DynamicJsonDocument d(512);
  d["ip"] = WiFi.localIP().toString();
  for(int p=0;p<2;p++){
    JsonObject po = d.createNestedObject(String("p")+p);
    po["running"] = isRunning(p);
    po["type"] = evtName(runningType(p));
    po["mlps"] = currentFlow(p);
    po["delivered_ml"] = currentDeliveredML(p);
  }
  String out; serializeJson(d,out);
  ws.textAll(out);
}

void webSetup(){
  pumpInit();
  loadSettings();
  pumpLoadSettings(s0,s1);

  server.addHandler(&ws);
  ws.onEvent(handleWS);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* r){ r->send(LittleFS, "/index.html", "text/html"); });
  server.on("/log", HTTP_GET, [](AsyncWebServerRequest* r){ r->send(LittleFS, "/log.html", "text/html"); });
  server.serveStatic("/app.css", LittleFS, "/app.css");
  server.serveStatic("/app.js", LittleFS, "/app.js");
  server.serveStatic("/log.js", LittleFS, "/log.js");

  // status
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* r){
    DynamicJsonDocument d(512);
    d["ip"]=WiFi.localIP().toString();
    for(int p=0;p<2;p++){
      JsonObject po = d.createNestedObject(String("p")+p);
      po["running"] = isRunning(p);
      po["type"] = evtName(runningType(p));
      po["delivered_ml"] = currentDeliveredML(p);
      po["mlps"] = currentFlow(p);
    }
    AsyncResponseStream* s = r->beginResponseStream("application/json");
    serializeJson(d,*s); r->send(s);
  });

  // settings
  server.on("/api/settings", HTTP_GET, [](AsyncWebServerRequest* r){
    DynamicJsonDocument d(512);
    d["p0"]["mlps"]=s0.ml_per_sec; d["p0"]["pwm"]=s0.pwm;
    d["p1"]["mlps"]=s1.ml_per_sec; d["p1"]["pwm"]=s1.pwm;
    AsyncResponseStream* s = r->beginResponseStream("application/json");
    serializeJson(d,*s); r->send(s);
  });
  server.on("/api/settings", HTTP_POST, [](AsyncWebServerRequest* r){ r->send(200,"text/plain","OK"); },
    NULL, [](AsyncWebServerRequest* r, uint8_t* data, size_t len, size_t, size_t){
      DynamicJsonDocument d(1024);
      if(deserializeJson(d,data,len)==DeserializationError::Ok){
        s0.ml_per_sec = d["p0"]["mlps"]|s0.ml_per_sec; s0.pwm = d["p0"]["pwm"]|s0.pwm;
        s1.ml_per_sec = d["p1"]["mlps"]|s1.ml_per_sec; s1.pwm = d["p1"]["pwm"]|s1.pwm;
        saveSettings();
        pumpLoadSettings(s0,s1);
      }
    });

  // schedule
  server.on("/api/schedule", HTTP_GET, [](AsyncWebServerRequest* r){
    DynamicJsonDocument d(8192);
    auto arr = d.to<JsonArray>();
    for(auto &s: scheduleRef()){
      JsonObject o = arr.add<JsonObject>();
      o["pump"]=s.pump; o["days"]=s.days; o["hour"]=s.hour; o["minute"]=s.minute; o["ml"]=s.ml;
    }
    AsyncResponseStream* s = r->beginResponseStream("application/json");
    serializeJson(d,*s); r->send(s);
  });
  server.on("/api/schedule", HTTP_POST, [](AsyncWebServerRequest* r){ r->send(200,"text/plain","OK"); },
    NULL, [](AsyncWebServerRequest* r, uint8_t* data, size_t len, size_t, size_t){
      DynamicJsonDocument d(8192);
      if(deserializeJson(d,data,len)==DeserializationError::Ok){
        auto &vec = scheduleRef(); vec.clear();
        for(JsonObject o: d.as<JsonArray>()){
          ScheduleItem s; s.pump=o["pump"]|0; s.days=o["days"]|0; s.hour=o["hour"]|0; s.minute=o["minute"]|0; s.ml=o["ml"]|0.0; vec.push_back(s);
        }
        saveSchedule();
      }
    });

  // commands
  server.on("/api/run", HTTP_POST, [](AsyncWebServerRequest* r){ r->send(200,"text/plain","OK"); },
    NULL, [](AsyncWebServerRequest* r, uint8_t* data, size_t len, size_t, size_t){
      DynamicJsonDocument d(512); if(deserializeJson(d,data,len)!=DeserializationError::Ok) return;
      uint8_t pump=d["pump"]|0; float ml=d["ml"]|0; if(ml<=0) return;
      if(startRun(pump, ml, EventType::START)){ logStartLike(pump, ml, currentFlow(pump)); wsBroadcastStatus(); }
    });

  server.on("/api/prime", HTTP_POST, [](AsyncWebServerRequest* r){ r->send(200,"text/plain","OK"); },
    NULL, [](AsyncWebServerRequest* r, uint8_t* data, size_t len, size_t, size_t){
      DynamicJsonDocument d(512); if(deserializeJson(d,data,len)!=DeserializationError::Ok) return;
      uint8_t pump=d["pump"]|0; float sec=d["sec"]|5;
      float ml = sec * currentFlow(pump);
      if(startRun(pump, ml, EventType::PRIME)){ logStartLike(pump, ml, currentFlow(pump)); wsBroadcastStatus(); }
    });

  server.on("/api/purge", HTTP_POST, [](AsyncWebServerRequest* r){ r->send(200,"text/plain","OK"); },
    NULL, [](AsyncWebServerRequest* r, uint8_t* data, size_t len, size_t, size_t){
      DynamicJsonDocument d(512); if(deserializeJson(d,data,len)!=DeserializationError::Ok) return;
      uint8_t pump=d["pump"]|0; float sec=d["sec"]|5;
      float ml = sec * currentFlow(pump);
      if(startRun(pump, ml, EventType::PURGE)){ logStartLike(pump, ml, currentFlow(pump)); wsBroadcastStatus(); }
    });

  server.on("/api/stop", HTTP_POST, [](AsyncWebServerRequest* r){
    r->send(200,"text/plain","OK");
  }, NULL, [](AsyncWebServerRequest* r, uint8_t* data, size_t len, size_t, size_t){
    (void)data; (void)len;
    for(uint8_t p=0;p<2;p++){
      if(isRunning(p)){
        float ml=currentDeliveredML(p);
        float mlps=currentFlow(p);
        stopRun(p);
        logStopLike(p, ml, mlps, 0);
      }
    }
    wsBroadcastStatus();
  });

  // logs
  server.on("/api/log.csv", HTTP_GET, [](AsyncWebServerRequest* r){
    r->send(LittleFS, LOG_PATH, "text/csv", true);
  });
  server.on("/api/log_clear", HTTP_POST, [](AsyncWebServerRequest* r){
    LittleFS.remove(LOG_PATH);
    File f = LittleFS.open(LOG_PATH,"w");
    f.println("datetime,pump,requested_ml,current_ml_per_sec,current_sec,event");
    f.close();
    r->send(200,"text/plain","CLEARED");
  });

  server.begin();
  logStartup();
}
