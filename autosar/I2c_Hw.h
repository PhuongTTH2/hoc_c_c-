#ifndef I2C_HW_H
#define I2C_HW_H

#include "Std_Types.h"
#include "I2c_Cfg.h"

/* Hardware Functions */
Std_ReturnType I2c_HwInit(uint8 Channel, const I2c_ChannelConfigType* Config);
void I2c_HwDeInit(uint8 Channel);

Std_ReturnType I2c_HwStartTransmit(uint8 Channel, uint8 Address, boolean IsWrite);
Std_ReturnType I2c_HwSendByte(uint8 Channel, uint8 Data);
Std_ReturnType I2c_HwReceiveByte(uint8 Channel, uint8* Data, boolean SendAck);
Std_ReturnType I2c_HwGenerateStop(uint8 Channel);
Std_ReturnType I2c_HwStartListening(uint8 Channel);

boolean I2c_HwIsBusBusy(uint8 Channel);
boolean I2c_HwIsTransferComplete(uint8 Channel);
uint8 I2c_HwGetStatus(uint8 Channel);
void I2c_HwClearErrors(uint8 Channel);

/* Interrupt Handlers */
void I2C0_IRQHandler(void);
void I2C1_IRQHandler(void);

#endif /* I2C_HW_H */