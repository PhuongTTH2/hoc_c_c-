/******************************************************************************
* AUTOSAR I2C Driver - Specification R24-11
* File: I2c.h
* Version: 1.0.0
******************************************************************************/

#ifndef I2C_H
#define I2C_H

/* Include AUTOSAR standard types */
#include "Std_Types.h"
#include "Dem.h"
#include "Det.h"

/* AUTOSAR checking */
#if (!defined(STD_TYPES_AR_RELEASE_MAJOR_VERSION) || (STD_TYPES_AR_RELEASE_MAJOR_VERSION != 4))
#error "AUTOSAR version mismatch - Std_Types.h"
#endif

/* Module Version Information */
#define I2C_SW_MAJOR_VERSION          1U
#define I2C_SW_MINOR_VERSION          0U
#define I2C_SW_PATCH_VERSION          0U

/* Module ID */
#define I2C_MODULE_ID                 110U
#define I2C_VENDOR_ID                 1U   /* Example vendor ID */

/* Instance ID - always 0 for this module */
#define I2C_INSTANCE_ID               0U

/* Service IDs */
#define I2C_INIT_SID                  0x00U
#define I2C_DEINIT_SID                0x01U
#define I2C_SETUPEB_SID               0x02U
#define I2C_ASYNCTRANSMIT_SID         0x03U
#define I2C_SYNCTRANSMIT_SID          0x04U
#define I2C_GETVERSIONINFO_SID        0x07U
#define I2C_GETSEQUENCERESULT_SID     0x09U
#define I2C_STARTLISTENING_SID        0x0AU
#define I2C_MAINFUNCTION_SID          0x10U

/* Development Error Codes */
#define I2C_E_PARAM_JOB               0x00U
#define I2C_E_PARAM_SEQUENCE          0x01U
#define I2C_E_PARAM_POINTER           0x02U
#define I2C_E_UNINIT                  0x03U
#define I2C_E_WRONG_CONDITION         0x04U

/* Runtime Error Codes */
#define I2C_E_NACK_RECEIVED           0x00U
#define I2C_E_ARBITRATION_FAILURE     0x01U
#define I2C_E_FIFO_HANDLING           0x02U
#define I2C_E_BUS_FAILURE             0x03U
#define I2C_E_WRONG_MODE              0x04U

/* Sequence Result States */
#define I2C_SEQ_OK                    0x00U
#define I2C_SEQ_PENDING               0x01U
#define I2C_SEQ_QUEUED                0x02U
#define I2C_SEQ_NACK                  0x03U
#define I2C_SEQ_FAILED                0x04U

/* API Service IDs for Det */
#define I2C_SERVICE_ID_INIT           0x00U
#define I2C_SERVICE_ID_DEINIT         0x01U
#define I2C_SERVICE_ID_SETUPEB        0x02U
#define I2C_SERVICE_ID_ASYNCTRANSMIT  0x03U
#define I2C_SERVICE_ID_SYNCTRANSMIT   0x04U
#define I2C_SERVICE_ID_GETVERSIONINFO 0x07U
#define I2C_SERVICE_ID_GETSEQUENCERESULT 0x09U
#define I2C_SERVICE_ID_STARTLISTENING 0x0AU

/* ============ TYPE DEFINITIONS ============ */

/* Imported types */
#include "Dem_Types.h"

/* Forward declaration of configuration structure */
typedef struct I2C_ConfigTypeTag I2C_ConfigType;

/* 8.2.1 I2C_AddressType */
typedef uint16 I2C_AddressType;

/* 8.2.2 I2C_DataType */
typedef uint8 I2C_DataType;

/* 8.2.3 I2C_DataPtrType */
typedef uint8* I2C_DataPtrType;

/* 8.2.4 I2C_DataConstPtrType */
typedef const uint8* I2C_DataConstPtrType;

/* 8.2.5 I2C_SequenceResultType */
typedef enum {
    I2C_SEQUENCE_OK = 0x00U,        /* Transmission finished successfully */
    I2C_SEQUENCE_PENDING = 0x01U,   /* Sequence is being processed */
    I2C_SEQUENCE_QUEUED = 0x02U,    /* Sequence is queued */
    I2C_SEQUENCE_NACK = 0x03U,      /* NACK received */
    I2C_SEQUENCE_FAILED = 0x04U     /* Transmission failed */
} I2C_SequenceResultType;

/* 8.2.6 I2C_HwUnitType */
typedef uint8 I2C_HwUnitType;

/* 8.2.7 I2C_JobType */
typedef uint8 I2C_JobType;

/* 8.2.8 I2C_SequenceType */
typedef uint8 I2C_SequenceType;

/* 8.2.9 I2C_NumberOfDataType */
typedef uint16 I2C_NumberOfDataType;

/* ============ API FUNCTION PROTOTYPES ============ */

/* 8.3.1 I2C_Init */
void I2C_Init(const I2C_ConfigType* ConfigPtr);

/* 8.3.2 I2C_DeInit */
void I2C_DeInit(void);

/* 8.3.3 I2C_SetupEB */
Std_ReturnType I2C_SetupEB(
    I2C_JobType JobId,
    I2C_AddressType NodeAddress,
    I2C_DataConstPtrType TxDataBufferPtr,
    I2C_DataPtrType RxDataBufferPtr,
    I2C_NumberOfDataType Length
);

/* 8.3.4 I2C_AsyncTransmit */
Std_ReturnType I2C_AsyncTransmit(I2C_SequenceType SequenceId);

/* 8.3.5 I2C_SyncTransmit */
Std_ReturnType I2C_SyncTransmit(I2C_SequenceType SequenceId);

/* 8.3.6 I2C_GetVersionInfo */
void I2C_GetVersionInfo(Std_VersionInfoType* versioninfo);

/* 8.3.7 I2C_GetSequenceResult */
I2C_SequenceResultType I2C_GetSequenceResult(I2C_SequenceType SequenceId);

/* 8.3.8 I2C_StartListening */
Std_ReturnType I2C_StartListening(I2C_SequenceType SequenceId);

/* 8.5.1 I2C_MainFunction */
void I2C_MainFunction(void);

#endif /* I2C_H */