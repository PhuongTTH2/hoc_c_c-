/******************************************************************************
* AUTOSAR I2C Driver - Internal Types
* File: I2c_Internal.h
******************************************************************************/

#ifndef I2C_INTERNAL_H
#define I2C_INTERNAL_H

#include "I2c.h"

/* Maximum limits */
#define I2C_MAX_CHANNELS               8U
#define I2C_MAX_JOBS                   32U
#define I2C_MAX_SEQUENCES              16U
#define I2C_MAX_JOBS_PER_SEQ           8U

/* Hardware Unit Modes */
typedef enum {
    I2C_HW_UNIT_MODE_CONTROLLER = 0U,
    I2C_HW_UNIT_MODE_TARGET
} I2C_HwUnitModeType;

/* I2C Transfer Directions */
typedef enum {
    I2C_TRANSFER_WRITE = 0U,
    I2C_TRANSFER_READ = 1U
} I2C_TransferDirectionType;

/* ============ INTERNAL STRUCTURES ============ */

/* Channel Configuration */
typedef struct {
    uint8 channelId;
    uint32 hwUnitBaseAddress;
    uint32 baudRate;
    I2C_HwUnitModeType hwUnitMode;
    uint8 targetListening;
} I2C_ChannelConfigType;

/* Job Configuration */
typedef struct {
    I2C_JobType jobId;
    uint16 deviceAddress;
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
    uint8 devErrorDetect;
    uint8 versionInfoApi;
} I2C_GeneralConfigType;

/* Main Configuration Structure */
struct I2C_ConfigTypeTag {
    I2C_GeneralConfigType generalConfig;
    uint8 numChannels;
    uint8 numJobs;
    uint8 numSequences;
    I2C_ChannelConfigType channels[I2C_MAX_CHANNELS];
    I2C_JobConfigType jobs[I2C_MAX_JOBS];
    I2C_SequenceConfigType sequences[I2C_MAX_SEQUENCES];
};

#endif /* I2C_INTERNAL_H */