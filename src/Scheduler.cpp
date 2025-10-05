#include <LittleFS.h>
#include <ArduinoJson.h>
#include "Model.h"
#include "Config.h"
#include "Timekeeper.h"
#include "Scheduler.h"
#include "Pump.h"

static std::vector<ScheduleItem> g_sched;
static uint16_t lastMinSig = 65535;

bool loadSchedule(){
  g_sched.clear();
  File f = LittleFS.open(SCHEDULE_PATH,"r");
  if(!f) return false;
  DynamicJsonDocument doc(8192);
  if(deserializeJson(doc,f)!=DeserializationError::Ok) return false;
  for(JsonObject o: doc.as<JsonArray>()){
    ScheduleItem s; s.pump=o["pump"]|0; s.days=o["days"]|0; s.hour=o["hour"]|0; s.minute=o["minute"]|0; s.ml=o["ml"]|0.0;
    g_sched.push_back(s);
  }
  return true;
}

void saveSchedule(){
  DynamicJsonDocument doc(8192);
  auto arr = doc.to<JsonArray>();
  for(auto &s: g_sched){
    JsonObject o = arr.add<JsonObject>();
    o["pump"]=s.pump; o["days"]=s.days; o["hour"]=s.hour; o["minute"]=s.minute; o["ml"]=s.ml;
  }
  File f = LittleFS.open(SCHEDULE_PATH,"w");
  serializeJson(doc,f);
}

void schedulerLoop(){
  if(!timeSynced()) return;
  int h = currentHour();
  int m = currentMinute();
  uint16_t sig = h*60 + m;
  if(sig==lastMinSig) return;
  lastMinSig = sig;
  int d = currentDOW();
  for(auto &s : g_sched){
    if((s.days & dowMaskFromIndex(d)) && s.hour==h && s.minute==m){
      startRun(s.pump, s.ml, EventType::START);
    }
  }
}

std::vector<ScheduleItem>& scheduleRef(){ return g_sched; }
