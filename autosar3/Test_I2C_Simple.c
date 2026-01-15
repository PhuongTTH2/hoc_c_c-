/******************************************************************************
* File: Test_I2C_Simple.c
* Description: Test đơn giản cho I2C Driver - Chỉ test chức năng cơ bản
******************************************************************************/

#include "I2c.h"
#include "I2c_Test.h"
#include "I2c_Mock.h"
#include <stdio.h>
#include <string.h>

/* Test Data Buffers */
static uint8 TxBuffer_Simple[16];
static uint8 RxBuffer_Simple[16];

/******************************************************************************
* Test Group 1: Basic API Functions
******************************************************************************/
uint8 Test_Simple_I2C_BasicAPI(void)
{
    printf("\n=== TEST GROUP 1: BASIC API FUNCTIONS ===\n");
    
    /* Test 1.1: Initialization */
    printf("Test 1.1: I2C_Init()\n");
    I2C_ConfigType config = {
        .HwUnitBaseAddress = 0x400A0000,
        .BaudRate = 100000,
        .HwUnitMode = I2C_HW_UNIT_MODE_CONTROLLER,
        .TargetListening = FALSE
    };
    
    I2C_Init(&config);
    
    /* Test 1.2: GetVersionInfo */
    printf("Test 1.3: I2C_GetVersionInfo()\n");
    Std_VersionInfoType version;
    I2C_GetVersionInfo(&version);
    
    TEST_ASSERT_EQUAL(1101U, version.moduleID, "Module ID should be 1101");
    TEST_ASSERT_EQUAL(24U, version.sw_major_version, "Major version should be 24");
    TEST_ASSERT_EQUAL(11U, version.sw_minor_version, "Minor version should be 11");
    
    printf("Test Group 1: PASS ✓\n");
    return TEST_PASS;
}

/******************************************************************************
* Test Group 2: SetupEB Function
******************************************************************************/
uint8 Test_Simple_I2C_SetupEB(void)
{
    printf("\n=== TEST GROUP 2: SETUPEB FUNCTION ===\n");
    
    /* Initialize test data */
    memset(TxBuffer_Simple, 0xAA, sizeof(TxBuffer_Simple));
    memset(RxBuffer_Simple, 0x00, sizeof(RxBuffer_Simple));
    
    /* Test 2.1: Setup write operation */
    printf("Test 2.1: Setup write operation\n");
    Std_ReturnType ret = I2C_SetupEB(
        0,          /* JobId 0 */
        0x50,       /* EEPROM address */
        TxBuffer_Simple,
        NULL_PTR,   /* No read */
        8           /* 8 bytes */
    );
    TEST_ASSERT_EQUAL(E_OK, ret, "SetupEB write should return E_OK");
    
    /* Test 2.2: Setup read operation */
    printf("Test 2.2: Setup read operation\n");
    ret = I2C_SetupEB(
        1,          /* JobId 1 */
        0x68,       /* RTC address */
        NULL_PTR,   /* No write */
        RxBuffer_Simple,
        4           /* 4 bytes */
    );
    TEST_ASSERT_EQUAL(E_OK, ret, "SetupEB read should return E_OK");
    
    /* Test 2.3: Setup with address override */
    printf("Test 2.3: Setup with address override\n");
    ret = I2C_SetupEB(
        2,          /* JobId 2 */
        0x00,       /* Use configured address */
        TxBuffer_Simple,
        NULL_PTR,
        5
    );
    TEST_ASSERT_EQUAL(E_OK, ret, "SetupEB with zero address should use configured address");
    
    printf("Test Group 2: PASS ✓\n");
    return TEST_PASS;
}

/******************************************************************************
* Test Group 3: Sync Transmit Operations
******************************************************************************/
uint8 Test_Simple_I2C_SyncTransmit(void)
{
    printf("\n=== TEST GROUP 3: SYNC TRANSMIT OPERATIONS ===\n");
    
    /* Initialize mock hardware to simulate success */
    Mock_I2C_Hw_SimulateTransferComplete(0);
    
    /* Setup a write operation */
    memset(TxBuffer_Simple, 0x55, 10);
    I2C_SetupEB(0, 0x50, TxBuffer_Simple, NULL_PTR, 10);
    
    /* Test 3.1: Normal sync transmit */
    printf("Test 3.1: Normal sync transmit\n");
    Std_ReturnType ret = I2C_SyncTransmit(0);
    TEST_ASSERT_EQUAL(E_OK, ret, "SyncTransmit should return E_OK");
    TEST_ASSERT_EQUAL(I2C_SEQ_OK, I2C_GetSequenceResult(0), 
                     "Sequence result should be OK");
    
    /* Test 3.2: Sync transmit when busy */
    printf("Test 3.2: Sync transmit when busy\n");
    ret = I2C_SyncTransmit(0);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret, "SyncTransmit when busy should return E_NOT_OK");
    
    /* Test 3.3: Invalid SequenceId */
    printf("Test 3.3: Invalid SequenceId\n");
    ret = I2C_SyncTransmit(255);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret, "Invalid SequenceId should return E_NOT_OK");
    
    printf("Test Group 3: PASS ✓\n");
    return TEST_PASS;
}

/******************************************************************************
* Test Group 4: Async Transmit Operations
******************************************************************************/
uint8 Test_Simple_I2C_AsyncTransmit(void)
{
    printf("\n=== TEST GROUP 4: ASYNC TRANSMIT OPERATIONS ===\n");
    
    /* Reset and setup */
    Mock_ResetAll();
    I2C_DeInit();
    
    I2C_ConfigType config = {
        .HwUnitBaseAddress = 0x400A0000,
        .BaudRate = 100000,
        .HwUnitMode = I2C_HW_UNIT_MODE_CONTROLLER,
        .TargetListening = FALSE
    };
    I2C_Init(&config);
    
    /* Setup test data */
    memset(TxBuffer_Simple, 0x33, 6);
    I2C_SetupEB(0, 0x50, TxBuffer_Simple, NULL_PTR, 6);
    I2C_SetupEB(1, 0x68, NULL_PTR, RxBuffer_Simple, 4);
    
    /* Test 4.1: Normal async transmit */
    printf("Test 4.1: Normal async transmit\n");
    Std_ReturnType ret = I2C_AsyncTransmit(0);
    TEST_ASSERT_EQUAL(E_OK, ret, "AsyncTransmit should return E_OK");
    TEST_ASSERT_EQUAL(I2C_SEQ_PENDING, I2C_GetSequenceResult(0), 
                     "Sequence should be PENDING");
    
    /* Test 4.2: Queue second async request */
    printf("Test 4.2: Queue second async request\n");
    ret = I2C_AsyncTransmit(1);
    TEST_ASSERT_EQUAL(E_OK, ret, "Second async should be queued");
    TEST_ASSERT_EQUAL(I2C_SEQ_QUEUED, I2C_GetSequenceResult(1), 
                     "Second sequence should be QUEUED");
    
    /* Test 4.3: Same SequenceId again */
    printf("Test 4.3: Same SequenceId again\n");
    ret = I2C_AsyncTransmit(0);
    TEST_ASSERT_EQUAL(E_NOT_OK, ret, "Same SequenceId should return E_NOT_OK");
    
    printf("Test Group 4: PASS ✓\n");
    return TEST_PASS;
}

/******************************************************************************
* Test Group 5: Error Handling
******************************************************************************/
uint8 Test_Simple_I2C_ErrorHandling(void)
{
    printf("\n=== TEST GROUP 5: ERROR HANDLING ===\n");
    
    /* Test 5.1: NACK error */
    printf("Test 5.1: NACK error simulation\n");
    Mock_I2C_Hw_SimulateNACK(0);
    
    /* Execute MainFunction to process errors */
    I2C_MainFunction();
    
    Mock_VerifyCall("Det_ReportRuntimeError", 1);
    Mock_VerifyCall("Dem_SetEventStatus", 1);
    
    /* Test 5.2: Arbitration loss */
    printf("Test 5.2: Arbitration loss simulation\n");
    Mock_I2C_Hw_SimulateArbitrationLoss(0);
    I2C_MainFunction();
    
    Mock_VerifyCall("Det_ReportRuntimeError", 2);
    Mock_VerifyCall("Dem_SetEventStatus", 2);
    
    /* Test 5.3: Bus error */
    printf("Test 5.3: Bus error simulation\n");
    Mock_I2C_Hw_SimulateBusError(0);
    I2C_MainFunction();
    
    Mock_VerifyCall("Det_ReportRuntimeError", 3);
    Mock_VerifyCall("Dem_SetEventStatus", 3);
    
    /* Test 5.4: Deinit without init */
    printf("Test 5.4: Deinit without init\n");
    Mock_ResetAll();
    I2C_DeInit();
    Mock_VerifyCall("Det_ReportError", 1);
    
    printf("Test Group 5: PASS ✓\n");
    return TEST_PASS;
}

/******************************************************************************
* Main Simple Test Runner
******************************************************************************/
uint8 Test_Simple_I2C_Driver(void)
{
    printf("\n" "==========================================");
    printf("\n" "    SIMPLE I2C DRIVER TEST SUITE");
    printf("\n" "==========================================" "\n");
    
    uint8 overallResult = TEST_PASS;
    
    /* Run all simple tests */
    overallResult &= Test_Simple_I2C_BasicAPI();
    overallResult &= Test_Simple_I2C_SetupEB();
    overallResult &= Test_Simple_I2C_SyncTransmit();
    overallResult &= Test_Simple_I2C_AsyncTransmit();
    overallResult &= Test_Simple_I2C_ErrorHandling();
    
    /* Final cleanup */
    I2C_DeInit();
    Mock_ResetAll();
    
    printf("\n" "==========================================");
    printf("\n" "SIMPLE I2C TEST COMPLETE: %s", 
           overallResult == TEST_PASS ? "ALL PASS ✓" : "FAILED ✗");
    printf("\n" "==========================================" "\n");
    
    return overallResult;
}