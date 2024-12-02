#include "Relay.h"

#include <Arduino.h>

namespace mys_toolkit {

Relay::Relay(Duration minStateDuration)
  : minStateDuration_(minStateDuration),
    nextStateChange_(Duration() + minStateDuration)
{}

bool Relay::getState()
{
  return state_;
}

void Relay::update(bool currSwState)
{
  if (isRising_(currSwState)) {
    requestedState_ = not requestedState_;
  }
  handleState_();
  update_();
  prevSwState_ = currSwState;
}

bool Relay::isRising_(bool swState)
{
  return swState == true and prevSwState_ == false;
}

void Relay::handleState_() {
  if (requestedState_ != state_ and Duration() >= nextStateChange_) {
      state_ = requestedState_;
      updateState_(state_);
      nextStateChange_ = Duration() + minStateDuration_;
  }
}

void Relay::set(bool state)
{
  requestedState_ = state;
  handleState_();
}


void GPIORelay::updateState_(bool state)
{
  digitalWrite(relayPin_, !state);
}

GPIORelay::GPIORelay(int relayPin, Duration minStateDuration)
  : Relay(minStateDuration),
    relayPin_(relayPin)
{
  pinMode(relayPin_, OUTPUT);
}

} //mys_toolkit
