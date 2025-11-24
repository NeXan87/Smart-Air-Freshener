#include "config.h"
#include "spray.h"

void startSpray() {
  // Гарантированная остановка перед стартом
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, LOW);
}

void runSpray() {
  for (uint8_t i = 0; i < SPRAY_PULSE_COUNT; i++) {
    digitalWrite(PIN_MOTOR_IN1, HIGH);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    delay(SPRAY_ON_MS);

    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    if (SPRAY_OFF_MS > 0 && i < SPRAY_PULSE_COUNT - 1) delay(SPRAY_OFF_MS);

    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, HIGH);
    delay(SPRAY_REVERSE_MS);

    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    if (i < SPRAY_PULSE_COUNT - 1) delay(100);
  }
}
