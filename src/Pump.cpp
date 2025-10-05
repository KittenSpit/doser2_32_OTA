#include <Arduino.h>
#include "Pins.h"
#include "Model.h"
#include "Pump.h"

static PumpSettings g_pump[2];
static ActiveJob g_job[2];

static void pumpApplyPWM(uint8_t idx, uint8_t pwm){
  uint8_t chan = (idx==0)?0:1;
  uint32_t duty = map(pwm,0,255,0,(1<<PWM_RES_BITS)-1);
  ledcWrite(chan,duty);
}

static void pumpSetDir(uint8_t idx, bool forward){
  digitalWrite(idx==0?P0_DIR:P1_DIR, forward?HIGH:LOW);
}

void pumpInit(){
  ledcSetup(0, PWM_FREQ, PWM_RES_BITS);
  ledcSetup(1, PWM_FREQ, PWM_RES_BITS);
  ledcAttachPin(P0_PWM, 0);
  ledcAttachPin(P1_PWM, 1);
  pinMode(P0_DIR, OUTPUT);
  pinMode(P1_DIR, OUTPUT);
  pumpApplyPWM(0,0); pumpApplyPWM(1,0);
}

void pumpLoadSettings(const PumpSettings s0, const PumpSettings s1){
  g_pump[0]=s0; g_pump[1]=s1;
}

bool startRun(uint8_t pump, float ml, EventType type){
  if(pump>1) return false;
  if(g_job[pump].active) return false;
  bool forward = (type!=EventType::PURGE);
  pumpSetDir(pump, forward);
  pumpApplyPWM(pump, g_pump[pump].pwm);
  g_job[pump] = {};
  g_job[pump].active = true;
  g_job[pump].type = type;
  g_job[pump].target_ml = ml;
  g_job[pump].ml_per_sec = g_pump[pump].ml_per_sec;
  g_job[pump].start_ms = millis();
  g_job[pump].delivered_ml = 0;
  return true;
}

void stopRun(uint8_t pump){
  if(pump>1) return;
  if(!g_job[pump].active) return;
  pumpApplyPWM(pump, 0);
  g_job[pump].active = false;
}

bool isRunning(uint8_t pump){ return (pump<=1) ? g_job[pump].active : false; }
float currentDeliveredML(uint8_t pump){ return (pump<=1) ? g_job[pump].delivered_ml : 0; }
float currentFlow(uint8_t pump){ return (pump<=1) ? g_job[pump].ml_per_sec : 0; }
EventType runningType(uint8_t pump){ return (pump<=1) ? g_job[pump].type : EventType::INFO; }

void pumpLoop(){
  static uint32_t last = millis();
  uint32_t now = millis();
  uint32_t dt = now - last; last = now;
  for(uint8_t p=0;p<2;p++){
    if(!g_job[p].active) continue;
    g_job[p].delivered_ml += (g_job[p].ml_per_sec) * (dt/1000.0f);
    if(g_job[p].type==EventType::START || g_job[p].type==EventType::PRIME || g_job[p].type==EventType::PURGE){
      if(g_job[p].target_ml > 0 && g_job[p].delivered_ml >= g_job[p].target_ml){
        stopRun(p);
      }
    }
  }
}
