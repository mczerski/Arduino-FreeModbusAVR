#ifndef _MB_WITH_WDT_H
#define _MB_WITH_WDT_H

#include "mb.h"
#include "SPIFlash.h"
#include <avr/wdt.h>

typedef struct {
  USHORT fileNumber;
  USHORT fileSize;
  uint8_t fileOffset;
} MBFile;

extern MBFile fileTable[];

eMBErrorCode eMBInitWithWDT(
    eMBMode eMode,
    UCHAR ucPort,
    UCHAR ucWdtValue,
    UCHAR ucSlaveID,
    UCHAR const * pucAdditional,
    USHORT usAdditionalLen,
    SPIFlash * flash = nullptr,
    int led_pin = -1);
eMBErrorCode eMBPollWithWDT();

template <typename S>
union ModbusDataBlock {
  static constexpr size_t mbLength = sizeof(S) / sizeof(uint16_t);
  S s;
  uint16_t data[mbLength];
};

template <typename S>
eMBErrorCode eMBCopyToRegBuffer(const ModbusDataBlock<S> & dataBlock, UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs)
{
  if (usAddress >= 1 and usAddress + usNRegs <= ModbusDataBlock<S>::mbLength) {
    uint16_t * buf = reinterpret_cast<uint16_t*>(pucRegBuffer);
    for (size_t i=0; i<usNRegs; i++) {
      buf[i] = __builtin_bswap16(dataBlock.data[i + usAddress - 1]);
    }
    return MB_ENOERR;
  }
  return MB_ENOREG;
}

#endif
