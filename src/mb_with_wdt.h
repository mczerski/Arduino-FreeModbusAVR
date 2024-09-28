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
    SPIFlash * flash = nullptr);
eMBErrorCode eMBPollWithWDT();

#endif
