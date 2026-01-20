/******************************************************************************
* AUTOSAR I2C Driver - Post-Build Configuration
* File: I2c_PBcfg.c
******************************************************************************/

#include "I2c.h"
#include "I2c_Cfg.h"

/* Forward declaration of callback functions */
static void I2C_SensorSeqEndNotification(I2C_SequenceType SequenceId, 
                                         I2C_SequenceResultType Result);
static void I2C_EepromSeqEndNotification(I2C_SequenceType SequenceId, 
                                         I2C_SequenceResultType Result);

/* ============ CHANNEL CONFIGURATIONS ============ */

static const I2C_ChannelConfigType I2C_Channels[I2C_NUM_CHANNELS] = {
    /* Channel 0 - Controller */
    {
        .channelId = 0,
        .hwUnitBaseAddress = I2C_CH0_HW_BASE_ADDR,
        .baudRate = I2C_CH0_BAUD_RATE,
        .hwUnitMode = I2C_CH0_MODE,
        .targetListening = I2C_CH0_TARGET_LISTENING
    },
    
    /* Channel 1 - Target */
    {
        .channelId = 1,
        .hwUnitBaseAddress = I2C_CH1_HW_BASE_ADDR,
        .baudRate = I2C_CH1_BAUD_RATE,
        .hwUnitMode = I2C_CH1_MODE,
        .targetListening = I2C_CH1_TARGET_LISTENING
    }
};

/* ============ JOB CONFIGURATIONS ============ */

static const I2C_JobConfigType I2C_Jobs[I2C_NUM_JOBS] = {
    /* Job 0: Read temperature sensor */
    {
        .jobId = I2C_JOB_READ_TEMP,
        .deviceAddress = I2C_DEV_ADDR_TEMP_SENSOR
    },
    
    /* Job 1: Write configuration */
    {
        .jobId = I2C_JOB_WRITE_CONFIG,
        .deviceAddress = I2C_DEV_ADDR_TEMP_SENSOR
    },
    
    /* Job 2: Read status */
    {
        .jobId = I2C_JOB_READ_STATUS,
        .deviceAddress = I2C_DEV_ADDR_EEPROM
    },
    
    /* Job 3: Write data to EEPROM */
    {
        .jobId = I2C_JOB_WRITE_DATA,
        .deviceAddress = I2C_DEV_ADDR_EEPROM
    }
};

/* ============ SEQUENCE CONFIGURATIONS ============ */

static const I2C_SequenceConfigType I2C_Sequences[I2C_NUM_SEQUENCES] = {
    /* Sequence 0: Read sensor (multiple jobs) */
    {
        .sequenceId = I2C_SEQ_READ_SENSOR,
        .assignedChannel = 0,  /* Use channel 0 */
        .numAssignedJobs = 2,
        .assignedJobs = {I2C_JOB_WRITE_CONFIG, I2C_JOB_READ_TEMP},
        .endNotification = I2C_SensorSeqEndNotification
    },
    
    /* Sequence 1: Write EEPROM */
    {
        .sequenceId = I2C_SEQ_WRITE_EEPROM,
        .assignedChannel = 0,  /* Use channel 0 */
        .numAssignedJobs = 1,
        .assignedJobs = {I2C_JOB_WRITE_DATA},
        .endNotification = I2C_EepromSeqEndNotification
    }
};

/* ============ GENERAL CONFIGURATION ============ */

static const I2C_GeneralConfigType I2C_GeneralConfig = {
    .devErrorDetect = I2C_DEV_ERROR_DETECT,
    .versionInfoApi = I2C_VERSION_INFO_API
};

/* ============ MAIN CONFIGURATION STRUCTURE ============ */

const I2C_ConfigType I2C_Config = {
    .generalConfig = I2C_GeneralConfig,
    .numChannels = I2C_NUM_CHANNELS,
    .numJobs = I2C_NUM_JOBS,
    .numSequences = I2C_NUM_SEQUENCES,
    .channels = I2C_Channels,
    .jobs = I2C_Jobs,
    .sequences = I2C_Sequences
};

/* ============ CALLBACK FUNCTION DEFINITIONS ============ */

static void I2C_SensorSeqEndNotification(I2C_SequenceType SequenceId, 
                                         I2C_SequenceResultType Result)
{
    /* Application-specific handling for sensor sequence completion */
    if (Result == I2C_SEQUENCE_OK)
    {
        /* Process sensor data */
    }
    else
    {
        /* Handle error */
    }
}

static void I2C_EepromSeqEndNotification(I2C_SequenceType SequenceId, 
                                         I2C_SequenceResultType Result)
{
    /* Application-specific handling for EEPROM sequence completion */
    if (Result == I2C_SEQUENCE_OK)
    {
        /* Confirm EEPROM write success */
    }
    else
    {
        /* Retry or error handling */
    }
}