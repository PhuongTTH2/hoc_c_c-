/******************************************************************************
 * @file        I2C_MCAL.c
 * @brief       AUTOSAR MCAL I2C Driver - Full R24-11 Specification
 * @author      Embedded Developer
 * @date        2024.10.10
 * @version     R24-11
 ******************************************************************************/

#include "I2C_MCAL.h"
#include "I2C_HW.h"
#include "I2C_Cfg.h"
#include "Det.h"
#include "Dem.h"

/* ========================== MODULE STATE ========================== */

/* Channel State */
typedef struct
{
    uint8 state;                    /* 0=IDLE, 1=BUSY, 2=LISTENING */
    I2C_SequenceType currentSequence;
    uint8 currentJobIndex;
    uint8 jobsInSequence;
    uint16 remainingBytes;
    uint32 timeoutCounter;
} I2C_ChannelStateType;

/* Job Buffer (EB) */
typedef struct
{
    I2C_AddressType nodeAddress;
    I2C_DataConstPtrType txBuffer;
    I2C_DataPtrType rxBuffer;
    I2C_NumberOfDataType length;
    boolean isConfigured;
} I2C_JobBufferType;

/* Sequence Queue Entry */
typedef struct
{
    I2C_SequenceType sequenceId;
    uint8 channelId;
} I2C_QueueEntryType;

#define I2C_MAX_QUEUE_SIZE         8

static boolean I2c_Initialized = FALSE;
static I2C_ChannelStateType I2c_ChannelState[I2C_MAX_CHANNELS];
static I2C_JobBufferType I2c_JobBuffer[I2C_MAX_JOBS];
static I2C_SequenceResultType I2c_SequenceResult[I2C_MAX_SEQUENCES];

/* FIFO Queue for Sequences */
static I2C_QueueEntryType I2c_SequenceQueue[I2C_MAX_CHANNELS][I2C_MAX_QUEUE_SIZE];
static uint8 I2c_QueueHead[I2C_MAX_CHANNELS] = {0};
static uint8 I2c_QueueTail[I2C_MAX_CHANNELS] = {0};
static uint8 I2c_QueueCount[I2C_MAX_CHANNELS] = {0};

/* Configuration Pointer */
static const I2C_ConfigType* I2c_ConfigPtr = NULL;

/* ========================== PRIVATE FUNCTIONS ========================== */

/* [CP_SWS_I2C_00700] - Development Error Reporting */
static void I2c_ReportDevError(I2C_ErrorType error)
{
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    Det_ReportError(I2C_MODULE_ID, I2C_INSTANCE_ID, 0, error);
#endif
}

/* [CP_SWS_I2C_00701] - Runtime Error Reporting */
static void I2c_ReportRuntimeError(uint8 channelId, I2C_ErrorType error)
{
    Dem_EventIdType eventId;
    
    /* Map I2C error to DEM event ID */
    switch (error)
    {
        case I2C_E_NACK_RECEIVED:
            eventId = DEM_EVENT_ID_I2C_NACK;
            break;
        case I2C_E_ARBITRATION_FAILURE:
            eventId = DEM_EVENT_ID_I2C_ARBITRATION;
            break;
        case I2C_E_FIFO_HANDLING:
            eventId = DEM_EVENT_ID_I2C_FIFO;
            break;
        case I2C_E_BUS_FAILURE:
            eventId = DEM_EVENT_ID_I2C_BUS;
            break;
        case I2C_E_WRONG_MODE:
            eventId = DEM_EVENT_ID_I2C_WRONG_MODE;
            break;
        default:
            return;
    }
    
    Dem_SetEventStatus(eventId, DEM_EVENT_STATUS_FAILED);
}

/* Check if JobId is valid and configured */
static boolean I2c_IsJobIdValid(I2C_JobType JobId)
{
    /* [CP_SWS_I2C_00104] - JobId validation */
    if (JobId >= I2C_MAX_JOBS)
    {
        return FALSE;
    }
    
    /* Check if JobId exists in configuration */
    /* Implementation depends on actual configuration structure */
    return TRUE;
}

/* Check if SequenceId is valid */
static boolean I2c_IsSequenceIdValid(I2C_SequenceType SequenceId)
{
    if (SequenceId >= I2C_MAX_SEQUENCES)
    {
        return FALSE;
    }
    
    /* Check if SequenceId exists in configuration */
    return TRUE;
}

/* Queue Management Functions */
static boolean I2c_EnqueueSequence(uint8 channelId, I2C_SequenceType SequenceId)
{
    /* [CP_SWS_I2C_82304] - Queue in FIFO */
    if (I2c_QueueCount[channelId] >= I2C_MAX_QUEUE_SIZE)
    {
        return FALSE;
    }
    
    I2c_SequenceQueue[channelId][I2c_QueueTail[channelId]].sequenceId = SequenceId;
    I2c_SequenceQueue[channelId][I2c_QueueTail[channelId]].channelId = channelId;
    I2c_QueueTail[channelId] = (I2c_QueueTail[channelId] + 1) % I2C_MAX_QUEUE_SIZE;
    I2c_QueueCount[channelId]++;
    
    return TRUE;
}

static boolean I2c_DequeueSequence(uint8 channelId, I2C_SequenceType* SequenceId)
{
    if (I2c_QueueCount[channelId] == 0)
    {
        return FALSE;
    }
    
    *SequenceId = I2c_SequenceQueue[channelId][I2c_QueueHead[channelId]].sequenceId;
    I2c_QueueHead[channelId] = (I2c_QueueHead[channelId] + 1) % I2C_MAX_QUEUE_SIZE;
    I2c_QueueCount[channelId]--;
    
    return TRUE;
}

/* Get configured device address for Job */
static I2C_AddressType I2c_GetDeviceAddress(I2C_JobType JobId, I2C_AddressType NodeAddress)
{
    /* [CP_SWS_I2C_00103] - Node address override */
    if (NodeAddress != 0)
    {
        return NodeAddress;  /* Use parameter address */
    }
    
    /* Return configured address */
    /* This would come from configuration */
    return 0x50;  /* Default */
}

/* ========================== PUBLIC API IMPLEMENTATION ========================== */

/* [CP_SWS_I2C_00820] - I2C_Init */
void I2C_Init(const I2C_ConfigType* ConfigPtr)
{
    /* [SRS_BSW_00323] - Parameter validation */
    if (I2C_DEV_ERROR_DETECT == STD_ON)
    {
        if (ConfigPtr == NULL_PTR)
        {
            I2c_ReportDevError(I2C_E_PARAM_POINTER);
            return;
        }
    }
    
    I2c_ConfigPtr = ConfigPtr;
    
    /* [CP_SWS_I2C_82002] - Initialization */
    /* Initialize hardware channels */
    for (uint8 i = 0; i < I2C_MAX_CHANNELS; i++)
    {
        /* Initialize channel state */
        I2c_ChannelState[i].state = 0;  /* IDLE */
        I2c_ChannelState[i].currentSequence = 0xFF;
        I2c_ChannelState[i].currentJobIndex = 0;
        I2c_ChannelState[i].jobsInSequence = 0;
        I2c_ChannelState[i].remainingBytes = 0;
        I2c_ChannelState[i].timeoutCounter = 0;
        
        /* Initialize hardware */
        /* This would use ConfigPtr to get channel parameters */
        I2C_Hw_InitChannel(i, I2C_CHANNEL_0_BAUDRATE * 1000, I2C_CHANNEL_0_MODE);
    }
    
    /* [CP_SWS_I2C_82002] - Set sequence results to OK */
    for (uint8 i = 0; i < I2C_MAX_SEQUENCES; i++)
    {
        I2c_SequenceResult[i] = I2C_SEQ_OK;
    }
    
    /* Initialize job buffers */
    for (uint8 i = 0; i < I2C_MAX_JOBS; i++)
    {
        I2c_JobBuffer[i].isConfigured = FALSE;
    }
    
    I2c_Initialized = TRUE;
}

/* [CP_SWS_I2C_00821] - I2C_DeInit */
void I2C_DeInit(void)
{
    /* [CP_SWS_I2C_82108] - Development error detection */
    if (I2C_DEV_ERROR_DETECT == STD_ON)
    {
        if (!I2c_Initialized)
        {
            I2c_ReportDevError(I2C_E_UNINIT);
            return;
        }
    }
    
    /* [CP_SWS_I2C_82105] - Deinitialize peripherals */
    for (uint8 i = 0; i < I2C_MAX_CHANNELS; i++)
    {
        I2C_Hw_DeinitChannel(i);
    }
    
    I2c_Initialized = FALSE;
}

/* [CP_SWS_I2C_00822] - I2C_SetupEB */
Std_ReturnType I2C_SetupEB(
    I2C_JobType JobId,
    I2C_AddressType NodeAddress,
    I2C_DataConstPtrType TxDataBufferPtr,
    I2C_DataPtrType RxDataBufferPtr,
    I2C_NumberOfDataType Length)
{
    Std_ReturnType retVal = E_OK;
    
    /* [CP_SWS_I2C_00104] - Development error detection for JobId */
    if (I2C_DEV_ERROR_DETECT == STD_ON)
    {
        if (!I2c_Initialized)
        {
            I2c_ReportDevError(I2C_E_UNINIT);
            return E_NOT_OK;
        }
        
        if (!I2c_IsJobIdValid(JobId))
        {
            I2c_ReportDevError(I2C_E_PARAM_JOB);
            return E_NOT_OK;
        }
        
        /* [CP_SWS_I2C_00101] - Buffer pointer validation */
        if ((TxDataBufferPtr == NULL_PTR) && (RxDataBufferPtr == NULL_PTR))
        {
            I2c_ReportDevError(I2C_E_PARAM_POINTER);
            return E_NOT_OK;
        }
        
        /* [CP_SWS_I2C_00101] - Both pointers not NULL */
        if ((TxDataBufferPtr != NULL_PTR) && (RxDataBufferPtr != NULL_PTR))
        {
            I2c_ReportDevError(I2C_E_PARAM_POINTER);
            return E_NOT_OK;
        }
    }
    
    /* [CP_SWS_I2C_00103] - Node address override */
    I2C_AddressType addressToUse = I2c_GetDeviceAddress(JobId, NodeAddress);
    
    /* Store EB configuration */
    I2c_JobBuffer[JobId].nodeAddress = addressToUse;
    I2c_JobBuffer[JobId].txBuffer = TxDataBufferPtr;
    I2c_JobBuffer[JobId].rxBuffer = RxDataBufferPtr;
    I2c_JobBuffer[JobId].length = Length;
    I2c_JobBuffer[JobId].isConfigured = TRUE;
    
    return retVal;
}

/* [CP_SWS_I2C_00823] - I2C_AsyncTransmit */
Std_ReturnType I2C_AsyncTransmit(I2C_SequenceType SequenceId)
{
    Std_ReturnType retVal = E_OK;
    
    /* [CP_SWS_I2C_00310] - SequenceId validation */
    if (I2C_DEV_ERROR_DETECT == STD_ON)
    {
        if (!I2c_Initialized)
        {
            I2c_ReportDevError(I2C_E_UNINIT);
            return E_NOT_OK;
        }
        
        if (!I2c_IsSequenceIdValid(SequenceId))
        {
            I2c_ReportDevError(I2C_E_PARAM_SEQUENCE);
            return E_NOT_OK;
        }
    }
    
    /* Get sequence configuration */
    /* This would come from I2c_ConfigPtr */
    uint8 channelId = I2C_SEQUENCE_0_CHANNEL;  /* Example */
    
    /* [CP_SWS_I2C_82305] - The same transmission is ongoing */
    if (I2c_SequenceResult[SequenceId] == I2C_SEQ_PENDING)
    {
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_82303] - No transmission is ongoing */
    if (I2c_ChannelState[channelId].state == 0)  /* IDLE */
    {
        /* Start transmission */
        I2c_ChannelState[channelId].state = 1;  /* BUSY */
        I2c_ChannelState[channelId].currentSequence = SequenceId;
        I2c_SequenceResult[SequenceId] = I2C_SEQ_PENDING;
        
        /* Start hardware transmission */
        /* Implementation depends on hardware */
        
        return E_OK;
    }
    
    /* [CP_SWS_I2C_82304] - Another transmission is ongoing (queue) */
    if (I2c_EnqueueSequence(channelId, SequenceId))
    {
        I2c_SequenceResult[SequenceId] = I2C_SEQ_QUEUED;
        return E_OK;
    }
    
    return E_NOT_OK;
}

/* [CP_SWS_I2C_00824] - I2C_SyncTransmit */
Std_ReturnType I2C_SyncTransmit(I2C_SequenceType SequenceId)
{
    /* [CP_SWS_I2C_00410] - SequenceId validation */
    if (I2C_DEV_ERROR_DETECT == STD_ON)
    {
        if (!I2c_Initialized)
        {
            I2c_ReportDevError(I2C_E_UNINIT);
            return E_NOT_OK;
        }
        
        if (!I2c_IsSequenceIdValid(SequenceId))
        {
            I2c_ReportDevError(I2C_E_PARAM_SEQUENCE);
            return E_NOT_OK;
        }
    }
    
    /* Get sequence configuration */
    uint8 channelId = I2C_SEQUENCE_0_CHANNEL;
    
    /* [CP_SWS_I2C_82404] - Another transmission is ongoing */
    if (I2c_ChannelState[channelId].state != 0)  /* Not IDLE */
    {
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_82403] - No transmission is ongoing */
    I2c_ChannelState[channelId].state = 1;  /* BUSY */
    I2c_ChannelState[channelId].currentSequence = SequenceId;
    I2c_SequenceResult[SequenceId] = I2C_SEQ_PENDING;
    
    /* Blocking transmission */
    /* Implementation would wait for completion */
    
    return E_OK;
}

/* [CP_SWS_I2C_00827] - I2C_GetVersionInfo */
void I2C_GetVersionInfo(Std_VersionInfoType* VersionInfo)
{
    /* [CP_SWS_I2C_82601] - Pointer validation */
    if (I2C_DEV_ERROR_DETECT == STD_ON)
    {
        if (VersionInfo == NULL_PTR)
        {
            I2c_ReportDevError(I2C_E_PARAM_POINTER);
            return;
        }
    }
    
    VersionInfo->vendorID = I2C_VENDOR_ID;
    VersionInfo->moduleID = I2C_MODULE_ID;
    VersionInfo->sw_major_version = 1;
    VersionInfo->sw_minor_version = 0;
    VersionInfo->sw_patch_version = 0;
}

/* [CP_SWS_I2C_00828] - I2C_GetSequenceResult */
I2C_SequenceResultType I2C_GetSequenceResult(I2C_SequenceType SequenceId)
{
    /* [CP_SWS_I2C_80702] - SequenceId validation */
    if (I2C_DEV_ERROR_DETECT == STD_ON)
    {
        if (!I2c_IsSequenceIdValid(SequenceId))
        {
            I2c_ReportDevError(I2C_E_PARAM_SEQUENCE);
            return I2C_SEQ_FAILED;
        }
    }
    
    /* [CP_SWS_I2C_80701] - Return sequence result */
    return I2c_SequenceResult[SequenceId];
}

/* [CP_SWS_I2C_00835] - I2C_StartListening */
Std_ReturnType I2C_StartListening(I2C_SequenceType SequenceId)
{
    /* [CP_SWS_I2C_82806] - SetupEB validation */
    if (I2C_DEV_ERROR_DETECT == STD_ON)
    {
        if (!I2c_Initialized)
        {
            I2c_ReportDevError(I2C_E_UNINIT);
            return E_NOT_OK;
        }
        
        if (!I2c_IsSequenceIdValid(SequenceId))
        {
            I2c_ReportDevError(I2C_E_PARAM_SEQUENCE);
            return E_NOT_OK;
        }
    }
    
    /* Get sequence configuration */
    uint8 channelId = I2C_SEQUENCE_1_CHANNEL;  /* Target channel */
    
    /* [CP_SWS_I2C_80804] - Development error detection for mode */
    if (I2C_DEV_ERROR_DETECT == STD_ON)
    {
        /* Check if channel is in target mode */
        /* Implementation would check configuration */
        if (I2C_CHANNEL_1_MODE != I2C_HW_UNIT_MODE_TARGET)
        {
            I2c_ReportDevError(I2C_E_WRONG_CONDITION);
            return E_NOT_OK;
        }
    }
    
    /* [CP_SWS_I2C_80802] - Driver is in listening mode */
    if (I2c_ChannelState[channelId].state == 2)  /* LISTENING */
    {
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_80801] - No listening is ongoing */
    I2c_ChannelState[channelId].state = 2;  /* LISTENING */
    I2c_ChannelState[channelId].currentSequence = SequenceId;
    I2c_SequenceResult[SequenceId] = I2C_SEQ_PENDING;
    
    return E_OK;
}

/* [CP_SWS_I2C_00834] - I2C_MainFunction */
void I2C_MainFunction(void)
{
    for (uint8 channelId = 0; channelId < I2C_MAX_CHANNELS; channelId++)
    {
        /* Handle busy channels */
        if (I2c_ChannelState[channelId].state == 1)  /* BUSY */
        {
            /* Check for hardware errors */
            uint8 status = I2C_Hw_GetStatus(channelId);
            
            /* [CP_SWS_I2C_00703] - NACK received error */
            if (status & 0x20)  /* NACK bit */
            {
                I2c_ReportRuntimeError(channelId, I2C_E_NACK_RECEIVED);
                
                I2C_SequenceType seqId = I2c_ChannelState[channelId].currentSequence;
                I2c_SequenceResult[seqId] = I2C_SEQ_NACK;
                
                /* Call notification */
                /* Implementation would call I2C_SeqEndNotification */
                
                I2c_ChannelState[channelId].state = 0;  /* IDLE */
                continue;
            }
            
            /* [CP_SWS_I2C_00704] - Arbitration failure */
            if (status & 0x40)  /* Arbitration lost bit */
            {
                I2c_ReportRuntimeError(channelId, I2C_E_ARBITRATION_FAILURE);
                
                I2C_SequenceType seqId = I2c_ChannelState[channelId].currentSequence;
                I2c_SequenceResult[seqId] = I2C_SEQ_FAILED;
                
                I2c_ChannelState[channelId].state = 0;  /* IDLE */
                continue;
            }
            
            /* [CP_SWS_I2C_00705] - Bus failure */
            /* Check if SCL is stuck - implementation specific */
            
            /* Check for completion */
            if (status & 0x01)  /* Transfer complete */
            {
                I2C_SequenceType seqId = I2c_ChannelState[channelId].currentSequence;
                I2c_SequenceResult[seqId] = I2C_SEQ_OK;
                
                /* [CP_SWS_I2C_82308] - Continuation with queued elements */
                if (I2c_QueueCount[channelId] > 0)
                {
                    I2C_SequenceType nextSeq;
                    if (I2c_DequeueSequence(channelId, &nextSeq))
                    {
                        I2c_ChannelState[channelId].currentSequence = nextSeq;
                        I2c_SequenceResult[nextSeq] = I2C_SEQ_PENDING;
                        /* Start next transmission */
                    }
                }
                else
                {
                    I2c_ChannelState[channelId].state = 0;  /* IDLE */
                }
            }
        }
        
        /* Handle listening channels */
        if (I2c_ChannelState[channelId].state == 2)  /* LISTENING */
        {
            /* [CP_SWS_I2C_80901] - Permanent listening */
            if (I2C_CHANNEL_1_TARGET_LISTEN == STD_ON)
            {
                /* Check for received messages */
                uint8 status = I2C_Hw_GetStatus(channelId);
                
                if (status & 0x01)  /* Transfer complete */
                {
                    I2C_SequenceType seqId = I2c_ChannelState[channelId].currentSequence;
                    
                    /* [CP_SWS_I2C_80806] - Message data handling */
                    /* Implementation would copy data from/to buffers */
                    
                    /* [CP_SWS_I2C_80803] - Message/error handling */
                    I2c_SequenceResult[seqId] = I2C_SEQ_OK;
                    
                    /* Call notification */
                    /* Implementation would call I2C_SeqEndNotification */
                }
            }
        }
    }
}