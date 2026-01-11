#include "I2c_PBcfg.h"
#include "I2c_Cfg.h"

/* Channel Configuration */
const I2c_ChannelConfigType I2c_ChannelConfig[] = {
    /* Channel 0 - IICA0 */
    {
        .I2cBaudRate = 100U,
        .I2cHwUnitBaseAddress = 0x40041800U,
        .I2cHwUnitMode = I2C_HW_UNIT_MODE_CONTROLLER,
        .I2cTargetListening = FALSE
    },
    /* Channel 1 - IICA1 */
    {
        .I2cBaudRate = 400U,
        .I2cHwUnitBaseAddress = 0x40046000U,
        .I2cHwUnitMode = I2C_HW_UNIT_MODE_CONTROLLER,
        .I2cTargetListening = FALSE
    }
};

/* Job Configuration */
const I2c_JobConfigType I2c_JobConfig[] = {
    /* Job 0: EEPROM Write */
    {
        .I2cDeviceAddress = 0x50U,
        .I2cJobId = I2C_JOB_EEPROM_WRITE
    },
    /* Job 1: EEPROM Read */
    {
        .I2cDeviceAddress = 0x50U,
        .I2cJobId = I2C_JOB_EEPROM_READ
    },
    /* Job 2: Sensor Write */
    {
        .I2cDeviceAddress = 0x48U,
        .I2cJobId = I2C_JOB_SENSOR_WRITE
    },
    /* Job 3: Sensor Read */
    {
        .I2cDeviceAddress = 0x48U,
        .I2cJobId = I2C_JOB_SENSOR_READ
    }
};

/* Sequence Configuration */
const I2c_SequenceConfigType I2c_SequenceConfig[] = {
    /* Sequence 0: EEPROM Access */
    {
        .I2cEndNotification = NULL,
        .I2cSequenceId = I2C_SEQ_EEPROM_ACCESS,
        .I2cAssignedChannel = &I2c_ChannelConfig[I2C_CHANNEL_0],
        .I2cAssignedJob = &I2c_JobConfig[0],
        .NumAssignedJobs = 2
    },
    /* Sequence 1: Sensor Access */
    {
        .I2cEndNotification = NULL,
        .I2cSequenceId = I2C_SEQ_SENSOR_ACCESS,
        .I2cAssignedChannel = &I2c_ChannelConfig[I2C_CHANNEL_1],
        .I2cAssignedJob = &I2c_JobConfig[2],
        .NumAssignedJobs = 2
    }
};

/* Global Configuration */
const I2c_ConfigType I2c_Config = {
    .I2cGeneral = {
        .I2cDevErrorDetect = I2C_DEV_ERROR_DETECT,
        .I2cVersionInfoApi = I2C_VERSION_INFO_API
    },
    .I2cChannelConfig = I2c_ChannelConfig,
    .I2cJobConfig = I2c_JobConfig,
    .I2cSequenceConfig = I2c_SequenceConfig,
    .NumChannels = I2C_NUM_CHANNELS,
    .NumJobs = I2C_NUM_JOBS,
    .NumSequences = I2C_NUM_SEQUENCES
};