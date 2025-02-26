#include "BL0942.h"
#include <EEPROM.h>

using namespace mys_toolkit;

namespace {
  template <typename T>
  int eeprom_get_max_(int address, int range, T &max_value) {
    T v;
    EEPROM.get(address, v);
    max_value = v;
    int max_idx = 0;
    for (int i = 1; i < range; i++) {
      EEPROM.get(address + i * sizeof(T), v);
      if (v > max_value) {
        max_value = v;
        max_idx = i;
      }
    }
    return max_idx;
  }

  template <typename T>
  void eeprom_get_range(int address, int range, T &value) {
      eeprom_get_max_(address, range, value);
  }

  template <typename T>
  void eeprom_put_range(int address, int range, T value) {
    T v;
    int max_idx = eeprom_get_max_(address, range, v);
    if (v != value) {
      EEPROM.put(address + ((max_idx + 1) % range) * sizeof(T), value);
    }
  }
  
  template <typename T>
  void eeprom_init_range(int address, int range, T value) {
    for (int i = 0; i < range; i++) {
      EEPROM.put(address + i * sizeof(T), value);
    }
  }
}

EnergyStore::EnergyStore(int eeprom_addr)
  : eeprom_addr_(eeprom_addr)
{
}

uint32_t EnergyStore::update(uint32_t cf_cnt) {
  // cf_cnt is actuallly 24 bit long
  if (not init_done_) {
    // initialize eeprom stored cf_cnt (happens after MCU restart)
    eeprom_get_range(eeprom_addr_, 24, base_cf_cnt_stored_);
    // if cf_cnt is not stored in eeprom initialize it with 0
    if (base_cf_cnt_stored_ == 0xffffffff) {
      base_cf_cnt_stored_ = 0;
      eeprom_init_range(eeprom_addr_, 24, base_cf_cnt_stored_);
    }
    // initialize received cf_cnt base with first received value (happens after MCU restart)
    base_cf_cnt_received_ = cf_cnt;
    init_done_ = true;
  }
  // handle cf_cnt rollover or BL0942 restart (not likely to happen)
  if (base_cf_cnt_received_ > cf_cnt) {
    base_cf_cnt_stored_ = base_cf_cnt_stored_ + base_cf_cnt_received_ + cf_cnt;
    base_cf_cnt_received_ = cf_cnt;
    eeprom_put_range(eeprom_addr_, 24, base_cf_cnt_stored_);
    last_store_timestamp_ = Duration();
  }
  uint32_t updated_cf_cnt = cf_cnt - base_cf_cnt_received_ + base_cf_cnt_stored_;
  // store cf_cnt once every hour
  if (Duration() >= last_store_timestamp_ + Duration(60 * 60 * 1000ul)) {
    eeprom_put_range(eeprom_addr_, 24, updated_cf_cnt);
    base_cf_cnt_stored_ = updated_cf_cnt;
    base_cf_cnt_received_ = cf_cnt;
    last_store_timestamp_ = Duration();
  }
  return updated_cf_cnt;
}

void EnergyStore::set(uint32_t cf_cnt) {
  eeprom_init_range(eeprom_addr_, 24, cf_cnt);
  init_done_ = false;
}

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

BL0942::BL0942(Stream &serial, int eeprom_addr, float i_gain, float v_gain)
  : serial_(serial),
    i_gain_(i_gain),
    v_gain_(v_gain),
    energy_store_(eeprom_addr)
{
}

void BL0942::begin() {
    writeReg_(USR_WRPROT, 0x55);  // unlock
    writeReg_(SOFT_RESET, 0x5a5a5a);
    writeReg_(USR_WRPROT, 0); // lock
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
    // chek for timoute
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
    int32_t i_rms_mA = round(packet_buffer_.i_rms * I_COEFF / i_gain_);
    int32_t v_rms_mV = round(packet_buffer_.v_rms * V_COEFF / v_gain_);
    int32_t p_rms_mW = round(packet_buffer_.watt * P_COEFF / p_gain);
    if (abs(i_rms_mA - i_rms_mA_) > 10) i_rms_mA_ = i_rms_mA;
    if (abs(v_rms_mV - v_rms_mV_) > 1000) v_rms_mV_ = v_rms_mV;
    if (abs(p_rms_mW - p_rms_mW_) > 500) p_rms_mW_ = p_rms_mW;
    // rounding to uint32_t is broken
    e_mWh_ = energy_store_.update(packet_buffer_.cf_cnt) * E_COEFF / p_gain;
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

void BL0942::set_energy(uint32_t e_mW) {
  double p_gain = i_gain_ * v_gain_;
  energy_store_.set(round(e_mW * p_gain / E_COEFF));
}
