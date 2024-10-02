#include "led.h"
#include <Arduino.h>

static unsigned long led_turn_off = 0;
static int led_pin = -1;

void init_led(int pin) {
  led_pin = pin;
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
}

void trigger_led() {
  if (led_pin < 0) return;
  led_turn_off = millis() + 200;
  digitalWrite(led_pin, LOW);
}

void update_led() {
  if (led_pin < 0) return;
  if (millis() >= led_turn_off) {
    digitalWrite(led_pin, HIGH);
  }
}

void error(eMBErrorCode code) {
  while (true) {
    if (led_pin < 0) continue;
    digitalWrite(led_pin, HIGH);
    delay(code * 200);
    digitalWrite(led_pin, LOW);
    delay(code * 200);
  }
}
