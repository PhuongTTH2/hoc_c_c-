#include "I2C_Cfg.h"

/* ========================== I2cGeneral ========================== */
static const I2cGeneral_Type I2cGeneral = 
{
    .I2cDevErrorDetect = I2C_DEV_ERROR_DETECT,
    .I2cVersionInfoApi = I2C_VERSION_INFO_API,
};

/* ========================== I2cChannel ========================== */
static const I2cChannel_Type I2cChannels[I2C_NUMBER_OF_CHANNELS] = 
{
    /* Channel 0: I2C0 Controller */
    {
        .I2cBaudRate = I2C_CHANNEL_0_BAUDRATE,
        .I2cHwUnitBaseAddress = I2C_CHANNEL_0_BASE_ADDRESS,
        .I2cHwUnitMode = I2C_CHANNEL_0_MODE,
        .I2cTargetListening = I2C_CHANNEL_0_TARGET_LISTEN,
    },
    
    /* Channel 1: I2C1 Target */
    {
        .I2cBaudRate = I2C_CHANNEL_1_BAUDRATE,
        .I2cHwUnitBaseAddress = I2C_CHANNEL_1_BASE_ADDRESS,
        .I2cHwUnitMode = I2C_CHANNEL_1_MODE,
        .I2cTargetListening = I2C_CHANNEL_1_TARGET_LISTEN,
    },
};

/* ========================== I2cJob ========================== */
static const I2cJob_Type I2cJobs[I2C_NUMBER_OF_JOBS] = 
{
    /* Job 0 */
    {
        .I2cDeviceAddress = I2C_JOB_0_DEVICE_ADDRESS,
        .I2cJobId = I2C_JOB_0_ID,
    },
    
    /* Job 1 */
    {
        .I2cDeviceAddress = I2C_JOB_1_DEVICE_ADDRESS,
        .I2cJobId = I2C_JOB_1_ID,
    },
    
    /* Job 2 */
    {
        .I2cDeviceAddress = I2C_JOB_2_DEVICE_ADDRESS,
        .I2cJobId = I2C_JOB_2_ID,
    },
};

/* ========================== I2cSequence ========================== */
static const I2cSequence_Type I2cSequences[I2C_NUMBER_OF_SEQUENCES] = 
{
    /* Sequence 0 */
    {
        .I2cEndNotification = I2C_SEQUENCE_0_END_NOTIFICATION,
        .I2cSequenceId = I2C_SEQUENCE_0_ID,
        .I2cAssignedChannel = I2C_SEQUENCE_0_CHANNEL,
        .I2cAssignedJob = I2C_SEQUENCE_0_JOBS,
    },
    
    /* Sequence 1 */
    {
        .I2cEndNotification = I2C_SEQUENCE_1_END_NOTIFICATION,
        .I2cSequenceId = I2C_SEQUENCE_1_ID,
        .I2cAssignedChannel = I2C_SEQUENCE_1_CHANNEL,
        .I2cAssignedJob = I2C_SEQUENCE_1_JOBS,
    },
};

/* ========================== I2cConfigSet ========================== */
static const I2cConfigSet_Type I2cConfigSet = 
{
    .I2cChannelCount = I2C_NUMBER_OF_CHANNELS,
    .I2cChannel = I2cChannels,
    .I2cJobCount = I2C_NUMBER_OF_JOBS,
    .I2cJob = I2cJobs,
    .I2cSequenceCount = I2C_NUMBER_OF_SEQUENCES,
    .I2cSequence = I2cSequences,
};

/* ========================== Main Configuration ========================== */
const I2C_ConfigType I2c_Config = 
{
    .I2cGeneral = &I2cGeneral,
    .I2cConfigSetCount = 1,
    .I2cConfigSet = &I2cConfigSet,
};

/* ========================== Notification Functions ========================== */
void EEPROM_SequenceComplete(I2C_SequenceType SequenceId, I2C_SequenceResultType Result)
{
    /* User implementation */
}

void Sensor_SequenceComplete(I2C_SequenceType SequenceId, I2C_SequenceResultType Result)
{
    /* User implementation */
}