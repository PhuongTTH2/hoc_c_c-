#ifndef I2C_CFG_H
#define I2C_CFG_H

#include "Std_Types.h"

/* ================= AUTOSAR CONFIGURATION ================= */
/* [CP_SWS_I2C_00243] Development Error Detection */
#define I2C_DEV_ERROR_DETECT                STD_ON

/* [CP_SWS_I2C_00244] Version Info API */
#define I2C_VERSION_INFO_API                STD_ON

/* [CP_SWS_I2C_00245] Support Synchronous Transmission */
#define I2C_SYNC_TRANSMIT_SUPPORT           STD_ON

/* [CP_SWS_I2C_00246] Support Asynchronous Transmission */
#define I2C_ASYNC_TRANSMIT_SUPPORT          STD_ON

/* [CP_SWS_I2C_00247] Support Target Mode */
#define I2C_TARGET_MODE_SUPPORT             STD_OFF  /* Simplified I2C không hỗ trợ! */

/* ================= HARDWARE CONFIGURATION ================= */
/* Số lượng kênh Simplified I2C */
#define I2C_NUMBER_OF_CHANNELS              2

/* Số lượng Jobs tối đa */
#define I2C_NUMBER_OF_JOBS                  4

/* Số lượng Sequences tối đa */
#define I2C_NUMBER_OF_SEQUENCES             2

/* Kích thước FIFO queue */
#define I2C_FIFO_QUEUE_SIZE                 8

/* Số Jobs tối đa trong 1 Sequence */
#define I2C_MAX_JOBS_PER_SEQUENCE           4

/* ================= CHANNEL CONFIGURATION ================= */
/* Channel 0: Simplified I2C0 */
#define I2C_CHANNEL_0_BAUDRATE              100000UL   /* 100kHz */
#define I2C_CHANNEL_0_MODE                  I2C_HW_UNIT_MODE_CONTROLLER  /* Chỉ master */
#define I2C_CHANNEL_0_BASE_ADDRESS          ((uint32)0x40030000)  /* USCU base */

/* Channel 1: Simplified I2C1 */
#define I2C_CHANNEL_1_BAUDRATE              400000UL   /* 400kHz */
#define I2C_CHANNEL_1_MODE                  I2C_HW_UNIT_MODE_CONTROLLER
#define I2C_CHANNEL_1_BASE_ADDRESS          ((uint32)0x40030010)

/* ================= JOB CONFIGURATION ================= */
#define I2C_JOB_0_ID                        0
#define I2C_JOB_0_DEVICE_ADDRESS            0x50      /* EEPROM */

#define I2C_JOB_1_ID                        1
#define I2C_JOB_1_DEVICE_ADDRESS            0x48      /* Temperature sensor */

#define I2C_JOB_2_ID                        2
#define I2C_JOB_2_DEVICE_ADDRESS            0x29      /* Light sensor */

#define I2C_JOB_3_ID                        3
#define I2C_JOB_3_DEVICE_ADDRESS            0x68      /* RTC */

/* ================= SEQUENCE CONFIGURATION ================= */
#define I2C_SEQUENCE_0_ID                   0
#define I2C_SEQUENCE_0_CHANNEL              0
#define I2C_SEQUENCE_0_JOBS                 {0, 1}    /* Job 0 và 1 */

#define I2C_SEQUENCE_1_ID                   1
#define I2C_SEQUENCE_1_CHANNEL              1
#define I2C_SEQUENCE_1_JOBS                 {2, 3}    /* Job 2 và 3 */

/* ================= HARDWARE REGISTERS ================= */
/* SPSm (Serial clock selection register) values for Simplified I2C */
#define SPS_DIV_1                           0x00      /* fCLK/1 */
#define SPS_DIV_4                           0x01      /* fCLK/4 */
#define SPS_DIV_16                          0x02      /* fCLK/16 */
#define SPS_DIV_64                          0x03      /* fCLK/64 */

/* Pin configuration for Simplified I2C */
#define I2C_PIN_SDA00                       15        /* P15 */
#define I2C_PIN_SCL00                       14        /* P14 */
#define I2C_PIN_SDA01                       17        /* P17 */
#define I2C_PIN_SCL01                       16        /* P16 */

#endif /* I2C_CFG_H */