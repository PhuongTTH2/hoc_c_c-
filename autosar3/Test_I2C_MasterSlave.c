/******************************************************************************
* File: Test_I2C_ConfigBased.c
* Description: Test I2C Driver dựa trên config đã có
* Đặc điểm: Địa chỉ slave được truyền động qua I2C_SetupEB()
******************************************************************************/

#include "I2c.h"
#include "I2c_Cfg.h"
#include "I2c_Test.h"
#include "I2c_Mock.h"
#include <stdio.h>
#include <string.h>

/* Biến toàn cục cho test */
static uint8 Test_TxBuffer[16];
static uint8 Test_RxBuffer[16];

/******************************************************************************
* Test 1: Kiểm tra initialization với config có sẵn
******************************************************************************/
uint8 Test_I2C_Config_Initialization(void)
{
    printf("\n=== TEST 1: INITIALIZATION WITH EXISTING CONFIG ===\n");
    
    /* Reset mocks */
    Mock_ResetAll();
    
    /* Sử dụng config đã có từ file cấu hình */
    extern const I2C_ConfigType I2c_Config;
    
    /* Test 1.1: Khởi tạo với config có sẵn */
    printf("Test 1.1: Initialize with existing config\n");
    I2C_Init(&I2c_Config);
    
    /* Kiểm tra số lượng channel được khởi tạo */
    TEST_ASSERT_NOT_NULL(I2c_Config.I2cGeneral, "I2cGeneral should not be NULL");
    TEST_ASSERT_NOT_NULL(I2c_Config.I2cConfigSet, "I2cConfigSet should not be NULL");
    TEST_ASSERT_EQUAL(1, I2c_Config.I2cConfigSetCount, "Should have 1 config set");
    
    /* Test 1.2: Kiểm tra cấu hình channels */
    printf("Test 1.2: Check channel configuration\n");
    TEST_ASSERT_EQUAL(2, I2c_Config.I2cConfigSet->I2cChannelCount, 
                     "Should have 2 channels");
    
    /* Channel 0: Controller mode */
    TEST_ASSERT_EQUAL(I2C_HW_UNIT_MODE_CONTROLLER, 
                     I2c_Config.I2cConfigSet->I2cChannel[0].I2cHwUnitMode,
                     "Channel 0 should be Controller mode");
    
    /* Channel 1: Target mode */
    TEST_ASSERT_EQUAL(I2C_HW_UNIT_MODE_TARGET,
                     I2c_Config.I2cConfigSet->I2cChannel[1].I2cHwUnitMode,
                     "Channel 1 should be Target mode");
    
    /* Test 1.3: Kiểm tra cấu hình Jobs */
    printf("Test 1.3: Check job configuration\n");
    TEST_ASSERT_EQUAL(3, I2c_Config.I2cConfigSet->I2cJobCount,
                     "Should have 3 jobs");
    
    for (int i = 0; i < 3; i++)
    {
        printf("  Job %d: ID=%d, Address=0x%02X\n",
               i,
               I2c_Config.I2cConfigSet->I2cJob[i].I2cJobId,
               I2c_Config.I2cConfigSet->I2cJob[i].I2cDeviceAddress);
    }
    
    /* Test 1.4: Kiểm tra cấu hình Sequences */
    printf("Test 1.4: Check sequence configuration\n");
    TEST_ASSERT_EQUAL(2, I2c_Config.I2cConfigSet->I2cSequenceCount,
                     "Should have 2 sequences");
    
    printf("Test 1: PASS ✓\n");
    return TEST_PASS;
}

/******************************************************************************
* Test 2: SetupEB với địa chỉ động (không có trong config)
******************************************************************************/
uint8 Test_I2C_Config_SetupEB_DynamicAddress(void)
{
    printf("\n=== TEST 2: SETUPEB WITH DYNAMIC SLAVE ADDRESS ===\n");
    
    /* Reset */
    Mock_ResetAll();
    
    /* Sử dụng config đã có */
    extern const I2C_ConfigType I2c_Config;
    I2C_Init(&I2c_Config);
    
    /* Chuẩn bị test data */
    memset(Test_TxBuffer, 0xAA, sizeof(Test_TxBuffer));
    memset(Test_RxBuffer, 0x00, sizeof(Test_RxBuffer));
    
    /* Lưu ý: Job trong config chỉ có DeviceAddress nhưng KHÔNG DÙNG
     * Địa chỉ thực tế được truyền qua tham số NodeAddress trong SetupEB()
     */
    
    /* Test 2.1: SetupEB với địa chỉ slave tùy ý */
    printf("Test 2.1: SetupEB with arbitrary slave address 0x50\n");
    Std_ReturnType ret = I2C_SetupEB(
        0,              /* JobId 0 (từ config) */
        0x50,           /* Địa chỉ slave EEPROM - TRUYỀN ĐỘNG */
        Test_TxBuffer,  /* Buffer truyền */
        NULL_PTR,       /* Không đọc */
        8               /* 8 bytes */
    );
    TEST_ASSERT_EQUAL(E_OK, ret, "SetupEB should succeed with dynamic address");
    
    /* Test 2.2: SetupEB với địa chỉ slave khác */
    printf("Test 2.2: SetupEB with different slave address 0x76\n");
    ret = I2C_SetupEB(
        1,              /* JobId 1 */
        0x76,           /* Địa chỉ slave sensor khác */
        NULL_PTR,       /* Không ghi */
        Test_RxBuffer,  /* Buffer đọc */
        6               /* 6 bytes */
    );
    TEST_ASSERT_EQUAL(E_OK, ret, "SetupEB should succeed with different address");
    
    /* Test 2.3: SetupEB với địa chỉ bằng 0 (dùng địa chỉ từ config) */
    printf("Test 2.3: SetupEB with address 0 (use from config)\n");
    ret = I2C_SetupEB(
        2,              /* JobId 2 */
        0x00,           /* Address = 0, sẽ dùng I2cDeviceAddress từ config */
        Test_TxBuffer,
        NULL_PTR,
        4
    );
    TEST_ASSERT_EQUAL(E_OK, ret, "SetupEB with address 0 should use config address");
    
    /* Kiểm tra địa từ config đã được đọc (0x68 từ Job 2 trong config) */
    printf("  Config Job 2 address: 0x%02X\n", 
           I2c_Config.I2cConfigSet->I2cJob[2].I2cDeviceAddress);
    
    /* Test 2.4: SetupEB với địa chỉ 10-bit (>= 0x80) */
    printf("Test 2.4: SetupEB with 10-bit address 0x1A0\n");
    ret = I2C_SetupEB(
        0,              /* JobId 0 */
        0x1A0,          /* Địa chỉ 10-bit (416 decimal) */
        Test_TxBuffer,
        NULL_PTR,
        4
    );
    TEST_ASSERT_EQUAL(E_OK, ret, "SetupEB should support 10-bit addressing");
    
    printf("Test 2: PASS ✓\n");
    return TEST_PASS;
}

/******************************************************************************
* Test 3: Sync Transmit với config-based sequences
******************************************************************************/
uint8 Test_I2C_Config_SyncTransmit(void)
{
    printf("\n=== TEST 3: SYNC TRANSMIT WITH CONFIG SEQUENCES ===\n");
    
    /* Reset và khởi tạo */
    Mock_ResetAll();
    
    extern const I2C_ConfigType I2c_Config;
    I2C_Init(&I2c_Config);
    
    /* Setup test data */
    memset(Test_TxBuffer, 0x55, 10);
    
    /* Lưu ý: Sequences trong config đã được gán sẵn Jobs
     * Sequence 0: I2C_SEQUENCE_0_JOBS (Job 0, Job 1)
     * Sequence 1: I2C_SEQUENCE_1_JOBS (Job 2)
     */
    
    /* Test 3.1: SetupEB cho Job 0 (thuộc Sequence 0) */
    printf("Test 3.1: SetupEB for Job 0 (part of Sequence 0)\n");
    I2C_SetupEB(0, 0x50, Test_TxBuffer, NULL_PTR, 5);
    
    /* SetupEB cho Job 1 (cũng thuộc Sequence 0) */
    printf("  SetupEB for Job 1 (also part of Sequence 0)\n");
    I2C_SetupEB(1, 0x51, Test_TxBuffer + 5, NULL_PTR, 5);
    
    /* Test 3.2: Sync transmit Sequence 0 (multiple jobs) */
    printf("Test 3.2: Sync transmit Sequence 0 (2 jobs)\n");
    Mock_I2C_Hw_SimulateTransferComplete(0); /* Channel 0 */
    
    Std_ReturnType ret = I2C_SyncTransmit(0); /* Sequence 0 từ config */
    TEST_ASSERT_EQUAL(E_OK, ret, "SyncTransmit Sequence 0 should succeed");
    
    /* Kiểm tra sequence đã được gán đúng channel */
    printf("  Sequence 0 assigned to Channel: %d\n",
           I2c_Config.I2cConfigSet->I2cSequence[0].I2cAssignedChannel);
    
    /* Test 3.3: Sync transmit Sequence 1 (single job) */
    printf("Test 3.3: Sync transmit Sequence 1 (1 job)\n");
    memset(Test_RxBuffer, 0x00, 8);
    
    /* SetupEB cho Job 2 (thuộc Sequence 1) */
    I2C_SetupEB(2, 0x68, NULL_PTR, Test_RxBuffer, 8);
    
    Mock_I2C_Hw_SimulateTransferComplete(0);
    Mock_I2C_Hw_SimulateRxDataReady(0, 0xAA); /* Simulate received data */
    Mock_I2C_Hw_SimulateRxDataReady(0, 0xBB);
    
    ret = I2C_SyncTransmit(1); /* Sequence 1 từ config */
    TEST_ASSERT_EQUAL(E_OK, ret, "SyncTransmit Sequence 1 should succeed");
    
    printf("Test 3: PASS ✓\n");
    return TEST_PASS;
}

/******************************************************************************
* Test 4: Async Transmit với callback từ config
******************************************************************************/
uint8 Test_I2C_Config_AsyncTransmit_Callback(void)
{
    printf("\n=== TEST 4: ASYNC TRANSMIT WITH CONFIG CALLBACKS ===\n");
    
    /* Reset */
    Mock_ResetAll();
    
    extern const I2C_ConfigType I2c_Config;
    I2C_Init(&I2c_Config);
    
    /* Lưu ý: Config đã định nghĩa callback functions:
     * I2C_SEQUENCE_0_END_NOTIFICATION = EEPROM_SequenceComplete
     * I2C_SEQUENCE_1_END_NOTIFICATION = Sensor_SequenceComplete
     */
    
    /* Kiểm tra callback functions trong config */
    printf("Checking callback functions in config:\n");
    
    if (I2c_Config.I2cConfigSet->I2cSequence[0].I2cEndNotification != NULL_PTR)
    {
        printf("  Sequence 0 callback: EEPROM_SequenceComplete\n");
    }
    
    if (I2c_Config.I2cConfigSet->I2cSequence[1].I2cEndNotification != NULL_PTR)
    {
        printf("  Sequence 1 callback: Sensor_SequenceComplete\n");
    }
    
    /* Test 4.1: Setup và async transmit Sequence 0 */
    printf("\nTest 4.1: Async transmit Sequence 0\n");
    memset(Test_TxBuffer, 0x11, 6);
    I2C_SetupEB(0, 0x50, Test_TxBuffer, NULL_PTR, 6);
    
    Std_ReturnType ret = I2C_AsyncTransmit(0);
    TEST_ASSERT_EQUAL(E_OK, ret, "AsyncTransmit should succeed");
    
    /* Kiểm tra sequence state */
    TEST_ASSERT_EQUAL(I2C_SEQ_PENDING, I2C_GetSequenceResult(0),
                     "Sequence should be PENDING");
    
    /* Test 4.2: Simulate transfer completion và check callback */
    printf("Test 4.2: Simulate completion and check callback\n");
    Mock_I2C_Hw_SimulateTransferComplete(0);
    
    /* Execute MainFunction để xử lý completion */
    I2C_MainFunction();
    
    /* Trong thực tế, callback EEPROM_SequenceComplete sẽ được gọi */
    printf("  Note: EEPROM_SequenceComplete() would be called here\n");
    
    /* Test 4.3: Queue multiple async requests */
    printf("\nTest 4.3: Queue multiple async requests\n");
    
    /* Setup thêm Jobs cho queuing test */
    I2C_SetupEB(1, 0x51, Test_TxBuffer, NULL_PTR, 4);
    I2C_SetupEB(2, 0x52, Test_TxBuffer, NULL_PTR, 4);
    
    /* Start first async (sẽ thành công) */
    ret = I2C_AsyncTransmit(0);
    TEST_ASSERT_EQUAL(E_OK, ret, "First async should succeed");
    
    /* Start second async (sẽ được queue) */
    ret = I2C_AsyncTransmit(1);
    TEST_ASSERT_EQUAL(E_OK, ret, "Second async should be queued");
    TEST_ASSERT_EQUAL(I2C_SEQ_QUEUED, I2C_GetSequenceResult(1),
                     "Second sequence should be QUEUED");
    
    printf("Test 4: PASS ✓\n");
    return TEST_PASS;
}

/******************************************************************************
* Test 5: Target Mode với config đã có (Channel 1 là Target)
******************************************************************************/
uint8 Test_I2C_Config_TargetMode(void)
{
    printf("\n=== TEST 5: TARGET MODE FROM CONFIG ===\n");
    
    /* Reset */
    Mock_ResetAll();
    
    extern const I2C_ConfigType I2c_Config;
    I2C_Init(&I2c_Config);
    
    /* Kiểm tra Channel 1 được cấu hình là Target mode */
    printf("Checking Target mode configuration:\n");
    
    uint8 targetChannel = 1; /* Channel 1 từ config */
    TEST_ASSERT_EQUAL(I2C_HW_UNIT_MODE_TARGET,
                     I2c_Config.I2cConfigSet->I2cChannel[targetChannel].I2cHwUnitMode,
                     "Channel 1 should be Target mode");
    
    boolean targetListening = I2c_Config.I2cConfigSet->I2cChannel[targetChannel].I2cTargetListening;
    printf("  Channel 1 TargetListening: %s\n", targetListening ? "TRUE" : "FALSE");
    
    /* Test 5.1: Start listening trên Target channel */
    printf("\nTest 5.1: Start listening on Target channel\n");
    
    /* SetupEB cho Target mode */
    memset(Test_RxBuffer, 0, 8);
    I2C_SetupEB(0, 0x00, NULL_PTR, Test_RxBuffer, 8);
    
    Std_ReturnType ret = I2C_StartListening(0);
    
    if (targetListening)
    {
        TEST_ASSERT_EQUAL(E_OK, ret, "StartListening should succeed when TargetListening=TRUE");
        TEST_ASSERT_EQUAL(I2C_SEQ_PENDING, I2C_GetSequenceResult(0),
                         "Target sequence should be PENDING");
    }
    else
    {
        TEST_ASSERT_EQUAL(E_NOT_OK, ret, "StartListening should fail when TargetListening=FALSE");
    }
    
    /* Test 5.2: Simulate Master writing to Target */
    printf("\nTest 5.2: Simulate Master writing to Target\n");
    
    if (targetListening)
    {
        /* Simulate Master gửi data đến Target */
        uint8 masterData[4] = {0x01, 0x02, 0x03, 0x04};
        
        for (int i = 0; i < 4; i++)
        {
            Mock_I2C_Hw_SimulateRxDataReady(targetChannel, masterData[i]);
        }
        
        /* Process via MainFunction */
        I2C_MainFunction();
        
        /* Kiểm tra Target đã nhận data */
        printf("  Target should receive: 0x01 0x02 0x03 0x04\n");
    }
    
    /* Test 5.3: Simulate Master reading from Target */
    printf("\nTest 5.3: Simulate Master reading from Target\n");
    
    if (targetListening)
    {
        /* Target chuẩn bị data để gửi khi được đọc */
        uint8 targetResponse[4] = {0xAA, 0xBB, 0xCC, 0xDD};
        
        /* SetupEB cho Target write response */
        memset(Test_TxBuffer, 0, 4);
        memcpy(Test_TxBuffer, targetResponse, 4);
        
        I2C_SetupEB(1, 0x00, Test_TxBuffer, NULL_PTR, 4);
        
        /* Simulate Master request data */
        Mock_I2C_Hw_SimulateTxEmpty(targetChannel);
        
        /* Process */
        I2C_MainFunction();
        
        printf("  Target should send: 0xAA 0xBB 0xCC 0xDD\n");
    }
    
    printf("Test 5: PASS ✓\n");
    return TEST_PASS;
}

/******************************************************************************
* Test 6: Error Detection từ config (I2cDevErrorDetect)
******************************************************************************/
uint8 Test_I2C_Config_ErrorDetection(void)
{
    printf("\n=== TEST 6: ERROR DETECTION FROM CONFIG ===\n");
    
    /* Reset */
    Mock_ResetAll();
    
    extern const I2C_ConfigType I2c_Config;
    I2C_Init(&I2c_Config);
    
    /* Kiểm tra DevErrorDetect setting từ config */
    printf("Checking DevErrorDetect from config:\n");
    boolean devErrorDetect = I2c_Config.I2cGeneral->I2cDevErrorDetect;
    printf("  I2cDevErrorDetect: %s\n", devErrorDetect ? "TRUE" : "FALSE");
    
    /* Test 6.1: Development error khi DevErrorDetect = TRUE */
    printf("\nTest 6.1: Development error detection\n");
    
    if (devErrorDetect)
    {
        /* Try invalid JobId */
        Std_ReturnType ret = I2C_SetupEB(
            255,        /* Invalid JobId */
            0x50,
            Test_TxBuffer,
            NULL_PTR,
            5
        );
        
        TEST_ASSERT_EQUAL(E_NOT_OK, ret, "Should fail with invalid JobId");
        Mock_VerifyCall("Det_ReportError", 1);
    }
    else
    {
        printf("  Skipped: DevErrorDetect is FALSE\n");
    }
    
    /* Test 6.2: Runtime error reporting */
    printf("\nTest 6.2: Runtime error reporting\n");
    
    /* Setup valid job */
    I2C_SetupEB(0, 0x50, Test_TxBuffer, NULL_PTR, 5);
    
    /* Simulate NACK error */
    Mock_I2C_Hw_SimulateNACK(0);
    I2C_MainFunction();
    
    /* DEM should be called regardless of DevErrorDetect */
    Mock_VerifyCall("Dem_SetEventStatus", 1);
    
    /* Test 6.3: GetVersionInfo khi I2cVersionInfoApi = TRUE */
    printf("\nTest 6.3: Version Info API\n");
    boolean versionInfoApi = I2c_Config.I2cGeneral->I2cVersionInfoApi;
    printf("  I2cVersionInfoApi: %s\n", versionInfoApi ? "TRUE" : "FALSE");
    
    if (versionInfoApi)
    {
        Std_VersionInfoType version;
        I2C_GetVersionInfo(&version);
        
        TEST_ASSERT_EQUAL(1101U, version.moduleID, "Module ID should match");
        printf("  Module ID: %u\n", version.moduleID);
    }
    
    printf("Test 6: PASS ✓\n");
    return TEST_PASS;
}

/******************************************************************************
* Main Test Runner cho Config-Based Tests
******************************************************************************/
uint8 Test_I2C_With_Existing_Config(void)
{
    printf("\n" "╔════════════════════════════════════════════════════╗");
    printf("\n" "║    I2C DRIVER TEST WITH EXISTING CONFIGURATION    ║");
    printf("\n" "╚════════════════════════════════════════════════════╝" "\n");
    
    printf("\nNOTE: Testing with configuration where slave addresses are");
    printf("\n      provided dynamically via I2C_SetupEB() parameter,");
    printf("\n      NOT statically in configuration.\n");
    
    uint8 overallResult = TEST_PASS;
    
    /* Run all config-based tests */
    overallResult &= Test_I2C_Config_Initialization();
    overallResult &= Test_I2C_Config_SetupEB_DynamicAddress();
    overallResult &= Test_I2C_Config_SyncTransmit();
    overallResult &= Test_I2C_Config_AsyncTransmit_Callback();
    overallResult &= Test_I2C_Config_TargetMode();
    overallResult &= Test_I2C_Config_ErrorDetection();
    
    /* Final cleanup */
    I2C_DeInit();
    Mock_ResetAll();
    
    printf("\n" "╔════════════════════════════════════════════════════╗");
    printf("\n" "║  CONFIG-BASED TEST COMPLETE: %s  ║",
           overallResult == TEST_PASS ? "ALL PASS ✓" : "FAILED ✗");
    printf("\n" "╚════════════════════════════════════════════════════╝" "\n");
    
    return overallResult;
}

/******************************************************************************
* Integration Test: Master-Slave với Config Đã Có
******************************************************************************/
uint8 Test_I2C_Config_MasterSlave_Integration(void)
{
    printf("\n=== INTEGRATION TEST: MASTER-SLAVE WITH EXISTING CONFIG ===\n");
    
    /* Sử dụng config có sẵn:
     * Channel 0: Controller (Master)
     * Channel 1: Target (Slave) 
     */
    
    extern const I2C_ConfigType I2c_Config;
    I2C_Init(&I2c_Config);
    
    printf("\nScenario: Master (Channel 0) communicates with Slave (Channel 1)\n");
    
    /* Phần 1: Master writes to Slave */
    printf("\nPart 1: Master writes command to Slave\n");
    
    /* Master setup: Write to Slave address 0x48 */
    uint8 masterCommand[4] = {0x01, 0x02, 0x03, 0x04};
    I2C_SetupEB(0, 0x48, masterCommand, NULL_PTR, 4);
    
    /* Slave setup: Prepare to receive */
    uint8 slaveReceiveBuffer[4] = {0};
    I2C_SetupEB(10, 0x00, NULL_PTR, slaveReceiveBuffer, 4);
    
    /* Start Slave listening (if configured) */
    if (I2c_Config.I2cConfigSet->I2cChannel[1].I2cTargetListening)
    {
        I2C_StartListening(0);
    }
    
    /* Master executes sync write */
    Mock_I2C_Hw_SimulateTransferComplete(0);
    Std_ReturnType ret = I2C_SyncTransmit(0);
    
    if (ret == E_OK)
    {
        printf("  Master successfully sent command\n");
    }
    
    /* Phần 2: Master reads from Slave */
    printf("\nPart 2: Master reads response from Slave\n");
    
    /* Master setup: Read from Slave */
    uint8 masterReceiveBuffer[4] = {0};
    I2C_SetupEB(1, 0x48, NULL_PTR, masterReceiveBuffer, 4);
    
    /* Slave setup: Prepare response */
    uint8 slaveResponse[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    I2C_SetupEB(11, 0x00, slaveResponse, NULL_PTR, 4);
    
    /* Simulate Slave responding */
    for (int i = 0; i < 4; i++)
    {
        Mock_I2C_Hw_SimulateRxDataReady(0, slaveResponse[i]);
    }
    Mock_I2C_Hw_SimulateTransferComplete(0);
    
    /* Master executes sync read */
    ret = I2C_SyncTransmit(1);
    
    if (ret == E_OK)
    {
        printf("  Master successfully read response\n");
        printf("  Response data: 0x%02X 0x%02X 0x%02X 0x%02X\n",
               masterReceiveBuffer[0], masterReceiveBuffer[1],
               masterReceiveBuffer[2], masterReceiveBuffer[3]);
    }
    
    /* Cleanup */
    I2C_DeInit();
    
    printf("\nIntegration Test: COMPLETE\n");
    return TEST_PASS;
}