#ifndef I2C_CFG_H
#define I2C_CFG_H

/* Module Version */
#define I2C_SW_MAJOR_VERSION   1U
#define I2C_SW_MINOR_VERSION   0U
#define I2C_SW_PATCH_VERSION   0U

/* General Parameters */
#define I2C_DEV_ERROR_DETECT   STD_ON
#define I2C_VERSION_INFO_API   STD_ON

/* Queue Configuration */
#define I2C_MAX_QUEUE_SIZE     10U

/* Number of Channels, Jobs, Sequences */
#define I2C_NUM_CHANNELS       2U
#define I2C_NUM_JOBS           4U
#define I2C_NUM_SEQUENCES      2U

/* Channel IDs */
#define I2C_CHANNEL_0          0U
#define I2C_CHANNEL_1          1U

/* Job IDs */
#define I2C_JOB_EEPROM_WRITE   0U
#define I2C_JOB_EEPROM_READ    1U
#define I2C_JOB_SENSOR_READ    2U
#define I2C_JOB_SENSOR_WRITE   3U

/* Sequence IDs */
#define I2C_SEQ_EEPROM_ACCESS  0U
#define I2C_SEQ_SENSOR_ACCESS  1U

/* HW Unit Modes */
#define I2C_HW_UNIT_MODE_CONTROLLER  0U
#define I2C_HW_UNIT_MODE_TARGET      1U

/* Error Status Values */
#define I2C_NO_ERR             0x00U
#define I2C_NACK_RECEIVED_ERR  0x02U
#define I2C_ARBITRATION_LOST_ERR 0x01U
#define I2C_BUS_FAILURE_ERR    0x03U
#define I2C_FIFO_OVERFLOW_ERR  0x04U
#define I2C_TIMEOUT_ERR        0x05U

#endif /* I2C_CFG_H */