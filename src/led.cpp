#include "led.h"
#include <Arduino.h>

static unsigned long led_on_since = 0;
static bool led_on = false;
static int led_pin = -1;

void init_led(int pin) {
  led_pin = pin;
  pinMode(led_pin, OUTPUT);
  led_on_since = millis();
  led_on = true;
  digitalWrite(led_pin, LOW);
}

void trigger_led() {
  if (led_pin < 0) return;
  led_on_since = millis();
  led_on = true;
  digitalWrite(led_pin, LOW);
}

void update_led() {
  if (led_pin < 0) return;
  if (led_on and millis() - led_on_since >= 200) {
    led_on = false;
    digitalWrite(led_pin, HIGH);
  }
}

void error(eMBErrorCode code) {
  while (true) {
    if (led_pin < 0) continue;
    for (int i = 0; i < code; i++) {
        digitalWrite(led_pin, LOW);
        delay(100);
        digitalWrite(led_pin, HIGH);
        delay(100);
    }
    delay(1000);
  }
}
