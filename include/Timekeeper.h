#pragma once
#include <Arduino.h>
void timeSetup();
bool timeSynced();
String isoNow();
int currentDOW();     // 0=Sun..6=Sat
int currentHour();
int currentMinute();
