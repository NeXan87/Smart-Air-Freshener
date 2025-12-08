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
  bool d7 = (digitalRead(PIN_SPRAY_1) == LOW);  // D7
  bool d8 = (digitalRead(PIN_SPRAY_2) == LOW);  // D8

  if (d7 && d8) {
    return MODE_3;  // 3 пшика — оба замкнуты
  } else if (d8) {
    return MODE_2;  // 2 пшика — только D8
  } else if (d7) {
    return MODE_1;  // 1 пшик — только D7
  } else {
    return MODE_OFF;  // ничего не замкнуто
  }
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