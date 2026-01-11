/* CDD_I2C.h - header for the CDD wrapper around I2C driver
 * Exposes init and sequence runner used by the test harness.
 */

#ifndef CDD_I2C_H
#define CDD_I2C_H

#include "I2c.h"

void CDD_I2C_Init(void);
Std_ReturnType CDD_I2C_RunSequence(I2C_SequenceType seqId);
const I2C_SequenceCfgType* CDD_I2C_GetSequenceCfg(I2C_SequenceType seqId);

#endif /* CDD_I2C_H */
