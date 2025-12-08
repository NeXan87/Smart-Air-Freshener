#pragma once

#include "utils.h"
#include <Arduino.h>

void startBlinkConfirm(SprayMode mode);
void blinkSprayConfirm();
void updateLed(LedColor red, LedColor green, LedColor blue);
void updateActivityLed(bool canSleep);
