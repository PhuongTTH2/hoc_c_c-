/******************************************************************************
* AUTOSAR I2C Driver - Internal Types
* File: I2c_Internal.h
******************************************************************************/

#ifndef I2C_INTERNAL_H
#define I2C_INTERNAL_H

#include "I2c.h"

/* Maximum limits */
#define I2C_MAX_CHANNELS     8U
#define I2C_MAX_JOBS         32U
#define I2C_MAX_SEQUENCES    16U
#define I2C_MAX_JOBS_PER_SEQ 8U

/* Hardware Unit Modes */
typedef enum {
    I2C_HW_UNIT_MODE_CONTROLLER = 0,
    I2C_HW_UNIT_MODE_TARGET
} I2C_HwUnitModeType;

/* ============ CONFIGURATION STRUCTURES ============ */

/* Channel Configuration */
typedef struct {
    uint8 channelId;
    uint32 hwUnitBaseAddress;
    uint32 baudRate;  /* in kbit/s */
    I2C_HwUnitModeType hwUnitMode;
    uint8 targetListening;  /* TRUE for always listening */
} I2C_ChannelConfigType;

/* Job Configuration */
typedef struct {
    I2C_JobType jobId;
    uint16 deviceAddress;  /* Target address */
} I2C_JobConfigType;

/* Sequence Configuration */
typedef struct {
    I2C_SequenceType sequenceId;
    uint8 assignedChannel;
    uint8 numAssignedJobs;
    uint8 assignedJobs[I2C_MAX_JOBS_PER_SEQ];
    void (*endNotification)(I2C_SequenceType, I2C_SequenceResultType);
} I2C_SequenceConfigType;

/* General Configuration */
typedef struct {
    uint8 devErrorDetect;     /* TRUE to enable Dev Error Detection */
    uint8 versionInfoApi;     /* TRUE to enable version info API */
} I2C_GeneralConfigType;

/* Main Configuration Structure (I2C_ConfigType) */
struct I2C_ConfigTypeTag {
    I2C_GeneralConfigType generalConfig;
    uint8 numChannels;
    uint8 numJobs;
    uint8 numSequences;
    I2C_ChannelConfigType channels[I2C_MAX_CHANNELS];
    I2C_JobConfigType jobs[I2C_MAX_JOBS];
    I2C_SequenceConfigType sequences[I2C_MAX_SEQUENCES];
};

/* ============ INTERNAL FUNCTIONS ============ */

/* Get channel ID for a sequence */
static inline uint8 GetSequenceChannel(I2C_SequenceType seqId)
{
    /* This would lookup from configuration */
    return 0; /* Simplified */
}

#endif /* I2C_INTERNAL_H */