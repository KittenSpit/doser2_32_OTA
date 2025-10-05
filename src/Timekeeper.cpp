#include <WiFi.h>
#include <time.h>
#include "Config.h"
#include "Timekeeper.h"

static bool s_timeReady = false;

void timeSetup(){
  configTime(0,0,"pool.ntp.org","time.nist.gov");
  setenv("TZ", TZ_EASTERN, 1);
  tzset();
}

bool timeSynced(){
  if(s_timeReady) return true;
  time_t now; time(&now);
  if(now > 1700000000) { s_timeReady = true; }
  return s_timeReady;
}

String isoNow(){
  time_t now = time(nullptr);
  struct tm tm; localtime_r(&now, &tm);
  char buf[32]; strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S", &tm);
  return String(buf);
}

int currentDOW(){ time_t now=time(nullptr); struct tm tm; localtime_r(&now,&tm); return tm.tm_wday; }
int currentHour(){ time_t now=time(nullptr); struct tm tm; localtime_r(&now,&tm); return tm.tm_hour; }
int currentMinute(){ time_t now=time(nullptr); struct tm tm; localtime_r(&now,&tm); return tm.tm_min; }
