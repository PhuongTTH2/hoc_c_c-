#ifndef I2C_MCAL_H
#define I2C_MCAL_H

/* ========================== Includes ========================== */
#include "Std_Types.h"
#include "I2C_Types.h"
#include "Det.h"

/* ========================== Module Information ========================== */
#define I2C_MODULE_ID              1101    /* From AUTOSAR spec */
#define I2C_VENDOR_ID              1
#define I2C_INSTANCE_ID            0

/* ========================== API Functions ========================== */
/* [CP_SWS_I2C_00820] */
void I2C_Init(const I2C_ConfigType* ConfigPtr);

/* [CP_SWS_I2C_00821] */
void I2C_DeInit(void);

/* [CP_SWS_I2C_00822] */
Std_ReturnType I2C_SetupEB(
    I2C_JobType JobId,
    I2C_AddressType NodeAddress,
    I2C_DataConstPtrType TxDataBufferPtr,
    I2C_DataPtrType RxDataBufferPtr,
    I2C_NumberOfDataType Length
);

/* [CP_SWS_I2C_00823] */
Std_ReturnType I2C_AsyncTransmit(I2C_SequenceType SequenceId);

/* [CP_SWS_I2C_00824] */
Std_ReturnType I2C_SyncTransmit(I2C_SequenceType SequenceId);

/* [CP_SWS_I2C_00827] */
void I2C_GetVersionInfo(Std_VersionInfoType* VersionInfo);

/* [CP_SWS_I2C_00828] */
I2C_SequenceResultType I2C_GetSequenceResult(I2C_SequenceType SequenceId);

/* [CP_SWS_I2C_00835] */
Std_ReturnType I2C_StartListening(I2C_SequenceType SequenceId);

/* [CP_SWS_I2C_00834] */
void I2C_MainFunction(void);

#endif /* I2C_MCAL_H */