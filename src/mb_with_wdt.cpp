#include "mb_with_wdt.h"
#include "led.h"
#include "SPIFlash.h"
#include <EEPROM.h>

static SPIFlash *flash_ = nullptr;

BOOL resetMCU = FALSE;
const USHORT MB_FILE_IMAGE = 1;
const USHORT MB_FILE_HEADER = 2;
MBFile fileTable[] = {
    {.fileNumber = MB_FILE_IMAGE, .fileSize = 32*1024 - 10, .fileOffset = 10},
    {.fileNumber = MB_FILE_HEADER, .fileSize = 10, .fileOffset = 0},
    {.fileNumber = 0, .fileSize = 0, .fileOffset = 0}
};
static ULONG baudrateTable[16] = {
    2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 76800, 115200, 230400, 250000, 460800, 500000, 921600, 1000000
};

eMBErrorCode eMBInitWithWDT(
    eMBMode eMode,
    UCHAR ucPort,
    UCHAR ucWdtValue,
    UCHAR ucSlaveID,
    UCHAR const * pucAdditional,
    USHORT usAdditionalLen,
    SPIFlash *flash,
    int led_pin)
{
    UCHAR ucSlaveAddress = 1;
    ULONG ulBaudRate = 9600;
    eMBParity eParity = MB_PAR_EVEN;
    uint16_t config = (EEPROM.read(0) << 8) | EEPROM.read(1);
    if (config != 0xffff) {
        ucSlaveAddress = config & 0xff;
        if (ucSlaveAddress == 0) {
            ucSlaveAddress = 1;
        }
        ulBaudRate = baudrateTable[(config >> 8) & 0xf];
        eParity = (config >> 12) & 0x3;
    }
    init_led(led_pin);
    wdt_enable(ucWdtValue);
    flash_ = flash;
    if (flash_ and !flash_->initialize())
        error(MB_EIO);
    eMBErrorCode status = eMBInit(eMode, ucSlaveAddress, ucPort, ulBaudRate, eParity);
    if (status != MB_ENOERR) error(status);
    status = eMBSetSlaveID(ucSlaveID, true, pucAdditional, usAdditionalLen);
    if (status != MB_ENOERR) error(status);
    status = eMBEnable();
    if (status != MB_ENOERR) error(status);
    trigger_led();
    return;
}

eMBErrorCode eMBPollWithWDT()
{
    if (!resetMCU)
        wdt_reset();
    eMBPoll();
    update_led();
}

eMBErrorCode eMBRegFileCB(UCHAR * pucFileBuffer, USHORT usFileNumber, USHORT usRecordNumber, USHORT usRecordLength, eMBRegisterMode eMode) {
  if (flash_ and eMode == MB_REG_WRITE) {
    for (const MBFile *mbFile = fileTable; mbFile->fileNumber != 0; mbFile++) {
        if (mbFile->fileNumber == usFileNumber && mbFile->fileSize >= 2 * (usRecordNumber + usRecordLength)) {
            if (usFileNumber == MB_FILE_IMAGE && usRecordNumber == 0) {
                flash_->blockErase32K(0);
                while (flash_->busy());
            }
            flash_->writeBytes(mbFile->fileOffset + 2 * usRecordNumber, pucFileBuffer, 2 * usRecordLength);
            while (flash_->busy());
            // reset MCU by not reseting watchdog timer
            if (usFileNumber == MB_FILE_HEADER && usRecordNumber == 0 and mbFile->fileSize == 2 *usRecordLength) {
                resetMCU = TRUE;
            }
            return MB_ENOERR;
        }
    }
  }
  else if (flash_ and eMode == MB_REG_READ) {
    for (const MBFile *mbFile = fileTable; mbFile->fileNumber != 0; mbFile++) {
        if (mbFile->fileNumber == usFileNumber && mbFile->fileSize >= 2 * (usRecordNumber + usRecordLength)) {
            flash_->readBytes(mbFile->fileOffset + 2 * usRecordNumber, pucFileBuffer, 2 * usRecordLength);
            return MB_ENOERR;
        }
    }
  }
  return MB_ENOREG;
}

extern eMBErrorCode eMBRegHoldingCB2(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode);

eMBErrorCode eMBRegHoldingCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode) {
  if (eMode == MB_REG_READ and usAddress == 100 and usNRegs == 1) {
    pucRegBuffer[0] = EEPROM.read(0);
    pucRegBuffer[1] = EEPROM.read(1);
    return MB_ENOERR;
  }
  if (eMode == MB_REG_WRITE and usAddress == 100 and usNRegs == 1) {
    if (0xfe >= pucRegBuffer[1] & 0xff >= 1 and (pucRegBuffer[0] >> 12) <= 3) {
      EEPROM.write(0, pucRegBuffer[0]);
      EEPROM.write(1, pucRegBuffer[1]);
      resetMCU = TRUE;
      return MB_ENOERR;
    }
    else {
      return MB_EINVAL;
    }
  }
  return eMBRegHoldingCB2(pucRegBuffer, usAddress, usNRegs, eMode);
}
