/**
 * @file    bat32a2x9_simplified_i2c.h
 * @brief   Header file for Simplified I2C Driver (Chapter 19)
 */

#ifndef BAT32A2X9_SIMPLIFIED_I2C_H
#define BAT32A2X9_SIMPLIFIED_I2C_H

#include <stdint.h>
#include <stdbool.h>

/* I2C Channel IDs */
typedef enum {
    IIC00 = 0,
    IIC01 = 1,
    IIC10 = 2,
    IIC11 = 3,
    IIC20 = 4,
    IIC21 = 5,
    IIC30 = 6,
    IIC31 = 7,
    I2C_CHANNEL_MAX
} I2C_Channel_ID_t;

/* I2C Status */
typedef enum {
    I2C_STATUS_OK = 0,
    I2C_STATUS_BUS_BUSY,
    I2C_STATUS_ARBITRATION_LOST,
    I2C_STATUS_OVERRUN_ERROR,
    I2C_STATUS_NO_ACK,
    I2C_STATUS_TIMEOUT,
    I2C_STATUS_NOT_INITIALIZED,
    I2C_STATUS_ERROR
} I2C_Status_t;

/* Pin Definitions (from datasheet) */
#define P43 0x43
#define P44 0x44
#define P46 0x46
#define P47 0x47
#define P53 0x53
#define P54 0x54
#define P56 0x56
#define P57 0x57
#define P63 0x63
#define P64 0x64
#define P66 0x66
#define P67 0x67
#define P73 0x73
#define P74 0x74
#define P76 0x76
#define P77 0x77

/* GPIO Register Macros (simplified) */
extern volatile uint8_t POM4, POM5, POM6, POM7;

/* Function Prototypes */

/* Initialization */
bool I2C_Master_Init(I2C_Channel_ID_t id, uint32_t frequency);
bool I2C_Slave_Init(I2C_Channel_ID_t id, uint8_t slave_address);

/* Master Operations */
bool I2C_Master_Write(I2C_Channel_ID_t id, uint8_t slave_addr, 
                      const uint8_t *data, uint16_t length, bool stop);
bool I2C_Master_Read(I2C_Channel_ID_t id, uint8_t slave_addr,
                     uint8_t *buffer, uint16_t length, bool stop);
bool I2C_Master_WriteRead(I2C_Channel_ID_t id, uint8_t slave_addr,
                          const uint8_t *write_data, uint16_t write_len,
                          uint8_t *read_data, uint16_t read_len);

/* Slave Operations */
bool I2C_Slave_Transmit(I2C_Channel_ID_t id, const uint8_t *data, uint16_t length);
bool I2C_Slave_Receive(I2C_Channel_ID_t id, uint8_t *buffer, uint16_t max_len, 
                       uint16_t *received_len);

/* Status and Control */
I2C_Status_t I2C_GetStatus(I2C_Channel_ID_t id);
bool I2C_ResetBus(I2C_Channel_ID_t id);

/* Test Functions */
void I2C_TestAllChannels(void);
void I2C_TestLoopback(I2C_Channel_ID_t master_id, I2C_Channel_ID_t slave_id, 
                      uint8_t slave_addr);

#endif /* BAT32A2X9_SIMPLIFIED_I2C_H */