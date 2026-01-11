#ifndef I2C_PRIVATE_H
#define I2C_PRIVATE_H

#include "I2c.h"

/* Internal function prototypes */
static I2c_HwRegType* I2c_GetHwReg(uint8 Channel);
static void I2c_CalculateTiming(uint32 BaudRate, uint8* iicwl, uint8* iicwh);
static void I2c_StartHwTransmit(uint8 Channel, const I2c_JobConfigType* Job,
                               const I2c_DataBufferType* Buffer);
static void I2c_HandleTransmissionComplete(uint8 Channel);
static void I2c_HandleTargetRequest(uint8 Channel);
static void I2c_ProcessNextQueuedSequence(uint8 Channel);
static void I2c_CleanupChannel(uint8 Channel);

/* System tick function (to be implemented by OS/BSP) */
extern uint32 GetSystemTick(void);

#endif /* I2C_PRIVATE_H */