/********************************************************************
 * Smart Air Freshener – Li-ion Battery Version (3.3V)
 * Platform: Arduino Pro Mini (ATmega328P 8MHz)
 * Author: ChatGPT
 ********************************************************************/

// -----------------------------------------------------------
// ПИНЫ
// -----------------------------------------------------------
#define PIN_LIGHT 3     // Датчик света (HIGH = свет включён)
#define PIN_BUTTON 2    // Кнопка ручного распыления (GND при нажатии)
#define PIN_MOTOR 4     // Мотор распылителя (HIGH = включён)
#define PIN_LED_R 5     // Красный светодиод (общий анод)
#define PIN_LED_G 6     // Зелёный светодиод
#define PIN_LED_B 7     // Синий светодиод
#define PIN_MODE 8      // Переключатель режима (LOW = авто)
#define PIN_BUZZER 9    // Пьезоизлучатель (HIGH = сигнал)

// -----------------------------------------------------------
// ТАЙМИНГИ
// -----------------------------------------------------------
#define TIME_LIGHT_READY_MS (5UL * 1000UL)  // Время до READY (5 сек)
#define TIME_BLOCK_MS (5UL * 1000UL)        // Время блокировки после распыления (5 сек)
#define TIME_LED_BLINK_MS 600UL             // Период мигания LED
#define TIME_READY_BEEP_MS 300UL            // Продолжительность короткого сигнала при READY

// -----------------------------------------------------------
// ПАРАМЕТРЫ РАСПЫЛЕНИЯ
// -----------------------------------------------------------
#define SPRAY_PHASE1_MS 1000UL  // Длительность первого импульса
#define SPRAY_PAUSE_MS 1000UL   // Пауза между импульсами
#define SPRAY_PHASE2_MS 1000UL  // Длительность второго импульса

// -----------------------------------------------------------
// АВТОМАТИЧЕСКИЕ НАСТРОЙКИ
// -----------------------------------------------------------
#define AUTO_SPRAY_ON_LIGHT_OFF true
// true = распыление при выключении света, false = сразу после готовности

// -----------------------------------------------------------
// СОСТОЯНИЯ УСТРОЙСТВА
// -----------------------------------------------------------
enum State { STATE_IDLE, STATE_LIGHT_WAIT, STATE_READY, STATE_SPRAY, STATE_BLOCKED };
State currentState = STATE_IDLE;

// -----------------------------------------------------------
// LED (общий анод)
// -----------------------------------------------------------
enum LedColor { LED_RED_OFF, LED_RED_ON, LED_GREEN_OFF, LED_GREEN_ON, LED_BLUE_OFF, LED_BLUE_ON };

// -----------------------------------------------------------
// ПЕРЕМЕННЫЕ ВРЕМЕНИ
// -----------------------------------------------------------
uint32_t tLightOn = 0, tBlockStart = 0, tBlink = 0;
uint32_t tSpray = 0;
uint8_t sprayPhase = 0;

// -----------------------------------------------------------
// ФУНКЦИИ
// -----------------------------------------------------------
void updateLed(LedColor red, LedColor green, LedColor blue) {
  digitalWrite(PIN_LED_R, red == LED_RED_ON ? LOW : HIGH);
  digitalWrite(PIN_LED_G, green == LED_GREEN_ON ? LOW : HIGH);
  digitalWrite(PIN_LED_B, blue == LED_BLUE_ON ? LOW : HIGH);
}

inline bool isLightOn() { return digitalRead(PIN_LIGHT) == HIGH; }
inline bool isButtonPressed() { return digitalRead(PIN_BUTTON) == LOW; }
inline bool isAutoModeEnabled() { return digitalRead(PIN_MODE) == LOW; }

// -----------------------------------------------------------
// РАСПЫЛЕНИЕ
// -----------------------------------------------------------
void startSpray() {
  currentState = STATE_SPRAY;
  sprayPhase = 0;
  tSpray = millis();
}

bool runSpray() {
  uint32_t now = millis();
  switch (sprayPhase) {
    case 0:
      digitalWrite(PIN_MOTOR, HIGH);
      if (now - tSpray >= SPRAY_PHASE1_MS) {
        digitalWrite(PIN_MOTOR, LOW);
        sprayPhase = 1;
        tSpray = now;
      }
      break;
    case 1:
      if (now - tSpray >= SPRAY_PAUSE_MS) {
        sprayPhase = 2;
        tSpray = now;
      }
      break;
    case 2:
      digitalWrite(PIN_MOTOR, HIGH);
      if (now - tSpray >= SPRAY_PHASE2_MS) {
        digitalWrite(PIN_MOTOR, LOW);
        return true;
      }
      break;
  }
  return false;
}

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
}

// -----------------------------------------------------------
// LOOP
// -----------------------------------------------------------
void loop() {
  uint32_t now = millis();
  bool isLight = isLightOn();
  bool isAuto = isAutoModeEnabled();

  // --------------------------
  // Сброс таймера готовности при смене режима
  // --------------------------
  static bool wasAutoMode = false;

  if (isAuto && !wasAutoMode && isLight) {
    tLightOn = now;  // авто включен → сброс таймера
  }

  if (!isAuto && wasAutoMode && currentState == STATE_READY) {
    currentState = STATE_LIGHT_WAIT; // READY → ручной → сброс
    tLightOn = now;
  }

  wasAutoMode = isAuto;

  // --------------------------
  // Блокировка
  // --------------------------
  if (currentState == STATE_BLOCKED) {
    if (isLight) {
      if (now - tBlink >= TIME_LED_BLINK_MS) {
        tBlink = now;
        static bool isRedBlink = false;
        isRedBlink = !isRedBlink;
        updateLed(isRedBlink ? LED_RED_ON : LED_RED_OFF, LED_GREEN_OFF, LED_BLUE_OFF);
      }
    } else {
      updateLed(LED_RED_OFF, LED_GREEN_OFF, LED_BLUE_OFF);
    }
    if (now - tBlockStart >= TIME_BLOCK_MS) {
      currentState = STATE_IDLE;
      updateLed(LED_RED_OFF, LED_GREEN_OFF, LED_BLUE_OFF);
    }
    return;
  }

  // --------------------------
  // Кнопка
  // --------------------------
  if (isButtonPressed() && currentState != STATE_SPRAY) {
    startSpray();
  }

  // --------------------------
  // Распыление
  // --------------------------
  if (currentState == STATE_SPRAY) {
    updateLed(LED_RED_OFF, LED_GREEN_ON, LED_BLUE_OFF);
    if (runSpray()) {
      tBlockStart = now;
      currentState = STATE_BLOCKED;
      updateLed(LED_RED_OFF, LED_GREEN_OFF, LED_BLUE_OFF);
    }
    return;
  }

  // --------------------------
  // Свет выключен
  // --------------------------
  if (!isLight) {
    tBlink = now;
    if (AUTO_SPRAY_ON_LIGHT_OFF && currentState == STATE_READY) startSpray();
    else currentState = STATE_IDLE;
    updateLed(LED_RED_OFF, LED_GREEN_OFF, LED_BLUE_OFF);
    return;
  }

  // --------------------------
  // Свет включен
  // --------------------------
  switch (currentState) {
    case STATE_IDLE:
      tLightOn = now;
      tBlink = now;
      currentState = STATE_LIGHT_WAIT;
      break;
    case STATE_LIGHT_WAIT:
      if (now - tLightOn < TIME_LIGHT_READY_MS) {
        if (now - tBlink >= TIME_LED_BLINK_MS) {
          tBlink = now;
          static bool isBlueBlink = false;
          isBlueBlink = !isBlueBlink;
          updateLed(LED_RED_OFF, LED_GREEN_OFF, isBlueBlink ? LED_BLUE_ON : LED_BLUE_OFF);
        }
      } else {
        if (isAuto) {
          currentState = STATE_READY;
          updateLed(LED_RED_OFF, LED_GREEN_OFF, LED_BLUE_ON);

          digitalWrite(PIN_BUZZER, HIGH);
          delay(TIME_READY_BEEP_MS);
          digitalWrite(PIN_BUZZER, LOW);

          if (!AUTO_SPRAY_ON_LIGHT_OFF) startSpray();
        } else {
          tLightOn = now; // ручной режим → сброс таймера
        }
      }
      break;
    case STATE_READY:
      updateLed(LED_RED_OFF, LED_GREEN_OFF, LED_BLUE_ON);
      break;
  }
}
