/******************************************************************************
* File: I2c_Cfg.c
* Description: File cấu hình I2C Driver
******************************************************************************/

#include "I2c.h"
#include "I2c_Cfg.h"

/* Cấu hình I2C Channels */
const I2C_ConfigType I2C_ConfigSet[I2C_NUM_OF_CHANNELS] = 
{
    /* Channel 0: I2C0, Controller mode, 100kHz */
    {
        .HwUnitBaseAddress = I2C0_BASE_ADDRESS,
        .BaudRate = I2C_STANDARD_MODE, /* 100 kHz */
        .HwUnitMode = I2C_HW_UNIT_MODE_CONTROLLER,
        .TargetListening = FALSE
    },
    
    /* Channel 1: I2C1, Target mode, 400kHz */
    {
        .HwUnitBaseAddress = I2C1_BASE_ADDRESS,
        .BaudRate = I2C_FAST_MODE, /* 400 kHz */
        .HwUnitMode = I2C_HW_UNIT_MODE_TARGET,
        .TargetListening = TRUE
    }
};

/* Cấu hình I2C Jobs */
const I2C_JobConfigType I2C_JobConfig[I2C_NUM_OF_JOBS] = 
{
    /* Job 0: Read from EEPROM */
    {
        .JobId = 0,
        .DeviceAddress = 0x50, /* EEPROM address */
        .AssignedChannel = 0    /* Use I2C0 */
    },
    
    /* Job 1: Write to sensor */
    {
        .JobId = 1,
        .DeviceAddress = 0x76, /* Sensor address */
        .AssignedChannel = 0    /* Use I2C0 */
    }
    
    /* Thêm các Job khác tại đây... */
};

/* Cấu hình I2C Sequences */
const I2C_SequenceConfigType I2C_SequenceConfig[I2C_NUM_OF_SEQUENCES] = 
{
    /* Sequence 0: Read EEPROM then write to sensor */
    {
        .SequenceId = 0,
        .AssignedChannel = 0,
        .NumJobs = 2,
        .AssignedJobs = {0, 1}, /* Job 0, Job 1 */
        .EndNotification = &I2C_SeqEndNotification0
    }
    
    /* Thêm các Sequence khác tại đây... */
};



/* Mapping Jobs cho Sequences (0xFF = không sử dụng) */
const uint8 I2C_SequenceJobMapping[I2C_NUM_OF_SEQUENCES][I2C_MAX_JOBS_PER_SEQUENCE] = 
{
    /* Sequence 0: 2 Jobs */
    {0, 1, 0xFF, 0xFF, 0xFF},
    
    /* Sequence 1: 1 Job */
    {2, 0xFF, 0xFF, 0xFF, 0xFF},
    
    /* Sequence 2: 3 Jobs */
    {3, 4, 5, 0xFF, 0xFF},
    
    /* Sequence 3: 2 Jobs */
    {6, 7, 0xFF, 0xFF, 0xFF},
    
    /* Sequence 4: 1 Job */
    {8, 0xFF, 0xFF, 0xFF, 0xFF}
};

/* Channel assignment cho mỗi Sequence */
const uint8 I2C_SequenceChannelMapping[I2C_NUM_OF_SEQUENCES] = 
{
    0, /* Sequence 0 -> Channel 0 */
    0, /* Sequence 1 -> Channel 0 */
    1, /* Sequence 2 -> Channel 1 */
    0, /* Sequence 3 -> Channel 0 */
    1  /* Sequence 4 -> Channel 1 */
};

/* Callback functions cho mỗi Sequence */
I2C_SeqEndNotificationType I2C_SeqEndNotificationFuncs[I2C_NUM_OF_SEQUENCES] = 
{
    NULL_PTR, /* Sequence 0 */
    NULL_PTR, /* Sequence 1 */
    NULL_PTR, /* Sequence 2 */
    NULL_PTR, /* Sequence 3 */
    NULL_PTR  /* Sequence 4 */
};