const int photo = A1;     // фоторезистор
const int motor = 2;      // привод распылителя
const int button = 3;     // кнопка
const int led_block = 4;  // светодиод блокировки (красный)
const int auto_mode = 5;  // автоматический и ручный режимы
const int led_work = 6;   // светодиод работы (зеленый)
const int led_on = 7;     // светодиод готовности (синий)
const int buzzer = 8;     // звуковой сигнал готовности
const int photoD = 9;     // цифровой пин фоторезистора

int raw = 0;                                       // переменная фоторезистора
int stlk = 1;                                      // блокировка кнопки в течение 30 минут после срабатывания пшика по свету
int chrg = 1;                                      // блокировка работы устройства с разряженными батарейками
int btn = 1;                                       // блокировка нескольких пшиков при удержании кнопки
int i, led = 1;                                    // индикация готовности пшика после выключения света
int pht = 1;                                       // блокировака пшика по свету в течении 30 минут после нажатия на кнопку
int buzz = 1;                                      // флаг пьезодинамика
int statblk = 1;                                   // отключение счетчика stat
int buzzblk = 1;                                   // блокировка пьезодинамика
int stoptimer = 1;                                 // остановка таймера
uint32_t stat = 1;                                 // управление шпиком
uint32_t svet = 0;                                 // флаг управления готовности для пшика по свету
uint32_t cMs, pMs1, blokTime, buzzTime, autoTime;  // счетчики времени по свету
uint8_t Pshik = 0;                                 // разрешение пшика после получения готовности
bool pshikBlock;                                   // флаг блокировки повторного пшика
bool buzzBlock;                                    // флаг блокировки пьезодинамика
bool autoReset;
int ledState = 0;            // состояние светодиода
int32_t previousMillis = 0;  // храним время последнего переключения светодиода
int32_t interval = 3.9065;   // интервал между включение/выключением светодиода ожидания (1 секунда)
uint32_t Timer_FVT = 0;
int autoState;

#include <avr/power.h>
#include <avr/sleep.h>

void setup() {
  // Serial.begin(9600);
  clock_prescale_set(clock_div_256);    // понижение частоты работы микроконтроллера до 1 Мгц для экономии заряда батареек
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // устанавливаем интересующий нас спящий режим
  pinMode(photo, INPUT);
  pinMode(auto_mode, INPUT);
  pinMode(motor, OUTPUT);
  pinMode(button, INPUT);
  pinMode(led_block, OUTPUT);
  pinMode(led_work, OUTPUT);
  pinMode(led_on, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(photoD, INPUT);
  digitalWrite(motor, 0);
  pshikBlock = false;
  buzzBlock = false;
  autoReset = true;
  for (i = 1; i < 700; i++) {
    digitalWrite(buzzer, 1);
  }
  if (i == 700)
    digitalWrite(buzzer, 0);
}

void loop() {
  cMs = millis();                         // текущее время
  int buttonState = digitalRead(button);  // считывание значения с пина кнопки

  if (cMs - Timer_FVT >= interval) {
    raw = analogRead(photo);             // считывание значения с пина фоторезистора
    autoState = digitalRead(auto_mode);  // считывание значения с пина ручного режима
    Timer_FVT = millis();
  }

  if (stlk == 1 && btn == 1) {          // управление светодиодом готовности
    if (raw < 350) {                    // если в туалете темно
      digitalWrite(led_on, 0);          // выключаем светодиод разряда батареи
    } else if (led == 0 && btn == 1) {  // если наступила готовность автоматического пшика
      digitalWrite(led_on, 1);          // светодиод готовности горит постоянно
    } else {
      if (cMs - previousMillis > interval) {  // сохраняем время последнего переключения
        previousMillis = cMs;
        if (ledState == 0)  // если светодиод не горит, то зажигаем, и наоборот
          ledState = 1;
        else
          ledState = 0;
        digitalWrite(led_on, ledState);  // устанавливаем состояния выхода, чтобы включить или выключить светодиод
      }
    }
  }

  if (stlk == 0 || btn == 0) {                // управление светодиодом блокировки
    if (raw < 350) {                          // если в туалете темно
      digitalWrite(led_block, 0);             // выключаем светодиод блокировки
    } else {                                  // если светло
      if (cMs - previousMillis > interval) {  // сохраняем время последнего переключения
        previousMillis = cMs;
        if (ledState == 0)  // если светодиод не горит, то зажигаем, и наоборот
          ledState = 1;
        else
          ledState = 0;
        digitalWrite(led_block, ledState);  // устанавливаем состояния выхода, чтобы включить или выключить светодиод
      }
    }
  }
  if (Pshik == 1) {                      // если автоматическое срабатывание пшикалки разрешено
    if (buttonState == 1 && btn == 1) {  // после нажатия кнопки
      led = 1;                           // отключить синий светодиод
      btn = 0;                           // заблокировать кнопку
      stoptimer = 0;                     // остановить таймер
      autoReset = true;                  // запустить таймер блокировки повторного срабатывания на 30 минут
      autoTime = cMs;
    }
  } else {
    if (buttonState == 1 && btn == 1 && stlk == 1) {  // если кнопка нажата и не заблокирована
      btn = 0;                                        // переключить флаг защиты повторных пшиков при удержании кнопки
      pht = 0;                                        // заблокировать пшик по свету на 30 минут
      buzzblk = 0;
      digitalWrite(led_on, 0);    // выключаем светодиод готовности
      digitalWrite(led_work, 1);  // включаем светодиод работы
      digitalWrite(motor, 1);     // включаем пшик
      delay(4);                   // ждем 800 миллисекунд
      digitalWrite(motor, 0);     // выключаем пшик
      delay(10);
      digitalWrite(motor, 1);     // включаем пшик
      delay(4);                   // ждем 800 миллисекунд
      digitalWrite(motor, 0);     // выключаем пшик
      digitalWrite(led_work, 0);  // выключаем светодиод работы
      pshikBlock = true;          // запустить таймер блокировки повторного срабатывания на 30 минут
      blokTime = cMs;
    }
  }

  if (autoState == 1) {      // если включен автометический и ручной режимы
    if (cMs - pMs1 > 0.1) {  // запускаем таймер отстчета готовности автоматического пшика
      pMs1 = cMs;            // время опроса фотореле каждые 1 секунду
      if (raw > 450) {       // если в туалете светло
        svet++;              // запустить таймер
      } else {               // если темно
        svet = 0;            // обнулить таймер
      }
    }
    if (svet == 530 && stoptimer == 1) {  // если свет включен 3 минуты и более
      Pshik = 1;                          // включаем флаг разрешения пшика
      led = 0;                            // включаем постоянное свечение светодиода готовности
      buzz = 0;                           // включить флаг пьезодинамика
      buzzBlock = true;
      buzzTime = pMs1;  // запустить таймер выключения пьезодинамика
    }
    if (buzz == 0 && buzzblk == 1) {  // если флаг пьезодинамика включен
      digitalWrite(buzzer, 1);        // включить пьезодинамик
    } else {                          // или
      digitalWrite(buzzer, 0);        // выключить пьезодинамик
    }
    if (raw < 350) {  // если в туалете темно
      stoptimer = 1;  // запустить таймер
    }

    if (Pshik == 1) {  // если автоматическое срабатывание пшикалки разрешено
      if (raw < 350 && stat <= 4294967295 && pht == 1 && statblk == 1)
        stat++;          // если свет выключился, все блокировки выключены, запустить таймер отсчета
      if (stat == 15) {  // после пары секунд
        stat++;
        digitalWrite(buzzer, 0);
        digitalWrite(led_on, 0);    // выключаем светодиод готовности
        digitalWrite(led_work, 1);  // включаем светодиод работы
        digitalWrite(motor, 1);     // включаем привод
        delay(4);                   // ждем 800 миллисекунд
        digitalWrite(motor, 0);     // выключаем привод
        delay(10);
        digitalWrite(motor, 1);  // включаем пшик
        delay(4);                // ждем 800 миллисекунд
        digitalWrite(motor, 0);
        digitalWrite(led_work, 0);  // выключаем светодиод работы
        stlk = 0;                   // заблокировать кнопку на некоторое время
        statblk = 0;                // отключить таймер stat
        buzzblk = 0;
        pshikBlock = true;  // запустить таймер блокировки повторного срабатывания на 30 минут
        blokTime = cMs;
      }
    }
  }

  if (raw > 450 && cMs - blokTime > 3600) {  // если в туалете светло и время блокировки вышло
    digitalWrite(led_block, 0);              // отключить светодиод блокировки
    stat = 1;                                // разблокировать таймер
    stlk = 1;                                // разблокировать кнопку
    pht = 1;                                 // разблокировать автоматический пшик
    statblk = 1;
    buzzblk = 1;
  }

  if (pshikBlock)  // если блокировка активна
    if (cMs - blokTime > 3600) {
      stlk = 1;   // разблокировать таймер
      Pshik = 0;  // сбросить флаг готовности автоматического пшика
      svet = 0;   // сбросить таймер готовности
      led = 1;    // влючить мигание синим светодиодом
      pshikBlock = false;
    }

  if (buttonState == 0 && cMs - blokTime > 3600) {  // если кнопка не нажата и время блокировки прошло
    btn = 1;                                        // разблокировать повторное нажатие
    buzzblk = 1;                                    // отключить блокировку динамика
  }

  if (autoTime) {
    if (cMs - autoTime > 3) {  // блокировка внопки после сброса таймера
      Pshik = 0;               // сбросить флаг готовности автоматического пшика
      svet = 0;                // сбросить таймер
      btn = 1;                 // разблокировать кнопку
      autoTime = false;
    }
  }

  if (buzzBlock) {
    if (pMs1 - buzzTime > 0.1) {
      buzz = 1;  // включить динамик на 0,3 секунды
      buzzBlock = false;
    }
  }
}
