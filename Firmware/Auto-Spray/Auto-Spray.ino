/********************************************************************
 * Smart Air Freshener – Li-ion Battery Version (3.3V)
 * Platform: Arduino Pro Mini (ATmega328P 8MHz)
 * Author: ChatGPT (functional split version)
 ********************************************************************/

#include "config.h"
#include "leds.h"
#include "spray.h"
#include "state.h"

// -----------------------------------------------------------
// SETUP
// -----------------------------------------------------------
void setup() {
  pinMode(PIN_LIGHT, INPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_MODE, INPUT_PULLUP);

  pinMode(PIN_MOTOR, OUTPUT);
  digitalWrite(PIN_MOTOR, LOW);

  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);

  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  updateLed(LED_RED_OFF, LED_GREEN_OFF, LED_BLUE_OFF);

  // Стартовый писк 1 секунда
  digitalWrite(PIN_BUZZER, HIGH);
  delay(1000);
  digitalWrite(PIN_BUZZER, LOW);

  initStateMachine();
}

// -----------------------------------------------------------
// LOOP
// -----------------------------------------------------------
void loop() {
  updateStateMachine();
}
