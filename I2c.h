
#ifndef I2C_H
#define I2C_H

#include "Std_Types.h"

#define I2C_MODULE_ID 0x5A
#define I2C_INSTANCE_ID 0

/* Basic typedefs used by the driver */
typedef uint8 I2C_JobType;         /* index/ID of job */
typedef uint8 I2C_SequenceType;    /* index/ID of sequence */
typedef uint8 I2C_AddressType;     /* 7-bit address (or extended when >0x7F) */
typedef uint32 I2C_NumberOfDataType;

typedef uint8* I2C_DataPtrType;
typedef const uint8* I2C_DataConstPtrType;

typedef enum {
    I2C_SEQ_OK,
    I2C_SEQ_PENDING,
    I2C_SEQ_QUEUED,
    I2C_SEQ_FAILED,
    I2C_SEQ_NACK
} I2C_SequenceResultType;

/* HW unit mode */
typedef enum {
    I2C_HW_UNIT_MODE_CONTROLLER = 0,
    I2C_HW_UNIT_MODE_TARGET
} I2C_HwUnitModeType;

/* Job direction */
typedef enum {
    I2C_DIR_WRITE = 0,
    I2C_DIR_READ,
    I2C_DIR_SLAVE
} I2C_JobDirectionType;

/* Channel (hardware unit) configuration */
typedef struct {
    uint32 BaseAddress;           /* peripheral base address */
    uint32 BaudRate;              /* bus baudrate */
    I2C_HwUnitModeType Mode;      /* controller/target */
    boolean TargetListening;      /* slave always-listening */
    I2C_AddressType OwnAddress;   /* own SVA for slave mode */
    uint32 PeripheralClockHz;     /* clock supplied to the I2C peripheral (used to compute WL/WH) */
    boolean UseAltPins;           /* select alternate pin group (e.g. P14/P15) when TRUE */
} I2C_ChannelCfgType;

/* Job configuration */
typedef struct {
    I2C_JobType JobId;
    I2C_AddressType DeviceAddress; /* target device address (controller jobs) */
    I2C_JobDirectionType Direction;
} I2C_JobCfgType;

/* Sequence configuration */
typedef void (*I2C_EndNotificationType)(void);

typedef struct {
    I2C_SequenceType SequenceId;
    const I2C_ChannelCfgType* Channel; /* reference to channel */
    const I2C_JobCfgType** JobList;    /* array of pointers to jobs */
    uint8 JobCount;
    I2C_EndNotificationType EndNotification;
} I2C_SequenceCfgType;

/* Root configuration container (Ecuc style) */
typedef struct {
    const I2C_ChannelCfgType* Channels;
    uint8 ChannelCount;

    const I2C_JobCfgType* Jobs;
    uint8 JobCount;

    const I2C_SequenceCfgType* Sequences;
    uint8 SequenceCount;
} I2C_ConfigType;

/* Public API matching AUTOSAR-like SWS (EB-mode) */
void I2C_Init(const I2C_ConfigType* ConfigPtr);
void I2C_DeInit(void);

/* Setup exclusive buffer job runtime data (TX or RX). Buffers are
 * plain pointers to the caller buffer. */
Std_ReturnType I2C_SetupEB(I2C_JobType JobId,
                           I2C_AddressType NodeAddress,
                           I2C_DataConstPtrType TxDataBufferPtr,
                           I2C_DataPtrType RxDataBufferPtr,
                           I2C_NumberOfDataType Length);

/* Blocking sequence execution (runs all jobs in sequence). */
Std_ReturnType I2C_SyncTransmit(I2C_SequenceType SequenceId);

/* Non-blocking: arm the sequence for background processing; the
 * application must call I2C_MainFunction periodically (e.g., from
 * a scheduler) to progress the transfer. */
Std_ReturnType I2C_AsyncTransmit(I2C_SequenceType SequenceId);
void I2C_MainFunction(void);

/* Start listening (slave mode) for the given sequence (which references
 * a channel configured as target). */
Std_ReturnType I2C_StartListening(I2C_SequenceType SequenceId);

I2C_SequenceResultType I2C_GetSequenceResult(I2C_SequenceType SequenceId);
void I2C_GetVersionInfo(Std_VersionInfoType* VersionInfo);

#endif
