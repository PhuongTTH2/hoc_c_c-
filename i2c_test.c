/* i2c_test.c
 * Simple test harness for bring-up/diagnostics.
 * - Calls CDD_I2C_Init
 * - Queries port capabilities
 * - Runs timing presets and stores results
 * - Runs example sequence (master) and stores result
 *
 * This harness is intentionally minimal and stores results in globals
 * so you can inspect them via debugger or extend to route via UART.
 */

#include "Std_Types.h"
#include "I2c.h"
#include "CDD_I2C.h"

/* Globals for test inspection */
Std_ReturnType g_i2c_sequence_result = E_NOT_OK;

/* Entry point for test harness; call this from your startup/test harness.
 * Example: call i2c_test_run(20000000u, 0x50); where fclk=20MHz slaveAddr=0x50
 */
void i2c_test_run(uint32 fclk_hz, I2C_AddressType slaveAddr)
{
    /* Init CDD/driver */
    CDD_I2C_Init();

    /* Init and run example sequence (blocking) */
    CDD_I2C_Init();
    g_i2c_sequence_result = CDD_I2C_RunSequence(0u);
}

/* End of i2c_test.c */
