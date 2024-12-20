#ifndef BL0942_h
#define BL0942_h

#include <Arduino.h>
#include "Duration.h"

struct DataPacket {
  uint32_t i_rms : 24;
  uint32_t v_rms : 24;
  uint32_t i_fast_rms : 24;
  int32_t watt : 24;
  uint32_t cf_cnt : 24;
  uint16_t frequency;
  uint8_t reserved1;
  uint8_t status;
  uint8_t reserved2;
  uint8_t reserved3;
  uint8_t checksum;
} __attribute__((packed)) __attribute__((scalar_storage_order("little-endian")));

struct DataReg {
  uint32_t value : 24;
  uint8_t checksum;
} __attribute__((packed)) __attribute__((scalar_storage_order("little-endian")));

class EnergyStore {
  uint32_t base_cf_cnt_stored_ = 0;
  uint32_t base_cf_cnt_received_ = 0;
  int eeprom_addr_;
  bool init_done_ = false;
  mys_toolkit::Duration last_store_timestamp_;
public:
  EnergyStore(int eeprom_addr);
  uint32_t update(uint32_t cf_cnt);
  void set(uint32_t cf_cnt);
};

class BL0942 {
  static constexpr byte WRITE_COMMAND = 0xA8;
  static constexpr byte READ_COMMAND = 0x58;
  static constexpr byte FULL_PACKET = 0xAA;
  static constexpr byte PACKET_HEADER = 0x55;
  static constexpr byte I_FAST_RMS_TH = 0x15;
  static constexpr byte OT_FUNX  = 0x18;
  static constexpr byte USR_WRPROT = 0x1D;
  static constexpr double V_REF = 1.218;
  static constexpr double I_R = 0.001;
  static constexpr double I_COEFF = V_REF / 305978 / I_R;
  static constexpr double I_FAST_COEFF = I_COEFF / 0.363;
  static constexpr double V_DIV_R1 = 510;
  static constexpr double V_DIV_R2 = 5*390000;
  static constexpr double V_DIV_COEFF = V_DIV_R1 / (V_DIV_R1 + V_DIV_R2);
  static constexpr double V_COEFF = V_REF / 73989 / V_DIV_COEFF;
  static constexpr double P_COEFF = V_REF * V_REF / 3537 / I_R / V_DIV_COEFF / 1000;
  static constexpr double E_COEFF = 1638.4 * 256 * P_COEFF / 3600;
  Stream &serial_;
  int32_t i_rms_mA_ = 0;
  int32_t v_rms_mV_ = 0;
  int32_t p_rms_mW_ = 0;
  uint32_t e_mWh_ = 0;
  int32_t i_fast_rms_th_mA_ = 0;
  byte cf_output_ = 0;
  byte last_read_cmd_ = FULL_PACKET;
  mys_toolkit::Duration last_read_timestamp_{0};
  EnergyStore energy_store_;
  float i_gain_ = 1.0;
  float v_gain_ = 1.0;
  bool readPacket_();
  bool readReg_(byte reg);
  void readCmd_(byte reg);
  byte calcChecksum_(byte reg, const byte* buffer, size_t size) const;
  void writeReg_(byte address, uint32_t data);
public:
  DataPacket packet_buffer_;
  DataReg reg_buffer_;
  BL0942(Stream &serial, int eeprom_addr, float i_gain = 1.0, float v_gain = 1.0);
  void begin();
  bool update();
  int32_t getIRms_mA() const;
  int32_t getVRms_mV() const;
  int32_t getPRms_mW() const;
  uint32_t getE_mWh() const;
  int32_t getIFastRmsTh_mA() const;
  byte getCfOutput() const;
  void calibrate(float i_gain = 1.0, float v_gain = 1.0);
  void set_energy(uint32_t e_mW);
};

#endif //BL0942_h
