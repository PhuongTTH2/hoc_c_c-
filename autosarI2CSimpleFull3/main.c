/******************************************************************************
* Application Example - Using AUTOSAR I2C Driver
* File: App_I2C.c
******************************************************************************/

#include "I2c.h"
#include "I2c_Cfg.h"

/* Global buffers */
static uint8 g_tempConfig[2] = {0x01U, 0x80U};  /* Register + value */
static uint8 g_tempData[2];
static uint8 g_eepromData[16];
static uint8 g_slaveBuffer[8];

void App_Init(void)
{
    /* Initialize I2C driver */
    I2C_Init(&I2C_Config);
}

void App_ReadTemperature(void)
{
    /* Setup temperature sensor jobs */
    (void)I2C_SetupEB(I2C_JOB_WRITE_TEMP_CONFIG,
                      I2C_ADDR_TEMP_SENSOR,
                      g_tempConfig,
                      NULL,
                      2U);
    
    (void)I2C_SetupEB(I2C_JOB_READ_TEMP_DATA,
                      I2C_ADDR_TEMP_SENSOR,
                      NULL,
                      g_tempData,
                      2U);
    
    /* Start asynchronous read */
    (void)I2C_AsyncTransmit(I2C_SEQUENCE_TEMP_SENSOR);
}

void App_WriteEeprom(uint16 address, const uint8* data, uint16 length)
{
    /* Prepare EEPROM write buffer (address + data) */
    static uint8 eepromBuffer[18];
    eepromBuffer[0] = (uint8)(address >> 8);
    eepromBuffer[1] = (uint8)(address & 0xFF);
    
    for (uint16 i = 0U; i < length && i < 16U; i++)
    {
        eepromBuffer[i + 2U] = data[i];
    }
    
    (void)I2C_SetupEB(I2C_JOB_WRITE_EEPROM_DATA,
                      I2C_ADDR_EEPROM,
                      eepromBuffer,
                      NULL,
                      (uint16)(length + 2U));
    
    (void)I2C_AsyncTransmit(I2C_SEQUENCE_EEPROM_WRITE);
}

void App_ReadEeprom(uint16 address, uint16 length)
{
    /* First write address, then read data */
    static uint8 addrBuffer[2];
    addrBuffer[0] = (uint8)(address >> 8);
    addrBuffer[1] = (uint8)(address & 0xFF);
    
    (void)I2C_SetupEB(I2C_JOB_WRITE_EEPROM_DATA,
                      I2C_ADDR_EEPROM,
                      addrBuffer,
                      NULL,
                      2U);
    
    (void)I2C_SetupEB(I2C_JOB_READ_EEPROM_DATA,
                      I2C_ADDR_EEPROM,
                      NULL,
                      g_eepromData,
                      length);
    
    /* Note: This is simplified - actual sequence would need two jobs */
    (void)I2C_AsyncTransmit(I2C_SEQUENCE_EEPROM_READ);
}

void App_StartSlaveMode(void)
{
    /* Setup slave receive job */
    (void)I2C_SetupEB(I2C_JOB_SLAVE_RECEIVE,
                      I2C_SLAVE_ADDRESS,
                      NULL,
                      g_slaveBuffer,
                      8U);
    
    /* Start listening as slave */
    (void)I2C_StartListening(I2C_SEQUENCE_SLAVE_RECEIVE);
}

void App_CyclicTask(void)
{
    /* Call I2C MainFunction periodically */
    I2C_MainFunction();
    
    /* Check temperature read status */
    I2C_SequenceResultType result = I2C_GetSequenceResult(I2C_SEQUENCE_TEMP_SENSOR);
    
    if (result == I2C_SEQUENCE_OK)
    {
        /* Process temperature data */
        int16 temperature = (g_tempData[0] << 8) | g_tempData[1];
        /* Send to RTE or process further */
    }
}