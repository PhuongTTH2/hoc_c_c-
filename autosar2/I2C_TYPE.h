#ifndef I2C_TYPES_H
#define I2C_TYPES_H

#include "Std_Types.h"
#include "Rte_Dem_Type.h"  /* [CP_SWS_I2C_00833] */

/* ========================== AUTOSAR API TYPES ========================== */
typedef uint16 I2C_AddressType;      /* [CP_SWS_I2C_00803] */
typedef uint8  I2C_DataType;         /* [CP_SWS_I2C_00804] */
typedef uint8* I2C_DataPtrType;      /* [CP_SWS_I2C_00805] */
typedef const uint8* I2C_DataConstPtrType; /* [CP_SWS_I2C_00806] */
typedef uint16 I2C_NumberOfDataType; /* [CP_SWS_I2C_00811] */
typedef uint8  I2C_HwUnitType;       /* [CP_SWS_I2C_00808] */
typedef uint8  I2C_JobType;          /* [CP_SWS_I2C_00809] */
typedef uint8  I2C_SequenceType;     /* [CP_SWS_I2C_00810] */

/* ========================== SEQUENCE RESULT ========================== */
/* [CP_SWS_I2C_00807] */
typedef enum
{
    I2C_SEQ_OK = 0x00,
    I2C_SEQ_PENDING = 0x01,
    I2C_SEQ_QUEUED = 0x02,
    I2C_SEQ_NACK = 0x03,
    I2C_SEQ_FAILED = 0x04
} I2C_SequenceResultType;

/* ========================== ERROR TYPES ========================== */
/* [CP_SWS_I2C_00700] - Development Errors */
typedef enum
{
    I2C_E_PARAM_JOB = 0x00,          /* API service called with wrong parameter */
    I2C_E_PARAM_SEQUENCE = 0x01,     /* API service called with wrong parameter */
    I2C_E_PARAM_POINTER = 0x02,      /* API service called with unexpected pointer */
    I2C_E_UNINIT = 0x03,             /* API service used without module initialization */
    I2C_E_WRONG_CONDITION = 0x04,    /* API is called under wrong condition */
    
    /* [CP_SWS_I2C_00701] - Runtime Errors */
    I2C_E_NACK_RECEIVED = 0x00,      /* NACK was received */
    I2C_E_ARBITRATION_FAILURE = 0x01, /* Master loses arbitration */
    I2C_E_FIFO_HANDLING = 0x02,      /* FIFO overflow error */
    I2C_E_BUS_FAILURE = 0x03,        /* SCL line is stuck low */
    I2C_E_WRONG_MODE = 0x04          /* Wrong mode for I2C_StartListening */
} I2C_ErrorType;

/* ========================== CONFIGURATION STRUCTURES ========================== */

/* [CP_SWS_I2C_00801] - I2C_ConfigType */
typedef struct
{
    /* Implementation specific - contents are I2C specific */
    /* This structure will be defined in implementation */
} I2C_ConfigType;

/* Sequence notification callback type */
typedef void (*I2C_SeqEndNotificationType)(I2C_SequenceType SequenceId, 
                                          I2C_SequenceResultType Result);

#endif /* I2C_TYPES_H */