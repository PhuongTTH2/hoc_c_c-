#ifndef I2C_CFG_H
#define I2C_CFG_H

#include "I2C_Types.h"

/* ========================== I2cGeneral ========================== */
#define I2C_DEV_ERROR_DETECT          STD_ON
#define I2C_VERSION_INFO_API          STD_ON

/* ========================== I2cChannel ========================== */
#define I2C_NUMBER_OF_CHANNELS        2

#define I2C_CHANNEL_0_BAUDRATE        100
#define I2C_CHANNEL_0_BASE_ADDRESS    0x40041800UL
#define I2C_CHANNEL_0_MODE            0      /* Controller mode */
#define I2C_CHANNEL_0_TARGET_LISTEN   STD_OFF

#define I2C_CHANNEL_1_BAUDRATE        100
#define I2C_CHANNEL_1_BASE_ADDRESS    0x40042C00UL
#define I2C_CHANNEL_1_MODE            1      /* Target mode */
#define I2C_CHANNEL_1_TARGET_LISTEN   STD_ON

/* ========================== I2cJob ========================== */
#define I2C_NUMBER_OF_JOBS            3

#define I2C_JOB_0_DEVICE_ADDRESS      0x50
#define I2C_JOB_0_ID                  0

#define I2C_JOB_1_DEVICE_ADDRESS      0x50
#define I2C_JOB_1_ID                  1

#define I2C_JOB_2_DEVICE_ADDRESS      0x48
#define I2C_JOB_2_ID                  2

/* ========================== I2cSequence ========================== */
#define I2C_NUMBER_OF_SEQUENCES       2

/* Sequence 0 */
#define I2C_SEQUENCE_0_END_NOTIFICATION   "EEPROM_SequenceComplete"
#define I2C_SEQUENCE_0_ID                 0
#define I2C_SEQUENCE_0_CHANNEL            0
#define I2C_SEQUENCE_0_JOBS               {0, 1}

/* Sequence 1 */
#define I2C_SEQUENCE_1_END_NOTIFICATION   "Sensor_SequenceComplete"
#define I2C_SEQUENCE_1_ID                 1
#define I2C_SEQUENCE_1_CHANNEL            0
#define I2C_SEQUENCE_1_JOBS               {2}

/* ========================== Exports ========================== */
extern const I2C_ConfigType I2c_Config;

/* Notification function declarations */
extern void EEPROM_SequenceComplete(I2C_SequenceType SequenceId, I2C_SequenceResultType Result);
extern void Sensor_SequenceComplete(I2C_SequenceType SequenceId, I2C_SequenceResultType Result);

#endif /* I2C_CFG_H */