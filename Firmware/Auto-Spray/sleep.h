#pragma once

#if ENABLE_SLEEP_MODE
#include <Arduino.h>
#include "config.h"

// Проверяет условия и, если возможно, переводит МК в сон
void maybeSleep(bool lightOn, bool isBlocked);

// Инициализирует прерывания для пробуждения
void initSleepMode();

#endif