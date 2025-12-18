#include "config.h"

#if ENABLE_SLEEP_MODE

#include "sleep.h"
#include "state.h"
#include "battery.h"
#include "leds.h"
#include <GyverPower.h>

void initSleepMode() {
  power.setSleepMode(POWERDOWN_SLEEP);
}

void sleepWDT() {
  disableInputPullups();
  power.sleep(SLEEP_1024MS);
  enableInputPullups();
}

void maybeSleep(bool isLightOn) {
  // Спать можно ТОЛЬКО если:
  // - свет ВЫКЛЮЧЕН
  // - не пшикает
  // - нет блокировки
  bool isIdle = (currentState == STATE_IDLE);
  bool isFullBlocked = isBatLow() && currentState == STATE_BLOCKED;

  if ((!isLightOn && isIdle) || (!isLightOn && isFullBlocked)) {
    disableInputPullups();
    attachInterrupt(
      digitalPinToInterrupt(PIN_BUTTON), []() {}, FALLING);
    attachInterrupt(
      digitalPinToInterrupt(PIN_LIGHT), []() {}, CHANGE);

    disableOutputPins();
    updateLed(LED_RED_OFF, LED_GREEN_OFF, LED_BLUE_OFF);
    power.sleep(SLEEP_FOREVER);

    enableInputPullups();
    detachInterrupt(digitalPinToInterrupt(PIN_BUTTON));
    detachInterrupt(digitalPinToInterrupt(PIN_LIGHT));
  }
}

#endif