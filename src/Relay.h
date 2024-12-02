#pragma once

#include "Duration.h"

namespace mys_toolkit {

class Relay
{
  bool requestedState_ = false;
  bool state_ = false;
  bool prevSwState_ = false;
  Duration minStateDuration_;
  Duration nextStateChange_{0};
  bool isRising_(bool swState);
  void handleState_();
  virtual void updateState_(bool state) = 0;
  virtual void update_() {}
public:
  Relay(Duration minStateDuration = Duration(0));
  virtual void begin() {};
  void update(bool currSwState);
  bool getState();
  void set(bool state);
};

class GPIORelay : public Relay
{
  int relayPin_;
  void updateState_(bool state) override;

public:
  GPIORelay(int relayPin, Duration minStateDuration = Duration(0));
};

} //mys_toolkit

