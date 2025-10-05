#pragma once
#include <Arduino.h>

// DRV8871 in PH/EN mode (PH = direction, EN = PWM)

// Pump 0
#define P0_PWM  18
#define P0_DIR  19

// Pump 1
#define P1_PWM  23
#define P1_DIR  22

// PWM config
#define PWM_FREQ     2000
#define PWM_RES_BITS 10
