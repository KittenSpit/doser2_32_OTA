#pragma once
#include <Arduino.h>
#include "Model.h"

void pumpInit();
void pumpLoadSettings(const PumpSettings s0, const PumpSettings s1);

bool startRun(uint8_t pump, float ml, EventType type);
void stopRun(uint8_t pump);
bool isRunning(uint8_t pump);
float currentDeliveredML(uint8_t pump);
float currentFlow(uint8_t pump);
EventType runningType(uint8_t pump);
void pumpLoop();
