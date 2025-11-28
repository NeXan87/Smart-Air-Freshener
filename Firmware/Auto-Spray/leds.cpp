#include "config.h"
#include "leds.h"
#include <Arduino.h>

void updateLed(LedColor red, LedColor green, LedColor blue) {
#if LED_COMMON_ANODE
  // Общий анод: LOW = включить, HIGH = выключить
  digitalWrite(PIN_LED_R, (red == LED_RED_ON) ? LOW : HIGH);
  digitalWrite(PIN_LED_G, (green == LED_GREEN_ON) ? LOW : HIGH);
  digitalWrite(PIN_LED_B, (blue == LED_BLUE_ON) ? LOW : HIGH);
#else
  // Общий катод: HIGH = включить, LOW = выключить
  digitalWrite(PIN_LED_R, (red == LED_RED_ON) ? HIGH : LOW);
  digitalWrite(PIN_LED_G, (green == LED_GREEN_ON) ? HIGH : LOW);
  digitalWrite(PIN_LED_B, (blue == LED_BLUE_ON) ? HIGH : LOW);
#endif
}

#if ACTIVITY_LED_ENABLED

void updateActivityLed(bool canSleep) {
  static uint32_t lastToggle = 0;
  static bool ledState = false;
  uint32_t now = millis();

  if (canSleep) {
    if (ledState) {
      digitalWrite(PIN_LED_BUILTIN, LOW);
      ledState = false;
    }
    return;
  }

  if (ledState) {
    if (now - lastToggle >= ACTIVITY_LED_ON_MS) {
      digitalWrite(PIN_LED_BUILTIN, LOW);
      ledState = false;
      lastToggle = now;
    }
  } else {
    if (now - lastToggle >= ACTIVITY_LED_OFF_MS) {
      digitalWrite(PIN_LED_BUILTIN, HIGH);
      ledState = true;
      lastToggle = now;
    }
  }
}

#else

// Заглушка, если отключено
void updateActivityLed(bool canSleep) {
  (void)canSleep;
}

#endif