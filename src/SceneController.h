#pragma once

#include <Duration.h>

namespace mys_toolkit {

class SceneController
{
  enum State {
    WAITING_FOR_RISING,
    WAITING_FOR_SCENE,
    WAITING_FOR_FALLING
  };
  State state_;
  bool prevSwState_;
  Duration lastSwRiseTime_;
  uint16_t scene0Counter_ = 0;
  uint16_t scene1Counter_ = 0;
  bool enableShortPress_;
  bool isRising_(bool swState);
  bool isHeldLongEnough_(bool swState);
  bool isFalling(bool swState);
  void sendScene_(uint8_t scene);

public:
  SceneController(bool enableShortPress=true);
  void update(bool state);
  uint16_t getScene0Counter() const;
  uint16_t getScene1Counter() const;
};

} //mys_toolkit


