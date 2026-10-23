#pragma once
// Copy this file to config.h and edit your local settings before flashing.
// config.h is intentionally excluded from Git.

// ---------- Wi-Fi ----------
// 1 = join your home Wi-Fi (falls back to the arm's hotspot after 15 s)
// 0 = always make the arm's own hotspot
#define WIFI_USE_HOME   0

#define WIFI_SSID       ""
#define WIFI_PASS       ""

// Hotspot used when WIFI_USE_HOME is 0 or home Wi-Fi cannot be reached.
// Set a unique password of at least 8 characters before using the arm.
#define AP_SSID         "RoboArm-Writer"
#define AP_PASS         "change-this-password"

#define HOSTNAME        "roboarm"

// ---------- Hardware ----------
// NodeMCU labels: D2 = GPIO4 (SDA), D1 = GPIO5 (SCL)
#define I2C_SDA         4
#define I2C_SCL         5
#define PCA_ADDR        0x40
#define SERVO_FREQ      50
#define PCA_OSC         27000000

// Hard pulse limits. No servo is sent anything outside this range.
#define US_MIN_HARD     500
#define US_MAX_HARD     2500

// ---------- Motion ----------
#define TICK_MS         20
#define SAFE_Z          15.0f
#define MAX_TEXT        40
#define MAX_CMDS        2500
#define US_PER_SERVO_DEG 11.1f
#define TILT_STEP       3.0f
#define RAW_DEG_S       150.0f
