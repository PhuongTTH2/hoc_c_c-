/**
 * @file    I2c.c
 * @brief   AUTOSAR I2C Driver Implementation with Requirements Mapping
 * @details Implements all 8 core functions with full requirements compliance
 */

#include "I2c.h"
#include "I2c_Private.h"
#include "I2c_Hw.h"
#include "Det.h"

/* Module state */
static boolean I2c_Initialized = FALSE;
I2c_ChannelStateType I2c_ChannelState[I2C_NUM_CHANNELS];

/* ========== REQUIREMENTS MAPPING ========== */

/**
 * @brief I2c_Init - Initializes the I2C module
 * @requirement [CP_SWS_I2C_82002] - Initialization requirements
 */
void I2c_Init(const I2c_ConfigType* ConfigPtr) {
    /* [CP_SWS_I2C_82002] - Validate config and initialize */
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL) {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_INIT, I2C_E_PARAM_POINTER);
        return;
    }
    if (I2c_Initialized == TRUE) {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_INIT, I2C_E_BUSY);
        return;
    }
#endif
    
    /* Initialize all channel states */
    for (uint8 ch = 0; ch < ConfigPtr->NumChannels; ch++) {
        const I2c_ChannelConfigType* chCfg = &ConfigPtr->I2cChannelConfig[ch];
        
        /* Initialize hardware with error checking */
        if (I2c_HwInit(ch, chCfg) != E_OK) {
            Det_ReportError(I2C_MODULE_ID, ch, I2C_SID_INIT, I2C_E_NO_COMM);
            continue;
        }
        
        /* Initialize channel state structure */
        I2c_ChannelState[ch].SeqResult.SequenceResult = I2C_SEQ_OK;
        I2c_ChannelState[ch].SeqResult.ErrorStatus = I2C_NO_ERR;
        I2c_ChannelState[ch].IsListening = FALSE;
        I2c_ChannelState[ch].QueuedCount = 0;
        I2c_ChannelState[ch].ActiveSequence = 0xFF;
        I2c_ChannelState[ch].CurrentJobIndex = 0;
        
        /* Clear queue */
        for (uint8 i = 0; i < I2C_MAX_QUEUE_SIZE; i++) {
            I2c_ChannelState[ch].Queue[i].Status = QUEUE_STATUS_FREE;
        }
        
        /* Configure interrupts if needed */
        if (chCfg->I2cTargetListening == TRUE) {
            I2c_HwConfigureInterrupts(ch, TRUE, I2C_INT_RX);
        }
    }
    
    I2c_Initialized = TRUE;
}

/**
 * @brief I2c_DeInit - Deinitializes the I2C module
 * @requirement [CP_SWS_I2C_82105] - Deinitialization
 * @requirement [CP_SWS_I2C_82108] - Development error detection
 */
void I2c_DeInit(void) {
    /* [CP_SWS_I2C_82108] - Error detection */
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (I2c_Initialized == FALSE) {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_DEINIT, I2C_E_NO_COMM);
        return;
    }
#endif
    
    /* [CP_SWS_I2C_82105] - Deinitialize all channels */
    for (uint8 ch = 0; ch < I2C_NUM_CHANNELS; ch++) {
        /* Stop any ongoing transmission */
        if (I2c_ChannelState[ch].ActiveSequence != 0xFF) {
            I2c_HwGenerateStop(ch);
        }
        
        /* Deinitialize hardware */
        I2c_HwDeInit(ch);
        
        /* Clear channel state */
        I2c_ChannelState[ch].ActiveSequence = 0xFF;
        I2c_ChannelState[ch].IsListening = FALSE;
        I2c_ChannelState[ch].QueuedCount = 0;
    }
    
    I2c_Initialized = FALSE;
}

/**
 * @brief I2c_SetupEB - Sets up data buffer for transmission
 * @requirement [CP_SWS_I2C_00101] - Buffer pointer validation
 * @requirement [CP_SWS_I2C_00102] - Controller mode buffer setup
 * @requirement [CP_SWS_I2C_00103] - Node address override
 * @requirement [CP_SWS_I2C_00104] - JobId validation
 * @requirement [CP_SWS_I2C_00105] - Target mode read setup
 * @requirement [CP_SWS_I2C_00106] - Target mode write setup
 */
Std_ReturnType I2c_SetupEB(uint8 Channel, uint8 Seq, const I2c_DataBufferType* BufPtr) {
    /* [CP_SWS_I2C_00101] - Buffer pointer validation */
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (Channel >= I2C_NUM_CHANNELS) {
        Det_ReportError(I2C_MODULE_ID, Channel, I2C_SID_SETUPEB, I2C_E_PARAM_CHANNEL);
        return E_NOT_OK;
    }
    if (BufPtr == NULL) {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_SETUPEB, I2C_E_PARAM_POINTER);
        return E_NOT_OK;
    }
    if (BufPtr->DataPtr == NULL && BufPtr->Length > 0) {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_SETUPEB, I2C_E_PARAM_DATA);
        return E_NOT_OK;
    }
#endif
    
    const I2c_ChannelConfigType* chCfg = &I2c_Config.I2cChannelConfig[Channel];
    
    if (chCfg->I2cHwUnitMode == I2C_HW_UNIT_MODE_CONTROLLER) {
        /* [CP_SWS_I2C_00102] - Controller mode buffer setup */
        
        /* [CP_SWS_I2C_00104] - JobId validation */
        boolean jobFound = FALSE;
        const I2c_SequenceConfigType* seqCfg = &I2c_Config.I2cSequenceConfig[Seq];
        for (uint8 i = 0; i < seqCfg->NumAssignedJobs; i++) {
            if (seqCfg->I2cAssignedJob[i].I2cJobId == BufPtr->JobId) {
                jobFound = TRUE;
                break;
            }
        }
        
        if (!jobFound) {
            Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_SETUPEB, I2C_E_INVALID_JOB);
            return E_NOT_OK;
        }
        
        /* [CP_SWS_I2C_00103] - Node address override is handled by BufPtr->NodeAddress */
        
        /* Add to queue */
        return I2c_AddToQueue(Channel, Seq, BufPtr);
        
    } else {
        /* Target mode setup */
        if (BufPtr->IsWrite == FALSE) {
            /* [CP_SWS_I2C_00105] - Target mode read setup */
            /* Target will send data when controller reads */
            I2c_ChannelState[Channel].TargetTxBuffer = *BufPtr;
        } else {
            /* [CP_SWS_I2C_00106] - Target mode write setup */
            /* Target will receive data when controller writes */
            I2c_ChannelState[Channel].TargetRxBuffer = *BufPtr;
        }
    }
    
    return E_OK;
}

/**
 * @brief I2c_AsyncTransmit - Starts asynchronous transmission
 * @requirement [CP_SWS_I2C_00310] - SequenceId validation
 * @requirement [CP_SWS_I2C_82303] - No transmission ongoing
 * @requirement [CP_SWS_I2C_82304] - Another transmission ongoing (queue)
 * @requirement [CP_SWS_I2C_82305] - Same transmission ongoing
 * @requirement [CP_SWS_I2C_82307] - Multiple jobs handling
 * @requirement [CP_SWS_I2C_82308] - Continuation with queued elements
 * @requirement [CP_SWS_I2C_82309] - SetupEB validation
 */
Std_ReturnType I2c_AsyncTransmit(uint8 Channel, uint8 Seq) {
    /* [CP_SWS_I2C_00310] - SequenceId validation */
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (Channel >= I2C_NUM_CHANNELS) {
        Det_ReportError(I2C_MODULE_ID, Channel, I2C_SID_ASYNCTRANSMIT, I2C_E_PARAM_CHANNEL);
        return E_NOT_OK;
    }
    if (Seq >= I2C_NUM_SEQUENCES) {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_ASYNCTRANSMIT, I2C_E_PARAM_SEQ);
        return E_NOT_OK;
    }
#endif
    
    /* [CP_SWS_I2C_82305] - Check if same transmission is ongoing */
    if (I2c_ChannelState[Channel].ActiveSequence == Seq) {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_ASYNCTRANSMIT, I2C_E_BUSY);
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_82303] - Check if any transmission is ongoing */
    if (I2c_ChannelState[Channel].ActiveSequence != 0xFF) {
        /* [CP_SWS_I2C_82304] - Queue this request */
        for (uint8 i = 0; i < I2C_MAX_QUEUE_SIZE; i++) {
            if (I2c_ChannelState[Channel].Queue[i].Status == QUEUE_STATUS_FREE) {
                I2c_ChannelState[Channel].Queue[i].SequenceId = Seq;
                I2c_ChannelState[Channel].Queue[i].Status = QUEUE_STATUS_QUEUED;
                I2c_ChannelState[Channel].QueuedCount++;
                I2c_ChannelState[Channel].SeqResult.SequenceResult = I2C_SEQ_QUEUED;
                return E_OK;
            }
        }
        
        /* [CP_SWS_I2C_00702] - FIFO handling error */
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_ASYNCTRANSMIT, I2C_E_QUEUE_FULL);
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_82309] - Validate SetupEB was called */
    const I2c_SequenceConfigType* seqCfg = &I2c_Config.I2cSequenceConfig[Seq];
    boolean bufferFound = FALSE;
    
    for (uint8 i = 0; i < I2C_MAX_QUEUE_SIZE; i++) {
        if (I2c_ChannelState[Channel].Queue[i].SequenceId == Seq &&
            I2c_ChannelState[Channel].Queue[i].Status == QUEUE_STATUS_READY) {
            bufferFound = TRUE;
            break;
        }
    }
    
    if (!bufferFound) {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_ASYNCTRANSMIT, I2C_E_INVALID_JOB);
        return E_NOT_OK;
    }
    
    /* Start transmission */
    I2c_ChannelState[Channel].ActiveSequence = Seq;
    I2c_ChannelState[Channel].SeqResult.SequenceResult = I2C_SEQ_PENDING;
    I2c_ChannelState[Channel].CurrentJobIndex = 0;
    
    /* [CP_SWS_I2C_82307] - Handle multiple jobs */
    return I2c_ProcessNextJob(Channel, Seq);
}

/**
 * @brief I2c_SyncTransmit - Starts synchronous transmission
 * @requirement [CP_SWS_I2C_00410] - SequenceId validation
 * @requirement [CP_SWS_I2C_82403] - No transmission ongoing
 * @requirement [CP_SWS_I2C_82404] - Another transmission ongoing
 * @requirement [CP_SWS_I2C_82407] - Multiple jobs handling
 * @requirement [CP_SWS_I2C_82409] - SetupEB validation
 */
Std_ReturnType I2c_SyncTransmit(uint8 Channel, uint8 Seq, uint32 Timeout) {
    /* [CP_SWS_I2C_00410] - SequenceId validation */
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (Channel >= I2C_NUM_CHANNELS) {
        Det_ReportError(I2C_MODULE_ID, Channel, I2C_SID_SYNCTRANSMIT, I2C_E_PARAM_CHANNEL);
        return E_NOT_OK;
    }
    if (Seq >= I2C_NUM_SEQUENCES) {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_SYNCTRANSMIT, I2C_E_PARAM_SEQ);
        return E_NOT_OK;
    }
#endif
    
    /* [CP_SWS_I2C_82403] - Check if transmission is ongoing */
    if (I2c_ChannelState[Channel].ActiveSequence != 0xFF) {
        /* [CP_SWS_I2C_82404] - Another transmission ongoing */
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_SYNCTRANSMIT, I2C_E_BUSY);
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_82409] - Validate SetupEB */
    const I2c_SequenceConfigType* seqCfg = &I2c_Config.I2cSequenceConfig[Seq];
    boolean bufferFound = FALSE;
    
    for (uint8 i = 0; i < I2C_MAX_QUEUE_SIZE; i++) {
        if (I2c_ChannelState[Channel].Queue[i].SequenceId == Seq &&
            I2c_ChannelState[Channel].Queue[i].Status == QUEUE_STATUS_READY) {
            bufferFound = TRUE;
            break;
        }
    }
    
    if (!bufferFound) {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_SYNCTRANSMIT, I2C_E_INVALID_JOB);
        return E_NOT_OK;
    }
    
    /* Start async transmission */
    Std_ReturnType ret = I2c_AsyncTransmit(Channel, Seq);
    if (ret != E_OK) {
        return ret;
    }
    
    /* Wait for completion with timeout */
    uint32 startTime = GetSystemTick();
    while (I2c_ChannelState[Channel].ActiveSequence != 0xFF) {
        I2c_MainFunction();
        
        if ((GetSystemTick() - startTime) > Timeout) {
            I2c_ChannelState[Channel].SeqResult.SequenceResult = I2C_SEQ_FAILED;
            I2c_ChannelState[Channel].SeqResult.ErrorStatus = I2C_TIMEOUT_ERR;
            return E_NOT_OK;
        }
    }
    
    return E_OK;
}

/**
 * @brief I2c_GetVersionInfo - Returns version information
 * @requirement [CP_SWS_I2C_82601] - Pointer validation
 */
void I2c_GetVersionInfo(Std_VersionInfoType* versioninfo) {
    /* [CP_SWS_I2C_82601] - Pointer validation */
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (versioninfo == NULL) {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_GETVERSIONINFO, I2C_E_PARAM_POINTER);
        return;
    }
#endif
    
    versioninfo->vendorID = I2C_VENDOR_ID;
    versioninfo->moduleID = I2C_MODULE_ID;
    versioninfo->sw_major_version = I2C_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = I2C_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = I2C_SW_PATCH_VERSION;
}

/**
 * @brief I2c_GetSequenceResult - Gets result of a sequence
 * @requirement [CP_SWS_I2C_80701] - Sequence result values
 * @requirement [CP_SWS_I2C_80702] - SequenceId validation
 */
Std_ReturnType I2c_GetSequenceResult(uint8 Channel, uint8 Seq, I2c_SequenceResultType* ResultPtr) {
    /* [CP_SWS_I2C_80702] - SequenceId validation */
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (Channel >= I2C_NUM_CHANNELS) {
        Det_ReportError(I2C_MODULE_ID, Channel, I2C_SID_GETSEQUENCERESULT, I2C_E_PARAM_CHANNEL);
        return E_NOT_OK;
    }
    if (Seq >= I2C_NUM_SEQUENCES) {
        Det_ReturnTypeError(I2C_MODULE_ID, 0, I2C_SID_GETSEQUENCERESULT, I2C_E_PARAM_SEQ);
        return E_NOT_OK;
    }
    if (ResultPtr == NULL) {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_GETSEQUENCERESULT, I2C_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#endif
    
    /* [CP_SWS_I2C_80701] - Return sequence result values */
    *ResultPtr = I2c_ChannelState[Channel].SeqResult;
    
    return E_OK;
}

/**
 * @brief I2c_StartListening - Starts listening in target mode
 * @requirement [CP_SWS_I2C_82806] - SetupEB validation
 * @requirement [CP_SWS_I2C_80801] - No listening ongoing
 * @requirement [CP_SWS_I2C_80802] - Already listening check
 * @requirement [CP_SWS_I2C_80803] - Message/error received handling
 * @requirement [CP_SWS_I2C_80804] - Target mode check
 * @requirement [CP_SWS_I2C_80805] - Wrong mode runtime error
 * @requirement [CP_SWS_I2C_80806] - Message data handling
 */
Std_ReturnType I2c_StartListening(uint8 Channel) {
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (Channel >= I2C_NUM_CHANNELS) {
        Det_ReportError(I2C_MODULE_ID, Channel, I2C_SID_STARTLISTENING, I2C_E_PARAM_CHANNEL);
        return E_NOT_OK;
    }
#endif
    
    const I2c_ChannelConfigType* chCfg = &I2c_Config.I2cChannelConfig[Channel];
    
    /* [CP_SWS_I2C_80804] - Target mode check */
    if (chCfg->I2cHwUnitMode != I2C_HW_UNIT_MODE_TARGET) {
        /* [CP_SWS_I2C_80805] - Wrong mode runtime error */
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_STARTLISTENING, I2C_E_WRONG_MODE);
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_80802] - Already listening check */
    if (I2c_ChannelState[Channel].IsListening == TRUE) {
        return E_OK; /* Already listening */
    }
    
    /* [CP_SWS_I2C_82806] - SetupEB validation */
    if (I2c_ChannelState[Channel].TargetRxBuffer.DataPtr == NULL &&
        I2c_ChannelState[Channel].TargetTxBuffer.DataPtr == NULL) {
        /* SetupEB not called */
        Det_ReportError(I2C_MODULE_ID, 0, I2C_SID_STARTLISTENING, I2C_E_INVALID_JOB);
        return E_NOT_OK;
    }
    
    /* Start hardware listening */
    if (I2c_HwStartListening(Channel) != E_OK) {
        return E_NOT_OK;
    }
    
    I2c_ChannelState[Channel].IsListening = TRUE;
    
    return E_OK;
}

/**
 * @brief I2c_MainFunction - Main function for processing
 * @requirement [CP_SWS_I2C_80901] - Permanent listening handling
 */
void I2c_MainFunction(void) {
    for (uint8 ch = 0; ch < I2C_NUM_CHANNELS; ch++) {
        /* Check for transmission completion */
        if (I2c_ChannelState[ch].ActiveSequence != 0xFF) {
            if (I2c_HwIsTransferComplete(ch)) {
                I2c_HandleTransferComplete(ch);
            }
            
            /* Check for errors */
            uint8 hwStatus = I2c_HwGetStatus(ch);
            
            /* [CP_SWS_I2C_00703] - NACK received error */
            if (hwStatus & I2C_HW_STATUS_NACK) {
                I2c_ChannelState[ch].SeqResult.ErrorStatus = I2C_NACK_RECEIVED_ERR;
                I2c_ChannelState[ch].SeqResult.SequenceResult = I2C_SEQ_FAILED;
                I2c_HwClearErrors(ch);
                I2c_CleanupChannel(ch);
            }
            
            /* [CP_SWS_I2C_00704] - Arbitration failure */
            if (hwStatus & I2C_HW_STATUS_ARB_LOST) {
                I2c_ChannelState[ch].SeqResult.ErrorStatus = I2C_ARBITRATION_LOST_ERR;
                I2c_ChannelState[ch].SeqResult.SequenceResult = I2C_SEQ_FAILED;
                I2c_HwClearErrors(ch);
                I2c_CleanupChannel(ch);
            }
            
            /* [CP_SWS_I2C_00705] - Bus failure */
            if (hwStatus & I2C_HW_STATUS_BUS_ERROR) {
                I2c_ChannelState[ch].SeqResult.ErrorStatus = I2C_BUS_FAILURE_ERR;
                I2c_ChannelState[ch].SeqResult.SequenceResult = I2C_SEQ_FAILED;
                I2c_HwClearErrors(ch);
                I2c_CleanupChannel(ch);
            }
        }
        
        /* [CP_SWS_I2C_80901] - Permanent listening handling */
        if (I2c_ChannelState[ch].IsListening) {
            I2c_HandleTargetRequest(ch);
        }
        
        /* [CP_SWS_I2C_82308] - Process queued sequences */
        I2c_ProcessQueuedSequences(ch);
    }
}

/* ========== INTERNAL HELPER FUNCTIONS ========== */

/**
 * @brief I2c_AddToQueue - Adds buffer to transmission queue
 */
static Std_ReturnType I2c_AddToQueue(uint8 Channel, uint8 Seq, const I2c_DataBufferType* BufPtr) {
    /* Find free queue slot */
    for (uint8 i = 0; i < I2C_MAX_QUEUE_SIZE; i++) {
        if (I2c_ChannelState[Channel].Queue[i].Status == QUEUE_STATUS_FREE) {
            /* Copy buffer data */
            I2c_ChannelState[Channel].Queue[i].Buffer = *BufPtr;
            I2c_ChannelState[Channel].Queue[i].SequenceId = Seq;
            I2c_ChannelState[Channel].Queue[i].Status = QUEUE_STATUS_READY;
            I2c_ChannelState[Channel].Queue[i].Channel = Channel;
            I2c_ChannelState[Channel].QueuedCount++;
            return E_OK;
        }
    }
    
    /* [CP_SWS_I2C_00702] - FIFO handling error */
    Det_ReportError(I2C_MODULE_ID, 0, 0, I2C_E_QUEUE_FULL);
    return E_NOT_OK;
}

/**
 * @brief I2c_ProcessNextJob - Processes next job in sequence
 * @requirement [CP_SWS_I2C_82307] - Multiple jobs handling
 */
static Std_ReturnType I2c_ProcessNextJob(uint8 Channel, uint8 Seq) {
    const I2c_SequenceConfigType* seqCfg = &I2c_Config.I2cSequenceConfig[Seq];
    uint8 jobIndex = I2c_ChannelState[Channel].CurrentJobIndex;
    
    if (jobIndex >= seqCfg->NumAssignedJobs) {
        /* All jobs completed */
        I2c_ChannelState[Channel].ActiveSequence = 0xFF;
        I2c_ChannelState[Channel].SeqResult.SequenceResult = I2C_SEQ_OK;
        
        /* Call notification callback */
        if (seqCfg->I2cEndNotification != NULL) {
            seqCfg->I2cEndNotification(Channel, Seq);
        }
        
        return E_OK;
    }
    
    /* Get current job */
    const I2c_JobConfigType* jobCfg = &seqCfg->I2cAssignedJob[jobIndex];
    
    /* Find buffer for this job */
    for (uint8 i = 0; i < I2C_MAX_QUEUE_SIZE; i++) {
        if (I2c_ChannelState[Channel].Queue[i].SequenceId == Seq &&
            I2c_ChannelState[Channel].Queue[i].Buffer.JobId == jobCfg->I2cJobId &&
            I2c_ChannelState[Channel].Queue[i].Status == QUEUE_STATUS_READY) {
            
            /* Start hardware transmission */
            uint8 address = (I2c_ChannelState[Channel].Queue[i].Buffer.NodeAddress != 0) ?
                            I2c_ChannelState[Channel].Queue[i].Buffer.NodeAddress :
                            jobCfg->I2cDeviceAddress;
            
            if (I2c_HwStartTransmit(Channel, address, 
                                   I2c_ChannelState[Channel].Queue[i].Buffer.IsWrite) != E_OK) {
                return E_NOT_OK;
            }
            
            /* Mark as active */
            I2c_ChannelState[Channel].Queue[i].Status = QUEUE_STATUS_ACTIVE;
            
            return E_OK;
        }
    }
    
    return E_NOT_OK;
}

/**
 * @brief I2c_HandleTransferComplete - Handles transmission completion
 */
static void I2c_HandleTransferComplete(uint8 Channel) {
    uint8 seqId = I2c_ChannelState[Channel].ActiveSequence;
    
    /* Find and mark completed job */
    for (uint8 i = 0; i < I2C_MAX_QUEUE_SIZE; i++) {
        if (I2c_ChannelState[Channel].Queue[i].SequenceId == seqId &&
            I2c_ChannelState[Channel].Queue[i].Status == QUEUE_STATUS_ACTIVE) {
            
            I2c_ChannelState[Channel].Queue[i].Status = QUEUE_STATUS_COMPLETED;
            I2c_ChannelState[Channel].QueuedCount--;
            break;
        }
    }
    
    /* Move to next job */
    I2c_ChannelState[Channel].CurrentJobIndex++;
    I2c_ProcessNextJob(Channel, seqId);
}

/**
 * @brief I2c_HandleTargetRequest - Handles incoming target request
 * @requirement [CP_SWS_I2C_80803] - Message/error received handling
 * @requirement [CP_SWS_I2C_80806] - Message data handling
 */
static void I2c_HandleTargetRequest(uint8 Channel) {
    /* Check if data received */
    if (I2c_HwIsRxReady(Channel)) {
        uint8 data;
        if (I2c_HwReceiveByte(Channel, &data, TRUE) == E_OK) {
            /* [CP_SWS_I2C_80806] - Handle received data */
            if (I2c_ChannelState[Channel].TargetRxBuffer.DataPtr != NULL &&
                I2c_ChannelState[Channel].TargetRxBuffer.Length > 0) {
                
                *I2c_ChannelState[Channel].TargetRxBuffer.DataPtr = data;
                I2c_ChannelState[Channel].TargetRxBuffer.DataPtr++;
                I2c_ChannelState[Channel].TargetRxBuffer.Length--;
            }
        }
    }
    
    /* [CP_SWS_I2C_80803] - Handle errors */
    uint8 hwStatus = I2c_HwGetStatus(Channel);
    if (hwStatus & I2C_HW_STATUS_ERROR) {
        /* Error handling */
        I2c_HwClearErrors(Channel);
    }
}

/**
 * @brief I2c_ProcessQueuedSequences - Processes queued sequences
 * @requirement [CP_SWS_I2C_82308] - Continuation with queued elements
 */
static void I2c_ProcessQueuedSequences(uint8 Channel) {
    if (I2c_ChannelState[Channel].ActiveSequence != 0xFF) {
        return; /* Still busy */
    }
    
    if (I2c_ChannelState[Channel].QueuedCount == 0) {
        return; /* No queued sequences */
    }
    
    /* Find next queued sequence */
    for (uint8 i = 0; i < I2C_MAX_QUEUE_SIZE; i++) {
        if (I2c_ChannelState[Channel].Queue[i].Status == QUEUE_STATUS_READY) {
            uint8 seqId = I2c_ChannelState[Channel].Queue[i].SequenceId;
            I2c_AsyncTransmit(Channel, seqId);
            break;
        }
    }
}

/**
 * @brief I2c_CleanupChannel - Cleans up channel after error
 */
static void I2c_CleanupChannel(uint8 Channel) {
    /* Generate stop condition */
    I2c_HwGenerateStop(Channel);
    
    /* Clear queue */
    for (uint8 i = 0; i < I2C_MAX_QUEUE_SIZE; i++) {
        if (I2c_ChannelState[Channel].Queue[i].Channel == Channel) {
            I2c_ChannelState[Channel].Queue[i].Status = QUEUE_STATUS_FREE;
        }
    }
    
    I2c_ChannelState[Channel].QueuedCount = 0;
    I2c_ChannelState[Channel].ActiveSequence = 0xFF;
    I2c_ChannelState[Channel].CurrentJobIndex = 0;
}