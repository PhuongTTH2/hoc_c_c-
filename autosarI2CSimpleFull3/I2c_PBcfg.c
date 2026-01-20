/******************************************************************************
* AUTOSAR I2C Driver - Post-Build Configuration
* File: I2c_PBcfg.c
* BAT32A2x9 with Simplified I2C and IICA
******************************************************************************/

#include "I2c.h"
#include "I2c_Internal.h"
#include "I2c_Cfg.h"
#include "Det.h"

/* ============ FORWARD DECLARATIONS ============ */
static void I2C_SensorSeqEndNotification(I2C_SequenceType SequenceId, 
                                         I2C_SequenceResultType Result);
static void I2C_EepromSeqEndNotification(I2C_SequenceType SequenceId, 
                                         I2C_SequenceResultType Result);
static void I2C_SlaveSeqEndNotification(I2C_SequenceType SequenceId, 
                                        I2C_SequenceResultType Result);

/* ============ CHANNEL CONFIGURATIONS ============ */

static const I2C_ChannelConfigType I2C_Channels[I2C_MAX_CHANNELS] = {
    /* Channel 0: Simplified I2C (Master Only) - Chapter 19 */
    {
        .channelId = I2C_CHANNEL_SIMPLE_MASTER,
        .hwUnitBaseAddress = 0x4004A000U,  /* IIC00 base address */
        .baudRate = 100000U,                /* 100 kHz standard mode */
        .hwUnitMode = I2C_HW_UNIT_MODE_CONTROLLER,
        .targetListening = 0U               /* Not listening */
    },
    
    /* Channel 1: IICA (Master/Slave) - Chapter 20 */
    {
        .channelId = I2C_CHANNEL_IICA_MASTER_SLAVE,
        .hwUnitBaseAddress = 0x4004B000U,  /* IICA0 base address */
        .baudRate = 400000U,                /* 400 kHz fast mode */
        .hwUnitMode = I2C_HW_UNIT_MODE_TARGET,  /* Can be configured as slave */
        .targetListening = 1U               /* Listening for slave address */
    },
    
    /* Empty slots */
    {0}, {0}, {0}, {0}, {0}, {0}
};

/* ============ JOB CONFIGURATIONS ============ */

static const I2C_JobConfigType I2C_Jobs[I2C_MAX_JOBS] = {
    /* Job 0: Write temperature sensor configuration */
    {
        .jobId = I2C_JOB_WRITE_TEMP_CONFIG,
        .deviceAddress = I2C_ADDR_TEMP_SENSOR
    },
    
    /* Job 1: Read temperature data */
    {
        .jobId = I2C_JOB_READ_TEMP_DATA,
        .deviceAddress = I2C_ADDR_TEMP_SENSOR
    },
    
    /* Job 2: Write EEPROM data */
    {
        .jobId = I2C_JOB_WRITE_EEPROM_DATA,
        .deviceAddress = I2C_ADDR_EEPROM
    },
    
    /* Job 3: Read EEPROM data */
    {
        .jobId = I2C_JOB_READ_EEPROM_DATA,
        .deviceAddress = I2C_ADDR_EEPROM
    },
    
    /* Job 4: Slave receive */
    {
        .jobId = I2C_JOB_SLAVE_RECEIVE,
        .deviceAddress = I2C_SLAVE_ADDRESS
    },
    
    /* Empty slots */
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
    {0}, {0}, {0}, {0}, {0}, {0}
};

/* ============ SEQUENCE CONFIGURATIONS ============ */

static const I2C_SequenceConfigType I2C_Sequences[I2C_MAX_SEQUENCES] = {
    /* Sequence 0: Temperature sensor read (Simplified I2C) */
    {
        .sequenceId = I2C_SEQUENCE_TEMP_SENSOR,
        .assignedChannel = I2C_CHANNEL_SIMPLE_MASTER,
        .numAssignedJobs = 2U,
        .assignedJobs = {I2C_JOB_WRITE_TEMP_CONFIG, I2C_JOB_READ_TEMP_DATA, 
                         0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU},
        .endNotification = I2C_SensorSeqEndNotification
    },
    
    /* Sequence 1: EEPROM write (IICA Master) */
    {
        .sequenceId = I2C_SEQUENCE_EEPROM_WRITE,
        .assignedChannel = I2C_CHANNEL_IICA_MASTER_SLAVE,
        .numAssignedJobs = 1U,
        .assignedJobs = {I2C_JOB_WRITE_EEPROM_DATA, 0xFFU, 0xFFU, 0xFFU, 
                         0xFFU, 0xFFU, 0xFFU, 0xFFU},
        .endNotification = I2C_EepromSeqEndNotification
    },
    
    /* Sequence 2: EEPROM read (IICA Master) */
    {
        .sequenceId = I2C_SEQUENCE_EEPROM_READ,
        .assignedChannel = I2C_CHANNEL_IICA_MASTER_SLAVE,
        .numAssignedJobs = 1U,
        .assignedJobs = {I2C_JOB_READ_EEPROM_DATA, 0xFFU, 0xFFU, 0xFFU,
                         0xFFU, 0xFFU, 0xFFU, 0xFFU},
        .endNotification = I2C_EepromSeqEndNotification
    },
    
    /* Sequence 3: Slave receive (IICA Slave) */
    {
        .sequenceId = I2C_SEQUENCE_SLAVE_RECEIVE,
        .assignedChannel = I2C_CHANNEL_IICA_MASTER_SLAVE,
        .numAssignedJobs = 1U,
        .assignedJobs = {I2C_JOB_SLAVE_RECEIVE, 0xFFU, 0xFFU, 0xFFU,
                         0xFFU, 0xFFU, 0xFFU, 0xFFU},
        .endNotification = I2C_SlaveSeqEndNotification
    },
    
    /* Empty slots */
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}
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

/* ============ CALLBACK FUNCTIONS ============ */

static void I2C_SensorSeqEndNotification(I2C_SequenceType SequenceId, 
                                         I2C_SequenceResultType Result)
{
    /* Application handling for temperature sensor sequence */
    if (Result == I2C_SEQUENCE_OK)
    {
        /* Temperature read successful - process data */
        /* Example: Rte_Write_TemperatureData(data); */
    }
    else if (Result == I2C_SEQUENCE_NACK)
    {
        Det_ReportRuntimeError(I2C_MODULE_ID, I2C_INSTANCE_ID,
                              I2C_SERVICE_ID_ASYNCTRANSMIT, I2C_E_NACK_RECEIVED);
    }
    else
    {
        /* Other error handling */
        Dem_SetEventStatus(I2C_EVENT_ID_TEMP_FAIL, DEM_EVENT_STATUS_FAILED);
    }
}

static void I2C_EepromSeqEndNotification(I2C_SequenceType SequenceId, 
                                         I2C_SequenceResultType Result)
{
    /* Application handling for EEPROM sequence */
    if (Result == I2C_SEQUENCE_OK)
    {
        /* EEPROM operation successful */
    }
    else if (Result == I2C_SEQUENCE_NACK)
    {
        Det_ReportRuntimeError(I2C_MODULE_ID, I2C_INSTANCE_ID,
                              I2C_SERVICE_ID_ASYNCTRANSMIT, I2C_E_NACK_RECEIVED);
    }
    else
    {
        /* Other error handling */
        Dem_SetEventStatus(I2C_EVENT_ID_EEPROM_FAIL, DEM_EVENT_STATUS_FAILED);
    }
}

static void I2C_SlaveSeqEndNotification(I2C_SequenceType SequenceId, 
                                        I2C_SequenceResultType Result)
{
    /* Application handling for slave mode sequence */
    if (Result == I2C_SEQUENCE_OK)
    {
        /* Data received successfully in slave mode */
        /* Process received data */
        
        /* Restart listening for next command */
        (void)I2C_StartListening(SequenceId);
    }
}