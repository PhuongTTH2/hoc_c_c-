/******************************************************************************
* AUTOSAR I2C Driver - External Interfaces
* File: I2c_External.h
******************************************************************************/

#ifndef I2C_EXTERNAL_H
#define I2C_EXTERNAL_H

#include "I2c.h"

/* 8.6.3.1 I2C_SeqEndNotification - Configurable callback interface */
typedef void (*I2C_SeqEndNotificationType)(
    I2C_SequenceType SequenceId,
    I2C_SequenceResultType Result
);

/* Note: The actual callback functions are defined in the application,
   not in the driver itself. The driver just calls them via function pointers
   configured in I2cSequence containers.
*/

#endif /* I2C_EXTERNAL_H */