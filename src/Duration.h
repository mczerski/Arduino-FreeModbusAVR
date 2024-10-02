#pragma once

#include <stdint.h>

namespace mys_toolkit {

enum class DurationPrescaler {
    CLK = 1,
    CLK_8,
    CLK_64,
    CLK_256,
    CLK_1024
};

class Duration
{
public:
  static void setPrescaler(DurationPrescaler duration_prescaler);
  typedef unsigned long duration_ms_t;
  explicit Duration(duration_ms_t duration);
  explicit Duration();
  duration_ms_t getMilis() const;
  duration_ms_t get() const;
  bool operator<(const Duration &other);
  bool operator<=(const Duration &other);
  void operator+=(const Duration &other);
  void operator*=(int factor);
  Duration operator+(const Duration &other);
private:
  duration_ms_t duration_;
};

} //mys_toolkit

