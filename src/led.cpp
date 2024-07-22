#define LED_PIN A1
#include "led.h"
#include <Arduino.h>

static unsigned long led_turn_off = 0;

void init_led() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void trigger_led() {
  led_turn_off = millis() + 200;
  digitalWrite(LED_PIN, LOW);
}

void update_led() {
  if (millis() >= led_turn_off) {
    digitalWrite(LED_PIN, HIGH);
  }
}

void error(eMBErrorCode code) {
  while (true) {
    digitalWrite(LED_PIN, HIGH);
    delay(code * 200);
    digitalWrite(LED_PIN, LOW);
    delay(code * 200);
  }
}
