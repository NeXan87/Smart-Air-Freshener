#include "config.h"
#include "utils.h"

bool checkSprayMode(SprayMode currentMode) {
  static SprayMode lastSprayMode = MODE_OFF;

  if (currentMode != lastSprayMode) {
    lastSprayMode = currentMode;
    return true;
  }
  return false;
}

SprayMode getCurrentMode() {
  if (digitalRead(PIN_SPRAY_2) == LOW) return MODE_1;
  if (digitalRead(PIN_SPRAY_3) == LOW) return MODE_2;
  if (digitalRead(PIN_SPRAY_4) == LOW) return MODE_3;
  return MODE_OFF;
}

void disableOutputPins() {
  noTone(PIN_BUZZER);
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, LOW);
  digitalWrite(PIN_LED_BUILTIN, LOW);

#if LED_COMMON_ANODE
  digitalWrite(PIN_LED_R, HIGH);
  digitalWrite(PIN_LED_G, HIGH);
  digitalWrite(PIN_LED_B, HIGH);
#else
  digitalWrite(PIN_LED_R, LOW);
  digitalWrite(PIN_LED_G, LOW);
  digitalWrite(PIN_LED_B, LOW);
#endif
}