#include "spray.h"
#include "config.h"

uint8_t sprayPhase = 0;
uint32_t tSpray = 0;

void startSpray() {
  sprayPhase = 0;
  tSpray = millis();
}

bool runSpray() {
  uint32_t now = millis();

  switch (sprayPhase) {
    case 0:
      digitalWrite(PIN_MOTOR, HIGH);
      if (now - tSpray >= SPRAY_PHASE1_MS) {
        digitalWrite(PIN_MOTOR, LOW);
        sprayPhase = 1;
        tSpray = now;
      }
      break;

    case 1:
      if (now - tSpray >= SPRAY_PAUSE_MS) {
        sprayPhase = 2;
        tSpray = now;
      }
      break;

    case 2:
      digitalWrite(PIN_MOTOR, HIGH);
      if (now - tSpray >= SPRAY_PHASE2_MS) {
        digitalWrite(PIN_MOTOR, LOW);
        return true;
      }
      break;
  }
  return false;
}
