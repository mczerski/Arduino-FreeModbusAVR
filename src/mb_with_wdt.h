#ifndef _MB_WITH_WDT_H
#define _MB_WITH_WDT_H

#include "mb.h"
#include <avr/wdt.h>

typedef struct {
  USHORT fileNumber;
  USHORT fileSize;
  uint8_t fileOffset;
} MBFile;

extern MBFile fileTable[];

eMBErrorCode eMBInitWithWDT(
    eMBMode eMode,
    UCHAR ucSlaveAddress,
    UCHAR ucPort,
    ULONG ulBaudRate,
    eMBParity eParity,
    UCHAR ucWdtValue,
    UCHAR ucVersionMajor,
    UCHAR ucVersionMinor);
eMBErrorCode eMBPollWithWDT();

#endif
