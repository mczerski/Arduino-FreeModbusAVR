#include "APDS9930Switch.h"

namespace mys_toolkit {

void MyAPDS9930::pcaSelect_(uint8_t i)
{
  if (apdsNum_ == 1)
    return;
  else {
    if (i > apdsNum_)
      return;
    Wire.beginTransmission(PCAADDR);
    Wire.write(4 + i);
    Wire.endTransmission();
  }
}

uint8_t MyAPDS9930::pcaGet_()
{
  if (apdsNum_ == 1)
    return digitalRead(intPin_) == LOW ? 1 : 0;
  else {
    Wire.requestFrom(PCAADDR, 1);
    return Wire.read() >> 4;
  }
}

void MyAPDS9930::init_(uint8_t i)
{
  pcaSelect_(i);
  if (!apds_[i].init()) {
      errorCode_[i] = 1;
      return;
  }
  if (!apds_[i].enableProximitySensor(true)) {
      errorCode_[i] = 20;
      return;
  }
  if (!apds_[i].setProximityGain(PGAIN_1X)) {
      errorCode_[i] = 21;
      return;
  }
  if (!apds_[i].setProximityIntLowThreshold(PROX_INT_LOW)) {
      errorCode_[i] = 22;
      return;
  }
  if (!apds_[i].setProximityIntHighThreshold(PROX_INT_HIGH)) {
      errorCode_[i] = 23;
      return;
  }
  return;
}

bool MyAPDS9930::update_(uint8_t i)
{
  pcaSelect_(i);
  if (apds_[i].getProximityInt() == 0)
    return false;
  uint16_t proximity_data = 0;
  if (!apds_[i].readProximity(proximity_data)) {
      errorCode_[i] = 24;
  }
  if (proximity_data < PROX_INT_HIGH) {
    if (!apds_[i].clearProximityInt()) {
        errorCode_[i] = 25;
    }
  }
  return true;
}

MyAPDS9930::MyAPDS9930(uint8_t intPin, int apdsNum)
  : intPin_(intPin), apdsNum_(apdsNum), apdsInts_(0)
{
}

void MyAPDS9930::init()
{
  pinMode(intPin_, INPUT_PULLUP);
  for (uint8_t i=0; i<apdsNum_; i++)
    init_(i);
}

void MyAPDS9930::update()
{
  for (uint8_t i=0; i<apdsNum_; i++) {
    pcaSelect_(i);
    if (apds_[i].getProximityIntEnable() == 0) {
      init_(i);
    }
  }
  uint8_t pca = pcaGet_();
  apdsInts_ &= pca;
  if (pca) {
    for (uint8_t i=0; i<apdsNum_; i++) {
      if (pca & (1 << i)) {
        if (update_(i)) {
          apdsInts_ |= (1 << i);
        }
      }
    }
  }
}

bool MyAPDS9930::getInt(uint8_t i) const
{
  return apdsInts_ & (1 << i);
}

uint8_t MyAPDS9930::getErrorCode(uint8_t i) const
{
  return errorCode_[i];
}

bool APDS9930Switch::doUpdate_()
{
  return myApds_.getInt(apdsNo_);
}

APDS9930Switch::APDS9930Switch(const MyAPDS9930 &myApds, uint8_t apdsNo)
  : Switch(false), myApds_(myApds), apdsNo_(apdsNo)
{
}

} //mys_toolkit
