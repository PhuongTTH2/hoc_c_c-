/* Example application using I2C Driver */
// Module Parameter	Pre-Compile	Post-Build	Lý do
// I2cDevErrorDetect	✓	✗	Safety - Không cho disable lỗi runtime
// I2cVersionInfoApi	✓	✗	Debug feature - Fixed at compile
// I2cBaudRate	        ✓	✓	Có thể tune cho từng ECU
// I2cHwUnitBaseAddress	✗	✓	Phụ thuộc hardware layout
// I2cTargetListening	✓	✓	Tùy application requirement
// I2cDeviceAddress	    ✓	✓	Có thể thay đổi sensor
// I2cEndNotification	✗	✓	Callback phụ thuộc application

#include "I2c.h"
#include "I2c_PBcfg.h"

/* Data buffers */
uint8 temperatureData[2];
uint8 configData[1] = {0x01};
uint8 eepromData[64];

int main(void)
{
    Std_ReturnType ret;
    I2C_SequenceResultType seqResult;
    
    /* 1. Initialize I2C Driver */
    I2C_Init(&I2C_Config);
    
    /* 2. Setup EB for reading temperature sensor */
    ret = I2C_SetupEB(
        I2C_JOB_WRITE_CONFIG,    /* Job ID */
        0,                       /* Use configured address */
        configData,              /* TX buffer */
        NULL,                    /* No RX for write */
        1                        /* 1 byte to write */
    );
    
    ret = I2C_SetupEB(
        I2C_JOB_READ_TEMP,       /* Job ID */
        0,                       /* Use configured address */
        NULL,                    /* No TX for read */
        temperatureData,         /* RX buffer */
        2                        /* 2 bytes to read */
    );
    
    /* 3. Start asynchronous transmission */
    ret = I2C_AsyncTransmit(I2C_SEQ_READ_SENSOR);
    
    /* 4. Do other work while I2C transfers... */
    
    /* 5. Check sequence result */
    seqResult = I2C_GetSequenceResult(I2C_SEQ_READ_SENSOR);
    
    if (seqResult == I2C_SEQUENCE_OK)
    {
        /* Process temperature data */
        uint16 temp = (temperatureData[0] << 8) | temperatureData[1];
    }
    
    /* 6. Deinitialize before shutdown */
    I2C_DeInit();
    
    return 0;
}