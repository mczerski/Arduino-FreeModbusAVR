#include "SceneController.h"

namespace mys_toolkit {

bool SceneController::isRising_(bool swState)
{
  return swState == true and prevSwState_ == false;
}

bool SceneController::isHeldLongEnough_(bool swState)
{
  return swState == true and prevSwState_ == true and lastSwRiseTime_ + Duration(1000) < Duration();
}

bool SceneController::isFalling(bool swState)
{
  return swState == false and prevSwState_ == true;
}

void SceneController::update(bool state)
{
  bool currSwState = state;
  if (isRising_(currSwState)) {
    lastSwRiseTime_ = Duration();
    state_ = WAITING_FOR_SCENE;
  }
  else if (state_ == WAITING_FOR_SCENE and isHeldLongEnough_(currSwState)) {
    state_ = WAITING_FOR_FALLING;
    scene1Counter_++;
  }
  else if (isFalling(currSwState)) {
    if (state_ == WAITING_FOR_SCENE) {
      if (enableShortPress_) {
        scene0Counter_++;
      }
    }
    state_ = WAITING_FOR_RISING;
  }
  prevSwState_ = currSwState;
}

uint16_t SceneController::getScene0Counter() const {
    return scene0Counter_;
}

uint16_t SceneController::getScene1Counter() const {
    return scene1Counter_;
}

SceneController::SceneController(bool enableShortPress)
  : state_(WAITING_FOR_RISING),
    prevSwState_(false),
    enableShortPress_(enableShortPress)
{
}

} //mys_toolkit
