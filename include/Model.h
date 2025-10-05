#pragma once
#include <Arduino.h>

// Event types
enum class EventType { START, STOP, PRIME, PURGE, INFO };

inline const char* evtName(EventType t){
  switch(t){
    case EventType::START: return "start";
    case EventType::STOP:  return "stop";
    case EventType::PRIME: return "prime";
    case EventType::PURGE: return "purge";
    default: return "info";
  }
}

struct PumpSettings {
  float   ml_per_sec = 1.0f;
  uint8_t pwm        = 180;  // 0..255
};

struct ScheduleItem {
  uint8_t pump   = 0;
  uint8_t days   = 0;    // bitmask bit0=Sun .. bit6=Sat
  uint8_t hour   = 8;
  uint8_t minute = 0;
  float   ml     = 5.0f;
};

struct ActiveJob {
  bool      active       = false;
  EventType type         = EventType::START;
  float     target_ml    = 0;
  float     ml_per_sec   = 0;
  uint32_t  start_ms     = 0;
  float     delivered_ml = 0;
};

inline uint8_t dowMaskFromIndex(int dow){ return (1 << (dow & 7)); }
