#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <Bounce2.h>

// === ПИНЫ ===
#define PIN_BUTTON 2         // D2 — кнопка (GND при нажатии)
#define PIN_LIGHT_DIGITAL 3  // D3 — цифровой выход датчика света (HIGH = светло)
#define PIN_MOTOR 4
#define PIN_LED_BLOCK 5         // red
#define PIN_AUTO_MODE_SWITCH 6  // LOW = автоматический режим включён
#define PIN_LED_WORK 7          // green
#define PIN_LED_READY 8         // blue
#define PIN_BUZZER 9

// === РЕЖИМ ===
#define SPRAY_ON_TIMER_ONLY  // раскомментируйте для пшика сразу по таймеру

// === КОНСТАНТЫ ===
#define BLOCK_DURATION_SEC (30UL * 60UL)   // 30 минут
#define AUTO_READY_DELAY_SEC (3UL * 60UL)  // 3 минуты
#define SPRAY_PULSE_COUNT 2
#define SPRAY_PULSE_DURATION_MS 4
#define SPRAY_INTER_PULSE_DELAY_MS 10
#define BUZZER_ON_DURATION_MS 300
#define LED_BLINK_INTERVAL_MS 1000

// === ФЛАГИ ПРОБУЖДЕНИЯ ===
volatile bool wokeByLight = false;
volatile bool wdtTriggered = false;

// === СОСТОЯНИЕ ===
static Bounce buttonDebouncer = Bounce();  // debounce для кнопки
static bool isBlocked = false;
static bool isAutoReady = false;
static uint32_t blockStartTime = 0;    // секунды
static uint32_t lightOnStartTime = 0;  // секунды
static uint32_t lastLedToggle = 0;     // миллисекунды
static bool isBuzzerOn = false;
static uint32_t buzzerOffTime = 0;  // миллисекунды
static bool systemAwake = true;     // true = активен, false = в соне (логически)

// === ПРЕРЫВАНИЯ ===

ISR(INT0_vect) {
  wokeByLight = true;
}
ISR(WDT_vect) {
  wdtTriggered = true;
}

// === ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ===

void setupWatchdogForBlocking() {
  cli();
  wdt_reset();
  WDTCSR |= _BV(WDCE) | _BV(WDE);
  WDTCSR = _BV(WDIE) | _BV(WDP2) | _BV(WDP1);  // 1 сек
  sei();
}

void disableWatchdog() {
  cli();
  wdt_reset();
  WDTCSR |= _BV(WDCE) | _BV(WDE);
  WDTCSR = 0;
  sei();
}

void sleepForever() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
  cli();
}

void performSpray() {
  digitalWrite(PIN_LED_WORK, HIGH);
  for (uint8_t i = 0; i < SPRAY_PULSE_COUNT; i++) {
    digitalWrite(PIN_MOTOR, HIGH);
    delay(SPRAY_PULSE_DURATION_MS);
    digitalWrite(PIN_MOTOR, LOW);
    if (i < SPRAY_PULSE_COUNT - 1) {
      delay(SPRAY_INTER_PULSE_DELAY_MS);
    }
  }
  digitalWrite(PIN_LED_WORK, LOW);
}

void enterBlockedState() {
  isBlocked = true;
  isAutoReady = false;
  blockStartTime = millis() / 1000UL;
  digitalWrite(PIN_BUZZER, LOW);
  isBuzzerOn = false;
  setupWatchdogForBlocking();  // включаем WDT для отсчёта блокировки
}

void updateLeds() {
  uint32_t now = millis();
  bool isLightOn = (digitalRead(PIN_LIGHT_DIGITAL) == HIGH);

  // Синий LED
  if (isAutoReady && isLightOn) {
    digitalWrite(PIN_LED_READY, HIGH);
  } else if (!isBlocked && isLightOn) {
    if (now - lastLedToggle >= LED_BLINK_INTERVAL_MS) {
      lastLedToggle = now;
      digitalWrite(PIN_LED_READY, !digitalRead(PIN_LED_READY));
    }
  } else {
    digitalWrite(PIN_LED_READY, LOW);
  }

  // Красный LED
  if (isBlocked && isLightOn) {
    if (now - lastLedToggle >= LED_BLINK_INTERVAL_MS) {
      lastLedToggle = now;
      digitalWrite(PIN_LED_BLOCK, !digitalRead(PIN_LED_BLOCK));
    }
  } else {
    digitalWrite(PIN_LED_BLOCK, LOW);
  }
}

void updateBuzzer() {
  if (isBuzzerOn && millis() >= buzzerOffTime) {
    digitalWrite(PIN_BUZZER, LOW);
    isBuzzerOn = false;
  }
}

void handleLightChange() {
  bool isLightOn = (digitalRead(PIN_LIGHT_DIGITAL) == HIGH);
  uint32_t nowSec = millis() / 1000UL;

  if (isLightOn) {
    // Свет включён — начинаем отсчёт
    if (lightOnStartTime == 0) lightOnStartTime = nowSec;
    uint32_t elapsed = nowSec - lightOnStartTime;
    if (!isBlocked && elapsed >= AUTO_READY_DELAY_SEC && !isAutoReady) {
      isAutoReady = true;
#ifdef SPRAY_ON_TIMER_ONLY
      performSpray();
      enterBlockedState();
#else
      isBuzzerOn = true;
      buzzerOffTime = millis() + BUZZER_ON_DURATION_MS;
      digitalWrite(PIN_BUZZER, HIGH);
#endif
    }
  } else {
    // Свет выключен
    lightOnStartTime = 0;
#ifndef SPRAY_ON_TIMER_ONLY
    if (isAutoReady && !isBlocked) {
      performSpray();
      enterBlockedState();
    }
#endif
    isAutoReady = false;
  }
}

// Проверяет, можно ли заснуть
void goToSleepIfNeeded() {
  bool isLightOn = (digitalRead(PIN_LIGHT_DIGITAL) == HIGH);
  uint32_t nowSec = millis() / 1000UL;

  // Если свет включён — не спим
  if (isLightOn) return;

  // Если есть блокировка — ждём её окончания
  if (isBlocked) {
    if (nowSec - blockStartTime < BLOCK_DURATION_SEC) {
      return;  // ещё не время спать
    } else {
      isBlocked = false;
      disableWatchdog();  // блокировка закончилась — выключаем WDT
    }
  }

  // Свет выключен, блокировки нет → можно спать
  digitalWrite(PIN_LED_READY, LOW);
  digitalWrite(PIN_LED_BLOCK, LOW);
  digitalWrite(PIN_BUZZER, LOW);
  systemAwake = false;
  sleepForever();  // засыпаем НАВСЕГДА, пока не прервут
}

// === SETUP / LOOP ===

void setup() {
  // Отключаем периферию
  PRR0 = _BV(PRTIM1) | _BV(PRTIM0) | _BV(PRUSART1) | _BV(PRSPI);
  PRR1 = _BV(PRTIM3) | _BV(PRTIM4) | _BV(PRTWI);

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_LIGHT_DIGITAL, INPUT);
  pinMode(PIN_MOTOR, OUTPUT);
  pinMode(PIN_LED_BLOCK, OUTPUT);
  pinMode(PIN_AUTO_MODE_SWITCH, INPUT_PULLUP);
  pinMode(PIN_LED_WORK, OUTPUT);
  pinMode(PIN_LED_READY, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  digitalWrite(PIN_MOTOR, LOW);
  digitalWrite(PIN_LED_BLOCK, LOW);
  digitalWrite(PIN_LED_WORK, LOW);
  digitalWrite(PIN_LED_READY, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  // Настройка Bounce2 для кнопки
  buttonDebouncer.attach(PIN_BUTTON);
  buttonDebouncer.interval(25);  // 25 мс debounce

  // Прерывание по свету (D3)
  EICRA = _BV(ISC01);  // по спаду (если свет = HIGH, темнота = LOW)
  EIMSK = _BV(INTF0);

  // Сигнал включения
  digitalWrite(PIN_BUZZER, HIGH);
  delay(200);
  digitalWrite(PIN_BUZZER, LOW);
}

void loop() {
  buttonDebouncer.update();

  // Обработка нажатия кнопки
  if (buttonDebouncer.fell()) {
    if (!isBlocked && digitalRead(PIN_AUTO_MODE_SWITCH) == LOW) {
      performSpray();
      enterBlockedState();
    }
  }

  // Обработка света
  if (wokeByLight) {
    wokeByLight = false;
    if (digitalRead(PIN_AUTO_MODE_SWITCH) == LOW) {
      handleLightChange();
    }
  }

  if (wdtTriggered) {
    wdtTriggered = false;
    setupWatchdogForBlocking();  // перезапуск WDT
  }

  // Обновление состояния (только если автоматический режим включён)
  if (digitalRead(PIN_AUTO_MODE_SWITCH) == LOW) {
    updateLeds();
    updateBuzzer();
  }

  // Решение: спать или нет?
  goToSleepIfNeeded();

  // Небольшая задержка, чтобы не грузить CPU (работает только когда свет включён)
  delay(50);
}