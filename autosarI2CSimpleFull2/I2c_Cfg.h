/******************************************************************************
* AUTOSAR I2C Driver - Configuration Header
* File: I2c_Cfg.h
******************************************************************************/

#ifndef I2C_CFG_H
#define I2C_CFG_H

#include "I2c.h"

/* ============ CONFIGURATION SETTINGS ============ */

/* Development Error Detection */
#define I2C_DEV_ERROR_DETECT      STD_ON

/* Version Info API */
#define I2C_VERSION_INFO_API      STD_ON

/* Number of channels */
#define I2C_NUM_CHANNELS          2U

/* Number of jobs */
#define I2C_NUM_JOBS              4U

/* Number of sequences */
#define I2C_NUM_SEQUENCES         2U

/* Timeout value in MainFunction cycles */
#define I2C_TIMEOUT_VALUE         1000U

/* ============ CHANNEL CONFIGURATION ============ */

/* Channel 0 - Controller mode */
#define I2C_CH0_HW_BASE_ADDR      0x40000000U
#define I2C_CH0_BAUD_RATE         100U  /* 100 kbit/s */
#define I2C_CH0_MODE              I2C_HW_UNIT_MODE_CONTROLLER
#define I2C_CH0_TARGET_LISTENING  FALSE

/* Channel 1 - Target mode */
#define I2C_CH1_HW_BASE_ADDR      0x40001000U
#define I2C_CH1_BAUD_RATE         100U  /* 100 kbit/s */
#define I2C_CH1_MODE              I2C_HW_UNIT_MODE_TARGET
#define I2C_CH1_TARGET_LISTENING  TRUE

/* ============ JOB CONFIGURATION ============ */

/* Job IDs */
#define I2C_JOB_READ_TEMP         0U
#define I2C_JOB_WRITE_CONFIG      1U
#define I2C_JOB_READ_STATUS       2U
#define I2C_JOB_WRITE_DATA        3U

/* Device addresses */
#define I2C_DEV_ADDR_TEMP_SENSOR  0x48U
#define I2C_DEV_ADDR_EEPROM       0x50U

/* ============ SEQUENCE CONFIGURATION ============ */

/* Sequence IDs */
#define I2C_SEQ_READ_SENSOR       0U
#define I2C_SEQ_WRITE_EEPROM      1U

#endif /* I2C_CFG_H */