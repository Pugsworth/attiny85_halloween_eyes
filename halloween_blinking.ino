#include <avr/power.h>
#include <avr/sleep.h>

const int PIN_LED = PB0;
const unsigned long ONE_SECOND = 1000;
const unsigned long HALF_SECOND = ONE_SECOND * 0.5;
const unsigned long QUART_SECOND = ONE_SECOND * 0.25;
const unsigned long TENTH_SECOND = ONE_SECOND * 0.10;
const unsigned long TEN_SECONDS = ONE_SECOND * 10;
const unsigned long ONE_MINUTE = ONE_SECOND * 60;
const unsigned long TEN_MINUTES = ONE_MINUTE * 10;
const unsigned long THIRTY_MINUTES = ONE_MINUTE * 30;

int level = 0; // led brightness
int tick = 0; // this ticks each watchdog interrupt.
volatile byte f_wdt = 0;

// Routines to set and claer bits (used in the sleep code)
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

void setup() {
//  Serial.begin(9600);
  pinMode(PIN_LED, OUTPUT);
  randomSeed(analogRead(0));
  delay(ONE_SECOND);

  setup_watchdog(6);

  offLED();
  delay(ONE_SECOND);
  onLED();
  blinkFast();
  delay(ONE_SECOND);
  blinkSlow();
  delay(TEN_SECONDS);

  demo1();
}

void loop() {
  if (f_wdt == 1) {
    f_wdt = 0;

    if (tick++ >= 120) {
      tick = 0;

      demo1();
    }

    enterSleep();
  }
}

void enterSleep() {
  cbi(ADCSRA, ADEN); // switch Analog to Digitalconverter OFF
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();

  // execution continues here after sleep
  sleep_disable();
  sbi(ADCSRA, ADEN); // switch Analog to Digitalconverter ON
}

// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii) {

  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;

  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
}

// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  f_wdt = 1;  // set global flag
}

void demo1() {
  int blinks = random(1, 4); // 1 to 3 blinks per "stare" period
//  Serial.print("Blinking "); Serial.print(blinks); Serial.println(" times");
  int delayRandom = TEN_SECONDS / blinks;
  fadeUp(ONE_SECOND * 3);
  while (blinks-- > 0) {
    delay(random(ONE_SECOND, delayRandom));
    choice(blinkSlow, blinkFast);
  }
  delay(TEN_SECONDS);
  fadeDown(ONE_SECOND);
}

void demo2() {
  offLED();
  delay(1000);
  fadeUp(1.0f);
  delay(1000);
  blinkFast();
  delay(1000);
  offLED();
  delay(1000);
  fadeUp(1.0f, 200);
  delay(1000);
  fadeDown(1.0f);
}

float Rand() {
  return (float)(random(1000) / 1000.0f);
}

void blinkFast() {
//  Serial.println("Fast Blink");
  offLED();
  delay(50);
  onLED();
  delay(QUART_SECOND);
  offLED();
  delay(50);
  onLED();
}

void blinkSlow() {
//  Serial.println("Slow Blink");
  fadeDown(QUART_SECOND);
  delay(TENTH_SECOND);
  fadeUp(QUART_SECOND);
  delay(HALF_SECOND);
  fadeDown(QUART_SECOND);
  delay(TENTH_SECOND);
  fadeUp(QUART_SECOND);
}

void onLED() {
  digitalWrite(PIN_LED, HIGH);
}

void offLED() {
  digitalWrite(PIN_LED, LOW);
}

void fadeUp(float duration, int maxBright) {
  float waitTime = duration / (float)(maxBright-level);
  while(level++ < maxBright) {
    analogWrite(PIN_LED, level);
    delayMicroseconds(waitTime*1000);
  }
}

void fadeUp(float duration) {
  fadeUp(duration, 255);
}

void fadeDown(float duration, int minBright) {
  float waitTime = max(duration / (float)(level - minBright), 0);
//  Serial.print("fadeDown waitTime: "); Serial.println(waitTime);
  while (level-- > minBright) {
    analogWrite(PIN_LED, level);
    delayMicroseconds(waitTime*1000);
  }
}

void fadeDown(float duration) {
  fadeDown(duration, 0);
}

void choice(void* func1, void* func2) {
  if (random(2) == 0) {
    ((void(*)())func1)();
  }
  else {
    ((void(*)())func2)();
  }
}
