#include "config.h"

#if ENABLE_SLEEP_MODE

#include "sleep.h"
#include <GyverPower.h>

GyverPower power;

void initSleepMode() {
  // Опционально: калибровка (для точного таймера сна)
  // power.autoCalibrate();

  // Отключаем ненужную периферию
  power.hardwareDisable(PWR_ADC | PWR_UART | PWR_WIRE | PWR_TIMER0 | PWR_TIMER1 | PWR_TIMER2);

  // Устанавливаем режим сна (по умолчанию и так POWERDOWN)
  power.setSleepMode(POWERDOWN_SLEEP);

  // BOD по умолчанию выключен — хорошо для экономии
}

void wakeUpButton() {
  power.wakeUp();  // ← обязательно!
}

void wakeUpLight() {
  power.wakeUp();  // ← обязательно!
}

void maybeSleep(bool lightOn, bool isBlocked) {
  // Спать можно ТОЛЬКО если:
  // - свет ВЫКЛЮЧЕН
  // - нет блокировки
  if (!lightOn && !isBlocked) {
    // Настраиваем прерывания для пробуждения
    attachInterrupt(
      digitalPinToInterrupt(PIN_BUTTON), wakeUpButton, FALLING);  // D2 — кнопка
    attachInterrupt(
      digitalPinToInterrupt(PIN_LIGHT), wakeUpLight, CHANGE);  // D3 — датчик света

    // Уходим в сон до прерывания
    power.sleep();

    // После пробуждения — отключаем прерывания, чтобы избежать повторного срабатывания
    detachInterrupt(digitalPinToInterrupt(PIN_BUTTON));
    detachInterrupt(digitalPinToInterrupt(PIN_LIGHT));
  }
}

#endif