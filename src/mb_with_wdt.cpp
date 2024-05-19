#include "mb_with_wdt.h"
#include "SPIFlash.h"

SPIFlash flash(8, 0x1F65);

BOOL resetMCU = FALSE;
const USHORT MB_FILE_IMAGE = 1;
const USHORT MB_FILE_HEADER = 2;
const USHORT MB_FILE_VERSION = 3;
MBFile fileTable[] = {
    {.fileNumber = MB_FILE_IMAGE, .fileSize = 32*1024 - 10, .fileOffset = 10},
    {.fileNumber = MB_FILE_HEADER, .fileSize = 10, .fileOffset = 0},
    {.fileNumber = 0, .fileSize = 0, .fileOffset = 0}
};
static UCHAR versionMajor = 0;
static UCHAR versionMinor = 0;

eMBErrorCode eMBInitWithWDT(
    eMBMode eMode,
    UCHAR ucSlaveAddress,
    UCHAR ucPort,
    ULONG ulBaudRate,
    eMBParity eParity,
    UCHAR ucWdtValue,
    UCHAR ucVersionMajor,
    UCHAR ucVersionMinor)
{
    versionMajor = ucVersionMajor;
    versionMinor = ucVersionMinor;
    wdt_enable(ucWdtValue);
    if (!flash.initialize())
        return MB_EIO;
    return eMBInit(eMode, ucSlaveAddress, ucPort, ulBaudRate, eParity);
}

eMBErrorCode eMBPollWithWDT()
{
    if (!resetMCU)
        wdt_reset();
    eMBPoll();
}

eMBErrorCode eMBRegFileCB(UCHAR * pucFileBuffer, USHORT usFileNumber, USHORT usRecordNumber, USHORT usRecordLength, eMBRegisterMode eMode) {
  if (eMode == MB_REG_WRITE) {
    for (const MBFile *mbFile = fileTable; mbFile->fileNumber != 0; mbFile++) {
        if (mbFile->fileNumber == usFileNumber && mbFile->fileSize >= 2 * (usRecordNumber + usRecordLength)) {
            if (usFileNumber == MB_FILE_IMAGE && usRecordNumber == 0) {
                flash.blockErase32K(0);
                while (flash.busy());
            }
            flash.writeBytes(mbFile->fileOffset + 2 * usRecordNumber, pucFileBuffer, 2 * usRecordLength);
            while (flash.busy());
            // reset MCU by not reseting watchdog timer
            if (usFileNumber == MB_FILE_HEADER && usRecordNumber == 0 and mbFile->fileSize == 2 *usRecordLength) {
                resetMCU = TRUE;
            }
            return MB_ENOERR;
        }
    }
  }
  else if (eMode == MB_REG_READ) {
    if (usFileNumber == MB_FILE_VERSION && usRecordNumber == 0 && usRecordLength == 1) {
        pucFileBuffer[0] = versionMajor;
        pucFileBuffer[1] = versionMinor;
        return MB_ENOERR;
    }
    for (const MBFile *mbFile = fileTable; mbFile->fileNumber != 0; mbFile++) {
        if (mbFile->fileNumber == usFileNumber && mbFile->fileSize >= 2 * (usRecordNumber + usRecordLength)) {
            flash.readBytes(mbFile->fileOffset + 2 * usRecordNumber, pucFileBuffer, 2 * usRecordLength);
            return MB_ENOERR;
        }
    }
  }
  return MB_ENOREG;
}
