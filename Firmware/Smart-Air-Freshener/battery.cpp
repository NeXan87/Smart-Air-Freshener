#include "config.h"
#include "battery.h"
#include "leds.h"
#include "state.h"
#include "utils.h"

uint16_t vbat = 9999;

uint16_t readBatteryVoltage() {
  enableADC();

  uint16_t adc = analogRead(PIN_BATTERY_POINT);
  float vout_mv = adc * VCC_ARDUINO / 1023;
  uint16_t vbat_mv = (uint16_t)(vout_mv * BATTERY_DIVIDER_RATIO);

  disableADC();

  return vbat_mv;
}

void updateBattery(bool isLightOn) {
  static uint32_t lastBatteryCheck = 0;
  static bool isBatteryLow = false;
  static bool isFirstCheck = true;
  static bool isBatter = true;

  if (isFirstCheck || (isLightOn && currentState != STATE_SPRAY && millis() - lastBatteryCheck >= BATTERY_CHECK_INTERVAL_MS)) {
    vbat = readBatteryVoltage();

    if (vbat <= BATTERY_LOW_MV) {
      isBatteryLow = true;
    } else {
      isBatteryLow = false;
    }

    lastBatteryCheck = millis();
    isFirstCheck = false;
  }

  if (isLightOn) {
    if (isBatteryLow) {
      updateBatteryLed();
    }
  } else {
    digitalWrite(PIN_ADD_LED, LOW);
  }
}

bool isBatLow() {
  // return false;  // для дебага
  return vbat <= BATTERY_BLOCKED_MV;
}