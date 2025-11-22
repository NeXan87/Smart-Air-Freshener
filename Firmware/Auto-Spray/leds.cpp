#include "leds.h"
#include <Arduino.h>

void updateLed(LedColor red, LedColor green, LedColor blue) {
  digitalWrite(PIN_LED_R, red == LED_RED_ON ? LOW : HIGH);
  digitalWrite(PIN_LED_G, green == LED_GREEN_ON ? LOW : HIGH);
  digitalWrite(PIN_LED_B, blue == LED_BLUE_ON ? LOW : HIGH);
}
