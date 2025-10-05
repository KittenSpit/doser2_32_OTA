#include <LittleFS.h>
#include <HTTPClient.h>
#include "Model.h"
#include "Config.h"
#include "Timekeeper.h"
#include "Logger.h"

static uint32_t lineCount = 0;

static void ensureLogHeader(){
  if(!LittleFS.exists(LOG_PATH)){
    File f = LittleFS.open(LOG_PATH,"w");
    f.println("datetime,pump,requested_ml,current_ml_per_sec,current_sec,event");
  }
  File f = LittleFS.open(LOG_PATH,"r");
  while(f.available()){ if(f.read()=='\n') lineCount++; }
}

static void rollLogIfNeeded(){
  if(lineCount <= LOG_MAX_LINES) return;
  LittleFS.remove("/log_old.csv");
  LittleFS.rename(LOG_PATH,"/log_old.csv");
  LittleFS.remove(LOG_PATH);
  ensureLogHeader();
  lineCount = 1;
}

static void postRemote(const String& payload){
  if(!REMOTE_LOG_URL || strlen(REMOTE_LOG_URL)==0) return;
  HTTPClient http;
  http.begin(REMOTE_LOG_URL);
  http.addHeader("Content-Type","application/json");
  http.POST(payload);
  http.end();
}

void loggerSetup(){ ensureLogHeader(); }

void logEvent(EventType t, uint16_t pump, float reqML, float mlps, float sec, const char* when){
  String ts = when? String(when) : isoNow();
  String line = ts+","+String(pump)+","+String(reqML,3)+","+String(mlps,3)+","+String(sec,3)+","+evtName(t);
  File f = LittleFS.open(LOG_PATH,"a"); f.println(line); f.close(); lineCount++; rollLogIfNeeded();
  String j = String("{")
    +"\"datetime\":\""+ts+"\","
    +"\"pump\":"+String(pump)+","
    +"\"requested_ml\":"+String(reqML,3)+","
    +"\"current_ml_per_sec\":"+String(mlps,3)+","
    +"\"current_sec\":"+String(sec,3)+","
    +"\"event\":\""+evtName(t)+"\""
    +"}";
  postRemote(j);
}

void logStartup(){
  loggerSetup();
  logEvent(EventType::INFO, 999, -1, -1, -1, isoNow().c_str());
}

void logStartLike(uint8_t pump, float targetML, float mlps){ logEvent(EventType::START, pump, targetML, mlps, 0); }
void logStopLike(uint8_t pump, float deliveredML, float mlps, float sec){ logEvent(EventType::STOP, pump, deliveredML, mlps, sec); }
