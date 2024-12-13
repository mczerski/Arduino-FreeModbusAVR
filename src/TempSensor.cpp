#include <avr/io.h>
#include <Arduino.h>
#include "TempSensor.h"

namespace mys_toolkit {

TempSensor::TempSensor(int temp_offset, float temp_gain)
  : temp_offset_(temp_offset),
    temp_gain_(temp_gain)
{}

void TempSensor::begin() {
  analogReference(INTERNAL);
}

void TempSensor::update() {
  temperature_ = readTemp_();
}

uint16_t TempSensor::readTemp_()
{
  // Configure ADC for reading temperature sensor
  ADMUX |= (1 << REFS0) | (1 << REFS1) | (1 << MUX3);
  ADMUX &= ~((1 << MUX2) | (1 << MUX1) | (1 << MUX0) | (1 << ADLAR));

  // Start conversion
  ADCSRA |= (1 << ADSC);

  // Wait for conversion to complete
  while (ADCSRA & (1 << ADSC));

  // Read the temperature
  return round((ADCW - temp_offset_ ) / temp_gain_);
}

int16_t TempSensor::getTemp()
{
  return temperature_;
}

void TempSensor::calibrate(int temp_offset, float temp_gain)
{
  temp_offset_ = temp_offset == -1 ? 245 : temp_offset;
  temp_gain_  = isnan(temp_gain) ? 1.0 : temp_gain;
}

} // mys_toolkit
