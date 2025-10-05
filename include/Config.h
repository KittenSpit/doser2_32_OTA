#pragma once
#include <Arduino.h>

// Timezone for Eastern with DST
#define TZ_EASTERN "EST5EDT,M3.2.0/2,M11.1.0/2"

// ===== WiFi (fill in) =====
static const char* WIFI_SSID = "OtisandLily";
static const char* WIFI_PASS = "SugarLake42!!";

// ===== Storage paths =====
#define LOG_PATH        "/log.csv"
#define SETTINGS_PATH   "/settings.json"
#define SCHEDULE_PATH   "/schedule.json"

// Cap CSV lines before rollover
#define LOG_MAX_LINES   5000

// Optional remote webhook to your PHP endpoint (empty to disable)
static const char* REMOTE_LOG_URL = ""; // e.g. "https://example.com/doser_log.php"

// OTA
#define OTA_HOSTNAME "aquadoser-esp32"
// Optional: set a password or leave empty for none
#define OTA_PASSWORD ""
