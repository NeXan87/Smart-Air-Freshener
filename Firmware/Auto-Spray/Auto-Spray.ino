/********************************************************************
 * Smart Air Freshener
 * Platform: Arduino Pro Mini (ATmega328P 8MHz, 3.3V)
 ********************************************************************/

#include "config.h"
#include "leds.h"
#include "spray.h"
#include "sleep.h"
#include "state.h"
#include "opt3001.h"
#include "utils.h"

#if ENABLE_SLEEP_MODE
#include "sleep.h"
#endif

void runStartupSequence() {
  tone(PIN_BUZZER, FREQ_SQUEAKER);  // Включаем писк

  updateLed(LED_RED_ON, LED_GREEN_OFF, LED_BLUE_OFF);
  delay(STARTUP_DELAY_MS);

  updateLed(LED_RED_OFF, LED_GREEN_ON, LED_BLUE_OFF);
  delay(STARTUP_DELAY_MS);

  updateLed(LED_RED_OFF, LED_GREEN_OFF, LED_BLUE_ON);
  delay(STARTUP_DELAY_MS);

  updateLed(LED_RED_OFF, LED_GREEN_OFF, LED_BLUE_OFF);
  delay(STARTUP_DELAY_MS);

  noTone(PIN_BUZZER);  // Выключаем писк
}

// -----------------------------------------------------------
// SETUP
// -----------------------------------------------------------
void setup() {
  // Serial.begin(9600);
#if USE_OPT3001
  pinMode(PIN_LIGHT, INPUT_PULLUP);
#else
  pinMode(PIN_LIGHT, INPUT);
#endif

  pinMode(PIN_MOTOR_IN1, OUTPUT);
  pinMode(PIN_MOTOR_IN2, OUTPUT);
  pinMode(PIN_ADD_LED, OUTPUT);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_BUILTIN, OUTPUT);
  pinMode(PIN_MODE, INPUT_PULLUP);
  pinMode(PIN_SPRAY_1, INPUT_PULLUP);
  pinMode(PIN_SPRAY_2, INPUT_PULLUP);

  digitalWrite(PIN_ADD_LED, LOW);
  digitalWrite(PIN_LED_BUILTIN, LOW);
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, LOW);

  runStartupSequence();
  initStateMachine();

#if ENABLE_SLEEP_MODE
  initSleepMode();
#endif

#if USE_OPT3001
  initOpt3001();
#endif
}

// -----------------------------------------------------------
// LOOP
// -----------------------------------------------------------
void loop() {
  SprayMode mode = getCurrentMode();

  updateStateMachine();

  if (checkSprayMode(mode)) {
    startBlinkConfirm(mode);
  }
  blinkSprayConfirm();

#if ENABLE_SLEEP_MODE
  bool lightOn = isLightOn();
  bool isNotBlocked = (currentState != STATE_BLOCKED);
  bool canSleep = (!lightOn && isNotBlocked);
#else
  bool canSleep = false;
#endif

#if ACTIVITY_LED_ENABLED
  updateActivityLed(canSleep);
#endif

#if ENABLE_SLEEP_MODE
  maybeSleep(lightOn, isNotBlocked);
#endif
}
