#include "Duration.h"
#include <Arduino.h>
#include <limits.h>

namespace mys_toolkit {

struct Prescaler {
  uint8_t divide;
  uint8_t multiply;
};
static Prescaler prescaler_ = {1, 1};

void Duration::setPrescaler(DurationPrescaler duration_prescaler)
{
  static Prescaler prescaler_mapping[] = {{0, 0}, {64, 1}, {8, 1}, {1, 1}, {1, 4}, {1, 16}};
  uint8_t prescaler_index = static_cast<uint8_t>(duration_prescaler);
  prescaler_ = prescaler_mapping[prescaler_index];
  #if defined(TCCR0B) && defined(CS02) &&  defined(CS01) && defined(CS00)
  TCCR0B &= 0xF8;
  TCCR0B |= prescaler_index;
  #else
  #error Timer 0 prescale factor not set correctly
  #endif
}

Duration::Duration(Duration::duration_ms_t duration)
  : duration_(duration * prescaler_.divide / prescaler_.multiply) 
{
}

Duration::Duration()
  : duration_(millis()) 
{
}

Duration::duration_ms_t Duration::getMilis() const
{
  return duration_ * prescaler_.multiply / prescaler_.divide;
}

Duration::duration_ms_t Duration::get() const
{
  return duration_;
}

bool Duration::operator<(const Duration &other)
{
  Duration::duration_ms_t difference = duration_ - other.duration_;
  return difference >= ULONG_MAX/2;
}

bool Duration::operator<=(const Duration &other)
{
  return this->duration_ == other.duration_ or *this < other;
}

void Duration::operator+=(const Duration &other)
{
  duration_ += other.duration_;
}

void Duration::operator*=(int factor)
{
  duration_ *= factor;
}

Duration Duration::operator+(const Duration &other)
{
  Duration result = *this;
  result += other;
  return result;
}

} //mys_toolkit
