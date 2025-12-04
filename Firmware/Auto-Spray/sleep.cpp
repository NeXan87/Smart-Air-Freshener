#include "config.h"

#if ENABLE_SLEEP_MODE

#include "sleep.h"
#include <GyverPower.h>

void maybeSleep(bool lightOn, bool isBlocked) {
  // Спать можно ТОЛЬКО если:
  // - свет ВЫКЛЮЧЕН
  // - нет блокировки
  if (!lightOn && !isBlocked) {
    // Отключаем всю ненужную периферию
    GyverPower::sleepConfig(GyverPower::WakeOn::Interrupt, GyverPower::Peripheral::All);

    // Настраиваем прерывания для пробуждения
    attachInterrupt(
      digitalPinToInterrupt(PIN_BUTTON), []() {}, FALLING);  // D2 — кнопка
    attachInterrupt(
      digitalPinToInterrupt(PIN_LIGHT), []() {}, CHANGE);  // D3 — датчик света

    // Уходим в сон до прерывания
    GyverPower::sleep();

    // После пробуждения — отключаем прерывания, чтобы избежать повторного срабатывания
    detachInterrupt(digitalPinToInterrupt(PIN_BUTTON));
    detachInterrupt(digitalPinToInterrupt(PIN_LIGHT));
  }
}
}

#endif