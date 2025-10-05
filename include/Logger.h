#pragma once
#include "Model.h"

void loggerSetup();
void logStartup();
void logStartLike(uint8_t pump, float targetML, float mlps);
void logStopLike(uint8_t pump, float deliveredML, float mlps, float sec);
void logEvent(EventType t, uint16_t pump, float reqML, float mlps, float sec, const char* when=nullptr);
