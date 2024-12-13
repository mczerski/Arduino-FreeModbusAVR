#include "BL0942.h"

using namespace mys_toolkit;

bool BL0942::readPacket_() {
  serial_.readBytes(reinterpret_cast<byte*>(&packet_buffer_), sizeof(packet_buffer_));
  // flush buffer
  while (serial_.available()) {serial_.read();}
  auto checksum = calcChecksum_(PACKET_HEADER, reinterpret_cast<byte*>(&packet_buffer_), sizeof(packet_buffer_));
  if (checksum != packet_buffer_.checksum) {
    return false;
  }
  return true;
}

bool BL0942::readReg_(byte reg) {
  serial_.readBytes(reinterpret_cast<byte*>(&reg_buffer_), sizeof(reg_buffer_));
  // flush buffer
  while (serial_.available()) {serial_.read();}
  auto checksum = calcChecksum_(reg, reinterpret_cast<byte*>(&reg_buffer_), sizeof(reg_buffer_));
  if (checksum != reg_buffer_.checksum) {
    return false;
  }
  return true;
}

void BL0942::readCmd_(byte reg) {
  serial_.write(READ_COMMAND);
  serial_.write(reg);
  last_read_cmd_ = reg;
  last_read_timestamp_ = Duration();
}

byte BL0942::calcChecksum_(byte reg, const byte* buffer, size_t size) const {
  byte checksum = READ_COMMAND + reg;
  // Whole package but checksum
  for (size_t i = 0; i < size - 1; i++) {
      checksum += buffer[i];
  }
  checksum ^= 0xFF;
  return checksum;
}

void BL0942::writeReg_(byte address, uint32_t data) {
  byte data_0 = (data >> 0) & 0xff;
  byte data_1 = (data >> 8) & 0xff;
  byte data_2 = (data >> 16) & 0xff;
  byte checksum = WRITE_COMMAND + address + data_0 + data_1 + data_2;
  checksum ^= 0xff;
  serial_.write(WRITE_COMMAND);
  serial_.write(address);
  serial_.write(data_0);
  serial_.write(data_1);
  serial_.write(data_2);
  serial_.write(checksum);
}

BL0942::BL0942(Stream &serial, float i_gain, float v_gain)
  : serial_(serial),
    i_gain_(i_gain),
    v_gain_(v_gain)
{
}

void BL0942::begin() {
    readCmd_(FULL_PACKET);
}

bool BL0942::update() {
  if (not last_read_timestamp_.get()) {
    // next read request
    switch (last_read_cmd_) {
        case FULL_PACKET:
            readCmd_(I_FAST_RMS_TH);
            break;
        case I_FAST_RMS_TH:
            readCmd_(OT_FUNX);
            break;
        case OT_FUNX:
            readCmd_(FULL_PACKET);
            break;
    }
    return false;
  }
  byte bytes_to_read = last_read_cmd_ == FULL_PACKET ? sizeof(packet_buffer_) + 1 : sizeof(reg_buffer_);
  if (serial_.available() <  bytes_to_read) {
    // chek for timout
    if (Duration() >= last_read_timestamp_ + Duration(100)) {
      last_read_timestamp_ = Duration(0);
      while (serial_.available()) {serial_.read();}
    }
    return false;
  }
  // data ready in serial buffer
  last_read_timestamp_ = Duration(0);
  if (last_read_cmd_ == FULL_PACKET) {
    if (serial_.read() != PACKET_HEADER) {
      while (serial_.available()) {serial_.read();}
      return false;
    }
    if (not readPacket_()) {
      return false;
    }
    double p_gain = i_gain_ * v_gain_;
    i_rms_mA_ = round(packet_buffer_.i_rms * I_COEFF / i_gain_);
    v_rms_mV_ = round(packet_buffer_.v_rms * V_COEFF / v_gain_);
    p_rms_mW_ = round(packet_buffer_.watt * P_COEFF / p_gain);
    e_mWh_ = round(packet_buffer_.cf_cnt * E_COEFF / p_gain);
  }
  else {
    if (not readReg_(last_read_cmd_)) {
      return false;
    }
    switch (last_read_cmd_) {
        case I_FAST_RMS_TH:
            i_fast_rms_th_mA_ = round((reg_buffer_.value << 8) * I_FAST_COEFF);
            if (abs(i_fast_rms_th_mA_ - 15000) > 2) {
              writeReg_(USR_WRPROT, 0x55);  // unlock
              writeReg_(I_FAST_RMS_TH, round(15000 / I_FAST_COEFF / 256));
              writeReg_(USR_WRPROT, 0); // lock
            }
            break;
        case OT_FUNX:
            cf_output_ = reg_buffer_.value & 0x3;
            if (cf_output_ != 1) {
              writeReg_(USR_WRPROT, 0x55);  // unlock
              writeReg_(OT_FUNX, 0x21);
              writeReg_(USR_WRPROT, 0); // lock
            }
            break;
    }
  }
  return true;
}

int32_t BL0942::getIRms_mA() const {
  return i_rms_mA_;
}

int32_t BL0942::getVRms_mV() const {
  return v_rms_mV_;
}

int32_t BL0942::getPRms_mW() const {
  return p_rms_mW_;
}

uint32_t BL0942::getE_mWh() const {
  return e_mWh_;
}

int32_t BL0942::getIFastRmsTh_mA() const {
  return i_fast_rms_th_mA_;
}

byte BL0942::getCfOutput() const {
  return cf_output_;
}

void BL0942::calibrate(float i_gain, float v_gain) {
    i_gain_ = isnan(i_gain) ? 1.0 : i_gain;
    v_gain_ = isnan(v_gain) ? 1.0 : v_gain;
}
