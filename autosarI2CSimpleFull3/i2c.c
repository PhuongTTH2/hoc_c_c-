/******************************************************************************
* AUTOSAR I2C Driver - Implementation
* File: I2c.c
* MCU: BAT32A2x9 with Simplified I2C (Chapter 19) and IICA (Chapter 20)
******************************************************************************/

#include "I2c.h"
#include "I2c_Internal.h"
#include "I2c_Cfg.h"
#include "Det.h"
#include "Dem.h"

/* ============ MODULE STATE ============ */
typedef enum {
    I2C_MODULE_UNINIT = 0U,
    I2C_MODULE_INIT
} I2C_ModuleStateType;

static I2C_ModuleStateType I2C_ModuleState = I2C_MODULE_UNINIT;

/* ============ CONFIGURATION ============ */
static const I2C_ConfigType* I2C_CurrentConfigPtr = NULL;

/* ============ SEQUENCE MANAGEMENT ============ */
typedef struct {
    I2C_SequenceResultType result;
    uint8 isPending;
    uint8 currentJobIndex;
    uint32 timeoutCounter;
    uint8 channelId;
} I2C_SequenceStateType;

static I2C_SequenceStateType I2C_SequenceStates[I2C_MAX_SEQUENCES];

/* ============ JOB BUFFER MANAGEMENT ============ */
typedef struct {
    I2C_JobType jobId;
    I2C_AddressType nodeAddress;
    I2C_DataConstPtrType txBuffer;
    I2C_DataPtrType rxBuffer;
    I2C_NumberOfDataType length;
    uint8 isConfigured;
} I2C_JobBufferType;

static I2C_JobBufferType I2C_JobBuffers[I2C_MAX_JOBS];

/* ============ QUEUE MANAGEMENT ============ */
#define I2C_QUEUE_SIZE 10U

typedef struct {
    I2C_SequenceType sequenceIds[I2C_QUEUE_SIZE];
    uint8 head;
    uint8 tail;
    uint8 count;
} I2C_QueueType;

static I2C_QueueType I2C_SequenceQueue = {0};

/* ============ PRIVATE FUNCTION DECLARATIONS ============ */
static Std_ReturnType I2C_ValidateJobId(I2C_JobType JobId);
static Std_ReturnType I2C_ValidateSequenceId(I2C_SequenceType SequenceId);
static uint8 I2C_GetSequenceChannel(I2C_SequenceType seqId);
static void I2C_QueueInit(I2C_QueueType* queue);
static Std_ReturnType I2C_QueueEnqueue(I2C_QueueType* queue, I2C_SequenceType sequenceId);
static Std_ReturnType I2C_QueueDequeue(I2C_QueueType* queue, I2C_SequenceType* sequenceId);
static void I2C_HwInitChannel(const I2C_ChannelConfigType* channelConfig);
static void I2C_StartHardwareTransfer(I2C_SequenceType SequenceId);
static void I2C_ProcessTransferComplete(uint8 channelId);
static void I2C_ProcessTransferError(uint8 channelId, uint8 error);
static I2C_SequenceType I2C_FindPendingSequence(uint8 channelId);
static void I2C_PollForCompletion(void);
static void I2C_ConfigureSimplifiedI2C(uint32 baudRate);
static void I2C_ConfigureIICA(uint32 baudRate, uint8 isSlave, uint8 slaveAddr);
static uint8 I2C_SimplifiedTransfer(uint8 slaveAddr, uint8* data, uint16 len, uint8 isRead);
static uint8 I2C_IicaTransfer(uint8 slaveAddr, uint8* data, uint16 len, uint8 isRead);

/* ============ PRIVATE FUNCTION IMPLEMENTATIONS ============ */

static Std_ReturnType I2C_ValidateJobId(I2C_JobType JobId)
{
    uint8 i;
    
    if (I2C_CurrentConfigPtr == NULL)
    {
        return E_NOT_OK;
    }
    
    for (i = 0U; i < I2C_CurrentConfigPtr->numJobs; i++)
    {
        if (I2C_CurrentConfigPtr->jobs[i].jobId == JobId)
        {
            return E_OK;
        }
    }
    
    if (I2C_CurrentConfigPtr->generalConfig.devErrorDetect == 1U)
    {
        Det_ReportError(I2C_MODULE_ID, I2C_INSTANCE_ID,
                       I2C_SERVICE_ID_SETUPEB, I2C_E_PARAM_JOB);
    }
    
    return E_NOT_OK;
}

static Std_ReturnType I2C_ValidateSequenceId(I2C_SequenceType SequenceId)
{
    uint8 i;
    
    if (I2C_CurrentConfigPtr == NULL)
    {
        return E_NOT_OK;
    }
    
    for (i = 0U; i < I2C_CurrentConfigPtr->numSequences; i++)
    {
        if (I2C_CurrentConfigPtr->sequences[i].sequenceId == SequenceId)
        {
            return E_OK;
        }
    }
    
    if (I2C_CurrentConfigPtr->generalConfig.devErrorDetect == 1U)
    {
        Det_ReportError(I2C_MODULE_ID, I2C_INSTANCE_ID,
                       I2C_SERVICE_ID_ASYNCTRANSMIT, I2C_E_PARAM_SEQUENCE);
    }
    
    return E_NOT_OK;
}

static uint8 I2C_GetSequenceChannel(I2C_SequenceType seqId)
{
    uint8 i;
    
    if (I2C_CurrentConfigPtr == NULL)
    {
        return 0xFFU;
    }
    
    for (i = 0U; i < I2C_CurrentConfigPtr->numSequences; i++)
    {
        if (I2C_CurrentConfigPtr->sequences[i].sequenceId == seqId)
        {
            return I2C_CurrentConfigPtr->sequences[i].assignedChannel;
        }
    }
    
    return 0xFFU;
}

static void I2C_QueueInit(I2C_QueueType* queue)
{
    uint8 i;
    
    queue->head = 0U;
    queue->tail = 0U;
    queue->count = 0U;
    
    for (i = 0U; i < I2C_QUEUE_SIZE; i++)
    {
        queue->sequenceIds[i] = 0xFFU;
    }
}

static Std_ReturnType I2C_QueueEnqueue(I2C_QueueType* queue, I2C_SequenceType sequenceId)
{
    if (queue->count >= I2C_QUEUE_SIZE)
    {
        return E_NOT_OK;
    }
    
    queue->sequenceIds[queue->tail] = sequenceId;
    queue->tail = (queue->tail + 1U) % I2C_QUEUE_SIZE;
    queue->count++;
    
    return E_OK;
}

static Std_ReturnType I2C_QueueDequeue(I2C_QueueType* queue, I2C_SequenceType* sequenceId)
{
    if (queue->count == 0U)
    {
        return E_NOT_OK;
    }
    
    *sequenceId = queue->sequenceIds[queue->head];
    queue->head = (queue->head + 1U) % I2C_QUEUE_SIZE;
    queue->count--;
    
    return E_OK;
}

static void I2C_ConfigureSimplifiedI2C(uint32 baudRate)
{
    /* Enable Simplified I2C clock (Chapter 19) */
    PER2_REG |= (1U << 5U);
    
    /* Configure pins P03 (SDA10) and P04 (SCL10) */
    PM00_REG |= ((1U << I2C_SIMPLE_SDA_PIN) | (1U << I2C_SIMPLE_SCL_PIN));
    PU00_REG |= ((1U << I2C_SIMPLE_SDA_PIN) | (1U << I2C_SIMPLE_SCL_PIN));
    POM00_REG |= ((1U << I2C_SIMPLE_SDA_PIN) | (1U << I2C_SIMPLE_SCL_PIN));
    
    /* Initialize Simplified I2C registers */
    I2C_SIMPLE_SMR00 = 0x38U;  /* I2C mode, master, clock synchronous */
    
    /* Calculate baud rate (assuming 16MHz system clock) */
    uint32 clock = 16000000U;  /* System clock */
    uint32 brg = (clock / (2U * baudRate)) - 1U;
    
    if (brg > 255U)
    {
        brg = 255U;
    }
    
    /* Set baud rate */
    I2C_SIMPLE_SCR00 = (uint8)brg;  /* BRG value */
}

static void I2C_ConfigureIICA(uint32 baudRate, uint8 isSlave, uint8 slaveAddr)
{
    /* Enable IICA clock (Chapter 20) */
    PER0_REG |= (1U << 10U);
    
    /* Configure pins P10 (SDA11) and P11 (SCL11) */
    PM10_REG |= ((1U << IICA_SDA_PIN) | (1U << IICA_SCL_PIN));
    PU10_REG |= ((1U << IICA_SDA_PIN) | (1U << IICA_SCL_PIN));
    POM10_REG |= ((1U << IICA_SDA_PIN) | (1U << IICA_SCL_PIN));
    
    if (isSlave)
    {
        /* Slave mode configuration */
        IICA0_IICCTL00 = 0x40U;  /* Slave mode, enable */
        IICA0_SVA0 = slaveAddr;  /* Set slave address */
    }
    else
    {
        /* Master mode configuration */
        IICA0_IICCTL00 = 0x00U;  /* Master mode, enable */
        
        /* Calculate timing values for baud rate */
        uint32 clock = 16000000U;  /* System clock */
        uint32 total = clock / baudRate;
        uint32 low = total / 2U;
        uint32 high = total - low - 6U;
        
        if (low > 255U) low = 255U;
        if (high > 255U) high = 255U;
    }
}

static void I2C_HwInitChannel(const I2C_ChannelConfigType* channelConfig)
{
    if (channelConfig->channelId == I2C_CHANNEL_SIMPLE_MASTER)
    {
        /* Simplified I2C (Master Only) */
        I2C_ConfigureSimplifiedI2C(channelConfig->baudRate);
    }
    else if (channelConfig->channelId == I2C_CHANNEL_IICA_MASTER_SLAVE)
    {
        /* IICA (Master/Slave) */
        uint8 isSlave = (channelConfig->hwUnitMode == I2C_HW_UNIT_MODE_TARGET) ? 1U : 0U;
        I2C_ConfigureIICA(channelConfig->baudRate, isSlave, I2C_SLAVE_ADDRESS);
    }
}

static uint8 I2C_SimplifiedTransfer(uint8 slaveAddr, uint8* data, uint16 len, uint8 isRead)
{
    uint8 status;
    
    /* Enable Simplified I2C */
    I2C_SIMPLE_SCR00 |= 0x01U;
    
    /* Wait for bus free */
    while ((I2C_SIMPLE_SSR00 & 0x80U) == 0U);
    
    /* Send START condition */
    I2C_SIMPLE_SMR00 |= 0x01U;
    
    /* Send slave address + R/W bit */
    uint8 addrByte = (slaveAddr << 1U) | (isRead ? 0x01U : 0x00U);
    I2C_SIMPLE_SDR00 = addrByte;
    
    /* Wait for transmission */
    while ((I2C_SIMPLE_SSR00 & 0x40U) == 0U);
    
    /* Check for NACK */
    status = I2C_SIMPLE_SSR00;
    if ((status & 0x10U) != 0U)
    {
        /* NACK received */
        I2C_SIMPLE_SMR00 |= 0x02U;  /* Send STOP */
        while ((I2C_SIMPLE_SSR00 & 0x04U) == 0U);
        return 0U;
    }
    
    /* Process data */
    if (isRead)
    {
        for (uint16 i = 0U; i < len; i++)
        {
            /* Read data */
            while ((I2C_SIMPLE_SSR00 & 0x20U) == 0U);
            data[i] = I2C_SIMPLE_SDR00;
        }
    }
    else
    {
        for (uint16 i = 0U; i < len; i++)
        {
            /* Write data */
            I2C_SIMPLE_SDR00 = data[i];
            while ((I2C_SIMPLE_SSR00 & 0x40U) == 0U);
            
            status = I2C_SIMPLE_SSR00;
            if ((status & 0x10U) != 0U)
            {
                /* NACK received */
                I2C_SIMPLE_SMR00 |= 0x02U;  /* Send STOP */
                while ((I2C_SIMPLE_SSR00 & 0x04U) == 0U);
                return 0U;
            }
        }
    }
    
    /* Send STOP condition */
    I2C_SIMPLE_SMR00 |= 0x02U;
    
    /* Wait for STOP to complete */
    while ((I2C_SIMPLE_SSR00 & 0x04U) == 0U);
    
    return 1U;
}

static uint8 I2C_IicaTransfer(uint8 slaveAddr, uint8* data, uint16 len, uint8 isRead)
{
    uint8 status;
    
    /* Send START */
    IICA0_IICCTL00 |= 0x20U;
    
    /* Wait for start condition sent */
    while ((IICA0_IICS0 & 0x08U) == 0U);
    
    /* Send slave address */
    uint8 addrByte = (slaveAddr << 1U) | (isRead ? 0x01U : 0x00U);
    IICA0_IICF0 = addrByte;
    
    /* Wait for transmission */
    while ((IICA0_IICS0 & 0x02U) == 0U);
    
    status = IICA0_IICS0;
    if ((status & 0x40U) == 0U)  /* Check NACK */
    {
        return 0U;
    }
    
    /* Data transfer */
    if (isRead)
    {
        for (uint16 i = 0U; i < len; i++)
        {
            if (i == (len - 1U))
            {
                /* Last byte - send NACK */
                IICA0_IICCTL00 |= 0x08U;
            }
            
            /* Read data */
            while ((IICA0_IICS0 & 0x01U) == 0U);
            data[i] = IICA0_IICF0;
        }
    }
    else
    {
        for (uint16 i = 0U; i < len; i++)
        {
            IICA0_IICF0 = data[i];
            while ((IICA0_IICS0 & 0x02U) == 0U);
            
            status = IICA0_IICS0;
            if ((status & 0x40U) == 0U)
            {
                return 0U;
            }
        }
    }
    
    /* Send STOP */
    IICA0_IICCTL00 |= 0x10U;
    
    return 1U;
}

static void I2C_StartHardwareTransfer(I2C_SequenceType SequenceId)
{
    const I2C_SequenceConfigType* seqConfig = NULL;
    const I2C_ChannelConfigType* channelConfig = NULL;
    uint8 i;
    
    /* Find sequence configuration */
    for (i = 0U; i < I2C_CurrentConfigPtr->numSequences; i++)
    {
        if (I2C_CurrentConfigPtr->sequences[i].sequenceId == SequenceId)
        {
            seqConfig = &I2C_CurrentConfigPtr->sequences[i];
            break;
        }
    }
    
    if (seqConfig == NULL)
    {
        return;
    }
    
    /* Find channel configuration */
    for (i = 0U; i < I2C_CurrentConfigPtr->numChannels; i++)
    {
        if (I2C_CurrentConfigPtr->channels[i].channelId == seqConfig->assignedChannel)
        {
            channelConfig = &I2C_CurrentConfigPtr->channels[i];
            break;
        }
    }
    
    if (channelConfig == NULL)
    {
        return;
    }
    
    /* Get first job */
    if (seqConfig->numAssignedJobs == 0U)
    {
        return;
    }
    
    I2C_JobType jobId = seqConfig->assignedJobs[0];
    
    /* Find job buffer */
    for (i = 0U; i < I2C_MAX_JOBS; i++)
    {
        if (I2C_JobBuffers[i].jobId == jobId && I2C_JobBuffers[i].isConfigured == 1U)
        {
            /* Determine operation type */
            uint8 isRead;
            uint8* data;
            uint16 length;
            
            if (I2C_JobBuffers[i].txBuffer != NULL && I2C_JobBuffers[i].rxBuffer == NULL)
            {
                isRead = 0U;
                data = (uint8*)I2C_JobBuffers[i].txBuffer;
                length = I2C_JobBuffers[i].length;
            }
            else if (I2C_JobBuffers[i].txBuffer == NULL && I2C_JobBuffers[i].rxBuffer != NULL)
            {
                isRead = 1U;
                data = I2C_JobBuffers[i].rxBuffer;
                length = I2C_JobBuffers[i].length;
            }
            else
            {
                return;
            }
            
            /* Start transfer based on channel type */
            uint8 result;
            uint8 slaveAddr = (uint8)(I2C_JobBuffers[i].nodeAddress & 0xFFU);
            
            if (channelConfig->channelId == I2C_CHANNEL_SIMPLE_MASTER)
            {
                result = I2C_SimplifiedTransfer(slaveAddr, data, length, isRead);
            }
            else if (channelConfig->channelId == I2C_CHANNEL_IICA_MASTER_SLAVE)
            {
                if (channelConfig->hwUnitMode == I2C_HW_UNIT_MODE_CONTROLLER)
                {
                    result = I2C_IicaTransfer(slaveAddr, data, length, isRead);
                }
                else
                {
                    /* Slave mode - already listening */
                    result = 1U;
                }
            }
            else
            {
                result = 0U;
            }
            
            /* Handle transfer result */
            if (result == 0U)
            {
                /* Transfer failed */
                I2C_SequenceStates[SequenceId].result = I2C_SEQUENCE_NACK;
                I2C_SequenceStates[SequenceId].isPending = 0U;
                
                /* Call notification callback */
                if (seqConfig->endNotification != NULL)
                {
                    seqConfig->endNotification(SequenceId, I2C_SEQUENCE_NACK);
                }
            }
            else
            {
                /* Transfer successful */
                I2C_SequenceStates[SequenceId].result = I2C_SEQUENCE_OK;
                I2C_SequenceStates[SequenceId].isPending = 0U;
                
                /* Call notification callback */
                if (seqConfig->endNotification != NULL)
                {
                    seqConfig->endNotification(SequenceId, I2C_SEQUENCE_OK);
                }
            }
            
            break;
        }
    }
}

static I2C_SequenceType I2C_FindPendingSequence(uint8 channelId)
{
    uint8 i;
    
    for (i = 0U; i < I2C_MAX_SEQUENCES; i++)
    {
        if (I2C_SequenceStates[i].isPending == 1U && 
            I2C_SequenceStates[i].channelId == channelId)
        {
            return i;
        }
    }
    
    return 0xFFU;
}

/* ============ API IMPLEMENTATIONS ============ */

void I2C_Init(const I2C_ConfigType* ConfigPtr)
{
    uint8 i;
    
    /* Check if already initialized */
    if (I2C_ModuleState == I2C_MODULE_INIT)
    {
        if (I2C_CurrentConfigPtr != NULL && 
            I2C_CurrentConfigPtr->generalConfig.devErrorDetect == 1U)
        {
            Det_ReportError(I2C_MODULE_ID, I2C_INSTANCE_ID,
                           I2C_SERVICE_ID_INIT, I2C_E_WRONG_CONDITION);
        }
        return;
    }
    
    /* Validate configuration pointer */
    if (ConfigPtr == NULL)
    {
        return;
    }
    
    /* Store configuration pointer */
    I2C_CurrentConfigPtr = ConfigPtr;
    
    /* Initialize sequence states */
    for (i = 0U; i < I2C_MAX_SEQUENCES; i++)
    {
        I2C_SequenceStates[i].result = I2C_SEQUENCE_OK;
        I2C_SequenceStates[i].isPending = 0U;
        I2C_SequenceStates[i].currentJobIndex = 0U;
        I2C_SequenceStates[i].timeoutCounter = 1000U;
        I2C_SequenceStates[i].channelId = 0xFFU;
    }
    
    /* Initialize job buffers */
    for (i = 0U; i < I2C_MAX_JOBS; i++)
    {
        I2C_JobBuffers[i].isConfigured = 0U;
    }
    
    /* Initialize queue */
    I2C_QueueInit(&I2C_SequenceQueue);
    
    /* Initialize hardware channels */
    for (i = 0U; i < ConfigPtr->numChannels; i++)
    {
        I2C_HwInitChannel(&ConfigPtr->channels[i]);
    }
    
    /* Set module state */
    I2C_ModuleState = I2C_MODULE_INIT;
}

void I2C_DeInit(void)
{
    /* Check module state */
    if (I2C_ModuleState == I2C_MODULE_UNINIT)
    {
        if (I2C_CurrentConfigPtr != NULL && 
            I2C_CurrentConfigPtr->generalConfig.devErrorDetect == 1U)
        {
            Det_ReportError(I2C_MODULE_ID, I2C_INSTANCE_ID,
                           I2C_SERVICE_ID_DEINIT, I2C_E_UNINIT);
        }
        return;
    }
    
    /* Disable Simplified I2C */
    I2C_SIMPLE_SCR00 = 0x00U;
    PER2_REG &= ~(1U << 5U);
    
    /* Disable IICA */
    IICA0_IICCTL00 = 0x00U;
    PER0_REG &= ~(1U << 10U);
    
    /* Reset module state */
    I2C_ModuleState = I2C_MODULE_UNINIT;
    I2C_CurrentConfigPtr = NULL;
}

Std_ReturnType I2C_SetupEB(
    I2C_JobType JobId,
    I2C_AddressType NodeAddress,
    I2C_DataConstPtrType TxDataBufferPtr,
    I2C_DataPtrType RxDataBufferPtr,
    I2C_NumberOfDataType Length
)
{
    uint8 i;
    
    /* Check module state */
    if (I2C_ModuleState == I2C_MODULE_UNINIT)
    {
        if (I2C_CurrentConfigPtr != NULL && 
            I2C_CurrentConfigPtr->generalConfig.devErrorDetect == 1U)
        {
            Det_ReportError(I2C_MODULE_ID, I2C_INSTANCE_ID,
                           I2C_SERVICE_ID_SETUPEB, I2C_E_UNINIT);
        }
        return E_NOT_OK;
    }
    
    /* Validate JobId */
    if (I2C_ValidateJobId(JobId) != E_OK)
    {
        return E_NOT_OK;
    }
    
    /* Check buffer pointers */
    if ((TxDataBufferPtr == NULL) && (RxDataBufferPtr == NULL))
    {
        if (I2C_CurrentConfigPtr->generalConfig.devErrorDetect == 1U)
        {
            Det_ReportError(I2C_MODULE_ID, I2C_INSTANCE_ID,
                           I2C_SERVICE_ID_SETUPEB, I2C_E_PARAM_POINTER);
        }
        return E_NOT_OK;
    }
    
    /* Find or configure job buffer */
    for (i = 0U; i < I2C_MAX_JOBS; i++)
    {
        if ((I2C_JobBuffers[i].isConfigured == 0U) || 
            (I2C_JobBuffers[i].jobId == JobId))
        {
            I2C_JobBuffers[i].jobId = JobId;
            I2C_JobBuffers[i].nodeAddress = NodeAddress;
            I2C_JobBuffers[i].txBuffer = TxDataBufferPtr;
            I2C_JobBuffers[i].rxBuffer = RxDataBufferPtr;
            I2C_JobBuffers[i].length = Length;
            I2C_JobBuffers[i].isConfigured = 1U;
            
            return E_OK;
        }
    }
    
    return E_NOT_OK;
}

Std_ReturnType I2C_AsyncTransmit(I2C_SequenceType SequenceId)
{
    const I2C_SequenceConfigType* seqConfig = NULL;
    uint8 i;
    
    /* Check module state */
    if (I2C_ModuleState == I2C_MODULE_UNINIT)
    {
        if (I2C_CurrentConfigPtr != NULL && 
            I2C_CurrentConfigPtr->generalConfig.devErrorDetect == 1U)
        {
            Det_ReportError(I2C_MODULE_ID, I2C_INSTANCE_ID,
                           I2C_SERVICE_ID_ASYNCTRANSMIT, I2C_E_UNINIT);
        }
        return E_NOT_OK;
    }
    
    /* Validate SequenceId */
    if (I2C_ValidateSequenceId(SequenceId) != E_OK)
    {
        return E_NOT_OK;
    }
    
    /* Find sequence configuration */
    for (i = 0U; i < I2C_CurrentConfigPtr->numSequences; i++)
    {
        if (I2C_CurrentConfigPtr->sequences[i].sequenceId == SequenceId)
        {
            seqConfig = &I2C_CurrentConfigPtr->sequences[i];
            break;
        }
    }
    
    if (seqConfig == NULL)
    {
        return E_NOT_OK;
    }
    
    /* Check if SetupEB was called for all jobs */
    for (i = 0U; i < seqConfig->numAssignedJobs; i++)
    {
        I2C_JobType jobId = seqConfig->assignedJobs[i];
        uint8 j;
        uint8 jobConfigured = 0U;
        
        for (j = 0U; j < I2C_MAX_JOBS; j++)
        {
            if ((I2C_JobBuffers[j].isConfigured == 1U) && 
                (I2C_JobBuffers[j].jobId == jobId))
            {
                jobConfigured = 1U;
                break;
            }
        }
        
        if (jobConfigured == 0U)
        {
            if (I2C_CurrentConfigPtr->generalConfig.devErrorDetect == 1U)
            {
                Det_ReportError(I2C_MODULE_ID, I2C_INSTANCE_ID,
                               I2C_SERVICE_ID_ASYNCTRANSMIT, I2C_E_PARAM_SEQUENCE);
            }
            return E_NOT_OK;
        }
    }
    
    /* Check if same sequence is already pending */
    if (I2C_SequenceStates[SequenceId].isPending == 1U)
    {
        return E_NOT_OK;
    }
    
    /* Check if any sequence is pending on same channel */
    for (i = 0U; i < I2C_MAX_SEQUENCES; i++)
    {
        if (I2C_SequenceStates[i].isPending == 1U)
        {
            uint8 channel1 = seqConfig->assignedChannel;
            uint8 channel2 = I2C_SequenceStates[i].channelId;
            
            if (channel1 == channel2)
            {
                /* Another transmission ongoing - queue it */
                if (I2C_QueueEnqueue(&I2C_SequenceQueue, SequenceId) == E_OK)
                {
                    I2C_SequenceStates[SequenceId].result = I2C_SEQUENCE_QUEUED;
                    return E_OK;
                }
                else
                {
                    return E_NOT_OK;
                }
            }
        }
    }
    
    /* Start transmission */
    I2C_SequenceStates[SequenceId].isPending = 1U;
    I2C_SequenceStates[SequenceId].result = I2C_SEQUENCE_PENDING;
    I2C_SequenceStates[SequenceId].currentJobIndex = 0U;
    I2C_SequenceStates[SequenceId].channelId = seqConfig->assignedChannel;
    
    /* Start hardware transfer (blocking for simplicity) */
    I2C_StartHardwareTransfer(SequenceId);
    
    return E_OK;
}

Std_ReturnType I2C_SyncTransmit(I2C_SequenceType SequenceId)
{
    /* Use async implementation - blocking in this simple version */
    return I2C_AsyncTransmit(SequenceId);
}

void I2C_GetVersionInfo(Std_VersionInfoType* versioninfo)
{
    /* Check module state */
    if (I2C_ModuleState == I2C_MODULE_UNINIT)
    {
        if (I2C_CurrentConfigPtr != NULL && 
            I2C_CurrentConfigPtr->generalConfig.devErrorDetect == 1U)
        {
            Det_ReportError(I2C_MODULE_ID, I2C_INSTANCE_ID,
                           I2C_SERVICE_ID_GETVERSIONINFO, I2C_E_UNINIT);
        }
        return;
    }
    
    if (versioninfo == NULL)
    {
        if (I2C_CurrentConfigPtr->generalConfig.devErrorDetect == 1U)
        {
            Det_ReportError(I2C_MODULE_ID, I2C_INSTANCE_ID,
                           I2C_SERVICE_ID_GETVERSIONINFO, I2C_E_PARAM_POINTER);
        }
        return;
    }
    
    if (I2C_CurrentConfigPtr->generalConfig.versionInfoApi == 0U)
    {
        return;
    }
    
    versioninfo->vendorID = I2C_VENDOR_ID;
    versioninfo->moduleID = I2C_MODULE_ID;
    versioninfo->sw_major_version = I2C_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = I2C_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = I2C_SW_PATCH_VERSION;
}

I2C_SequenceResultType I2C_GetSequenceResult(I2C_SequenceType SequenceId)
{
    /* Check module state */
    if (I2C_ModuleState == I2C_MODULE_UNINIT)
    {
        if (I2C_CurrentConfigPtr != NULL && 
            I2C_CurrentConfigPtr->generalConfig.devErrorDetect == 1U)
        {
            Det_ReportError(I2C_MODULE_ID, I2C_INSTANCE_ID,
                           I2C_SERVICE_ID_GETSEQUENCERESULT, I2C_E_UNINIT);
        }
        return I2C_SEQUENCE_FAILED;
    }
    
    /* Validate SequenceId */
    if (I2C_ValidateSequenceId(SequenceId) != E_OK)
    {
        return I2C_SEQUENCE_FAILED;
    }
    
    return I2C_SequenceStates[SequenceId].result;
}

Std_ReturnType I2C_StartListening(I2C_SequenceType SequenceId)
{
    const I2C_SequenceConfigType* seqConfig = NULL;
    const I2C_ChannelConfigType* channelConfig = NULL;
    uint8 i;
    
    /* Check module state */
    if (I2C_ModuleState == I2C_MODULE_UNINIT)
    {
        if (I2C_CurrentConfigPtr != NULL && 
            I2C_CurrentConfigPtr->generalConfig.devErrorDetect == 1U)
        {
            Det_ReportError(I2C_MODULE_ID, I2C_INSTANCE_ID,
                           I2C_SERVICE_ID_STARTLISTENING, I2C_E_UNINIT);
        }
        return E_NOT_OK;
    }
    
    /* Validate SequenceId */
    if (I2C_ValidateSequenceId(SequenceId) != E_OK)
    {
        return E_NOT_OK;
    }
    
    /* Find sequence and channel configuration */
    for (i = 0U; i < I2C_CurrentConfigPtr->numSequences; i++)
    {
        if (I2C_CurrentConfigPtr->sequences[i].sequenceId == SequenceId)
        {
            seqConfig = &I2C_CurrentConfigPtr->sequences[i];
            
            /* Find channel config */
            uint8 chanId = seqConfig->assignedChannel;
            for (uint8 j = 0U; j < I2C_CurrentConfigPtr->numChannels; j++)
            {
                if (I2C_CurrentConfigPtr->channels[j].channelId == chanId)
                {
                    channelConfig = &I2C_CurrentConfigPtr->channels[j];
                    break;
                }
            }
            break;
        }
    }
    
    if ((seqConfig == NULL) || (channelConfig == NULL))
    {
        return E_NOT_OK;
    }
    
    /* Check if in Target mode */
    if (channelConfig->hwUnitMode != I2C_HW_UNIT_MODE_TARGET)
    {
        if (I2C_CurrentConfigPtr->generalConfig.devErrorDetect == 1U)
        {
            Det_ReportError(I2C_MODULE_ID, I2C_INSTANCE_ID,
                           I2C_SERVICE_ID_STARTLISTENING, I2C_E_WRONG_CONDITION);
        }
        return E_NOT_OK;
    }
    
    /* Check if already listening */
    if (I2C_SequenceStates[SequenceId].isPending == 1U)
    {
        return E_NOT_OK;
    }
    
    /* Configure IICA for slave mode listening */
    if (channelConfig->channelId == I2C_CHANNEL_IICA_MASTER_SLAVE)
    {
        /* Enable slave mode */
        IICA0_IICCTL00 = 0x40U;  /* Slave mode */
        IICA0_SVA0 = I2C_SLAVE_ADDRESS;
        
        /* Enable interrupts for slave mode */
        IICA0_IICCTL10 |= 0x80U;
    }
    
    /* Start listening */
    I2C_SequenceStates[SequenceId].isPending = 1U;
    I2C_SequenceStates[SequenceId].result = I2C_SEQUENCE_PENDING;
    
    return E_OK;
}

void I2C_MainFunction(void)
{
    uint8 i;
    I2C_SequenceType nextSequence;
    
    if (I2C_ModuleState != I2C_MODULE_INIT)
    {
        return;
    }
    
    /* Process timeout counters */
    for (i = 0U; i < I2C_MAX_SEQUENCES; i++)
    {
        if (I2C_SequenceStates[i].isPending == 1U)
        {
            if (I2C_SequenceStates[i].timeoutCounter > 0U)
            {
                I2C_SequenceStates[i].timeoutCounter--;
                
                if (I2C_SequenceStates[i].timeoutCounter == 0U)
                {
                    /* Timeout occurred */
                    I2C_SequenceStates[i].result = I2C_SEQUENCE_FAILED;
                    I2C_SequenceStates[i].isPending = 0U;
                    
                    /* Report to DEM */
                    Dem_SetEventStatus(I2C_EVENT_ID_TIMEOUT, DEM_EVENT_STATUS_FAILED);
                }
            }
        }
    }
    
    /* Check for queued sequences */
    if (I2C_SequenceQueue.count > 0U)
    {
        /* Check if any channel is free for the next queued sequence */
        if (I2C_QueueDequeue(&I2C_SequenceQueue, &nextSequence) == E_OK)
        {
            /* Start the queued sequence */
            (void)I2C_AsyncTransmit(nextSequence);
        }
    }
}