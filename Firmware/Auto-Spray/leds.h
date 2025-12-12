#pragma once

#include "utils.h"
#include <Arduino.h>

void updateSprayMode(SprayMode currentMode);
void startBlinkConfirm(SprayMode count);
void blinkSprayConfirm();
void updateLed(LedColor red, LedColor green, LedColor blue);

#if ACTIVITY_LED_ENABLED
void updateActivityLed();
#endif
