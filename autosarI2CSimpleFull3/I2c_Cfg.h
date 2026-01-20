/******************************************************************************
* AUTOSAR I2C Driver - Configuration Header
* File: I2c_Cfg.h
* BAT32A2x9 Specific Configuration
******************************************************************************/

#ifndef I2C_CFG_H
#define I2C_CFG_H

#include "Std_Types.h"

/* ============ MODULE CONFIGURATION ============ */
#define I2C_DEV_ERROR_DETECT            STD_ON
#define I2C_VERSION_INFO_API            STD_OFF

/* ============ HARDWARE CONSTANTS (BAT32A2x9) ============ */

/* Simplified I2C (Chapter 19) - IIC00 Registers */
#define I2C_SIMPLE_SMR00                (*((volatile uint8*)0x4004A020))
#define I2C_SIMPLE_SCR00                (*((volatile uint8*)0x4004A021))
#define I2C_SIMPLE_SDR00                (*((volatile uint8*)0x4004A022))
#define I2C_SIMPLE_SSR00                (*((volatile uint8*)0x4004A023))

/* IICA (Chapter 20) - IICA0 Registers */
#define IICA0_IICCTL00                  (*((volatile uint8*)0x4004B000))
#define IICA0_IICS0                     (*((volatile uint8*)0x4004B001))
#define IICA0_IICF0                     (*((volatile uint8*)0x4004B002))
#define IICA0_IICCTL10                  (*((volatile uint8*)0x4004B003))
#define IICA0_SVA0                      (*((volatile uint8*)0x4004B004))

/* Peripheral Enable Registers */
#define PER0_REG                        (*((volatile uint8*)0x40040800))
#define PER2_REG                        (*((volatile uint8*)0x40040802))

/* Port Control Registers */
#define PM00_REG                        (*((volatile uint8*)0x40040020))
#define PU00_REG                        (*((volatile uint8*)0x40040030))
#define POM00_REG                       (*((volatile uint8*)0x40040050))

#define PM10_REG                        (*((volatile uint8*)0x40040021))
#define PU10_REG                        (*((volatile uint8*)0x40040031))
#define POM10_REG                       (*((volatile uint8*)0x40040051))

/* ============ CHANNEL CONFIGURATION ============ */
#define I2C_NUM_CHANNELS                2U

/* Channel IDs */
#define I2C_CHANNEL_SIMPLE_MASTER       0U  /* Simplified I2C (Master Only) */
#define I2C_CHANNEL_IICA_MASTER_SLAVE   1U  /* IICA (Master/Slave) */

/* ============ JOB CONFIGURATION ============ */
#define I2C_NUM_JOBS                    5U

/* Job IDs */
#define I2C_JOB_WRITE_TEMP_CONFIG       0U
#define I2C_JOB_READ_TEMP_DATA          1U
#define I2C_JOB_WRITE_EEPROM_DATA       2U
#define I2C_JOB_READ_EEPROM_DATA        3U
#define I2C_JOB_SLAVE_RECEIVE           4U

/* ============ SEQUENCE CONFIGURATION ============ */
#define I2C_NUM_SEQUENCES               4U

/* Sequence IDs */
#define I2C_SEQUENCE_TEMP_SENSOR        0U
#define I2C_SEQUENCE_EEPROM_WRITE       1U
#define I2C_SEQUENCE_EEPROM_READ        2U
#define I2C_SEQUENCE_SLAVE_RECEIVE      3U

/* ============ DEVICE ADDRESSES ============ */
#define I2C_ADDR_TEMP_SENSOR            0x48U  /* Temperature sensor */
#define I2C_ADDR_EEPROM                 0x50U  /* EEPROM */
#define I2C_SLAVE_ADDRESS               0x20U  /* Our slave address */

/* ============ PIN CONFIGURATION ============ */
/* Simplified I2C uses P03(SDA10) and P04(SCL10) */
#define I2C_SIMPLE_SDA_PIN              3U
#define I2C_SIMPLE_SCL_PIN              4U

/* IICA uses P10(SDA11) and P11(SCL11) */
#define IICA_SDA_PIN                    0U
#define IICA_SCL_PIN                    1U

/* ============ DEM EVENT IDs ============ */
#define I2C_EVENT_ID_TEMP_FAIL          100U
#define I2C_EVENT_ID_EEPROM_FAIL        101U
#define I2C_EVENT_ID_TIMEOUT            102U
#define I2C_EVENT_ID_ARBITRATION_LOST   103U

#endif /* I2C_CFG_H */