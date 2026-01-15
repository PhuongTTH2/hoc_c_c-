#include "I2c.h"
#include "I2c_Cfg.h"
#include "I2c_SimplifiedHw.h"
#include "Det.h"
#include "Dem.h"

/* ================= CONTEXT STRUCTURES ================= */
typedef struct
{
    boolean IsConfigured;
    I2C_DataConstPtrType TxBuffer;
    I2C_DataPtrType RxBuffer;
    I2C_NumberOfDataType Length;
    I2C_NumberOfDataType CurrentIndex;
    I2C_AddressType DeviceAddress;
    boolean IsWriteOperation;
} I2C_SimplifiedJobContextType;

typedef struct
{
    I2C_SequenceResultType Result;
    uint8 AssignedJobs[I2C_MAX_JOBS_PER_SEQUENCE];
    uint8 NumJobs;
    uint8 CurrentJobIndex;
    boolean IsActive;
    boolean IsQueued;
    uint8 AssignedChannel;
} I2C_SimplifiedSequenceContextType;

typedef struct
{
    boolean IsInitialized;
    boolean IsTransmitting;
    uint32 BaudRate;
} I2C_SimplifiedChannelContextType;

/* ================= GLOBAL VARIABLES ================= */
static boolean I2C_Simplified_ModuleInitialized = FALSE;
static I2C_SimplifiedJobContextType I2C_JobContext[I2C_NUMBER_OF_JOBS];
static I2C_SimplifiedSequenceContextType I2C_SequenceContext[I2C_NUMBER_OF_SEQUENCES];
static I2C_SimplifiedChannelContextType I2C_ChannelContext[I2C_NUMBER_OF_CHANNELS];

/* Configuration từ file I2c_Cfg.c */
extern const uint8 I2C_SequenceJobMapping[I2C_NUMBER_OF_SEQUENCES][I2C_MAX_JOBS_PER_SEQUENCE];

/******************************************************************************
* Hàm: I2C_Simplified_Init
* Mô tả: Khởi tạo Simplified I2C Driver (AUTOSAR-compliant)
******************************************************************************/
void I2C_Simplified_Init(const I2C_ConfigType* ConfigPtr)
{
    uint8 i, j;
    
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR)
    {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, I2C_E_PARAM_POINTER);
        return;
    }
#endif
    
    /* Simplified I2C chỉ hỗ trợ Controller mode */
    for (i = 0; i < I2C_NUMBER_OF_CHANNELS; i++)
    {
        if (ConfigPtr[i].HwUnitMode != I2C_HW_UNIT_MODE_CONTROLLER)
        {
            Det_ReportError(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, I2C_E_WRONG_MODE);
            continue;
        }
        
        /* Khởi tạo phần cứng Simplified I2C */
        I2C_SimplifiedHw_Init(i, ConfigPtr[i].BaudRate);
        
        /* Lưu context */
        I2C_ChannelContext[i].IsInitialized = TRUE;
        I2C_ChannelContext[i].BaudRate = ConfigPtr[i].BaudRate;
        I2C_ChannelContext[i].IsTransmitting = FALSE;
    }
    
    /* Khởi tạo Job contexts */
    for (i = 0; i < I2C_NUMBER_OF_JOBS; i++)
    {
        I2C_JobContext[i].IsConfigured = FALSE;
        I2C_JobContext[i].TxBuffer = NULL_PTR;
        I2C_JobContext[i].RxBuffer = NULL_PTR;
        I2C_JobContext[i].Length = 0;
        I2C_JobContext[i].CurrentIndex = 0;
        I2C_JobContext[i].DeviceAddress = 0;
    }
    
    /* Khởi tạo Sequence contexts */
    for (i = 0; i < I2C_NUMBER_OF_SEQUENCES; i++)
    {
        I2C_SequenceContext[i].Result = I2C_SEQ_OK;
        I2C_SequenceContext[i].IsActive = FALSE;
        I2C_SequenceContext[i].IsQueued = FALSE;
        I2C_SequenceContext[i].CurrentJobIndex = 0;
        I2C_SequenceContext[i].NumJobs = 0;
        
        /* Copy job mapping từ configuration */
        for (j = 0; j < I2C_MAX_JOBS_PER_SEQUENCE; j++)
        {
            if (I2C_SequenceJobMapping[i][j] != 0xFF)
            {
                I2C_SequenceContext[i].AssignedJobs[j] = I2C_SequenceJobMapping[i][j];
                I2C_SequenceContext[i].NumJobs++;
            }
        }
    }
    
    I2C_Simplified_ModuleInitialized = TRUE;
}

/******************************************************************************
* Hàm: I2C_Simplified_SetupEB
* Mô tả: Cài đặt External Buffer cho Simplified I2C
* Note: Đơn giản hơn Full I2C vì không hỗ trợ Target mode
******************************************************************************/
Std_ReturnType I2C_Simplified_SetupEB(
    I2C_JobType JobId,
    I2C_AddressType NodeAddress,
    I2C_DataConstPtrType TxDataBufferPtr,
    I2C_DataPtrType RxDataBufferPtr,
    I2C_NumberOfDataType Length
)
{
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (JobId >= I2C_NUMBER_OF_JOBS)
    {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }
    
    /* Simplified I2C: buffer phải khác NULL và chỉ 1 trong 2 */
    if ((TxDataBufferPtr == NULL_PTR && RxDataBufferPtr == NULL_PTR) ||
        (TxDataBufferPtr != NULL_PTR && RxDataBufferPtr != NULL_PTR))
    {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, I2C_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#endif
    
    I2C_JobContext[JobId].TxBuffer = TxDataBufferPtr;
    I2C_JobContext[JobId].RxBuffer = RxDataBufferPtr;
    I2C_JobContext[JobId].Length = Length;
    I2C_JobContext[JobId].CurrentIndex = 0;
    I2C_JobContext[JobId].DeviceAddress = NodeAddress;
    I2C_JobContext[JobId].IsWriteOperation = (TxDataBufferPtr != NULL_PTR);
    I2C_JobContext[JobId].IsConfigured = TRUE;
    
    return E_OK;
}

/******************************************************************************
* Hàm: I2C_Simplified_SyncTransmit
* Mô tả: Truyền đồng bộ - ĐƠN GIẢN HÓA cho Simplified I2C
******************************************************************************/
Std_ReturnType I2C_Simplified_SyncTransmit(I2C_SequenceType SequenceId)
{
    I2C_SimplifiedSequenceContextType* SeqCtx;
    I2C_SimplifiedJobContextType* JobCtx;
    uint8 ChannelId, JobId;
    uint8 i;
    uint8 AckStatus;
    Std_ReturnType RetVal = E_OK;
    
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (!I2C_Simplified_ModuleInitialized)
    {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, I2C_E_UNINIT);
        return E_NOT_OK;
    }
    
    if (SequenceId >= I2C_NUMBER_OF_SEQUENCES)
    {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, I2C_E_PARAM_SEQUENCE);
        return E_NOT_OK;
    }
#endif
    
    SeqCtx = &I2C_SequenceContext[SequenceId];
    ChannelId = SeqCtx->AssignedChannel;
    
    /* Kiểm tra channel đã khởi tạo */
    if (!I2C_ChannelContext[ChannelId].IsInitialized)
    {
        return E_NOT_OK;
    }
    
    /* Kiểm tra đã cấu hình tất cả Jobs */
    for (i = 0; i < SeqCtx->NumJobs; i++)
    {
        JobId = SeqCtx->AssignedJobs[i];
        if (!I2C_JobContext[JobId].IsConfigured)
        {
            Det_ReportError(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, I2C_E_PARAM_SEQUENCE);
            return E_NOT_OK;
        }
    }
    
    SeqCtx->Result = I2C_SEQ_PENDING;
    SeqCtx->IsActive = TRUE;
    I2C_ChannelContext[ChannelId].IsTransmitting = TRUE;
    
    /* Thực hiện từng Job trong Sequence */
    for (i = 0; i < SeqCtx->NumJobs; i++)
    {
        JobId = SeqCtx->AssignedJobs[i];
        JobCtx = &I2C_JobContext[JobId];
        
        /* --- BẮT ĐẦU GIAO DỊCH --- */
        I2C_SimplifiedHw_ManualStart(ChannelId);
        
        /* Gửi địa chỉ slave + R/W bit */
        uint8 SlaveAddressByte = (JobCtx->DeviceAddress << 1);
        if (!JobCtx->IsWriteOperation)
        {
            SlaveAddressByte |= 0x01;  /* Read operation */
        }
        
        if (I2C_SimplifiedHw_SendByte(ChannelId, SlaveAddressByte, &AckStatus) != E_OK)
        {
            SeqCtx->Result = I2C_SEQ_FAILED;
            RetVal = E_NOT_OK;
            break;
        }
        
        if (AckStatus != 1)  /* NACK received */
        {
            SeqCtx->Result = I2C_SEQ_NACK;
            RetVal = E_NOT_OK;
            break;
        }
        
        /* Xử lý data transfer */
        if (JobCtx->IsWriteOperation)
        {
            /* Write operation: gửi data từ TxBuffer */
            for (uint16 j = 0; j < JobCtx->Length; j++)
            {
                if (I2C_SimplifiedHw_SendByte(ChannelId, JobCtx->TxBuffer[j], &AckStatus) != E_OK)
                {
                    SeqCtx->Result = I2C_SEQ_FAILED;
                    RetVal = E_NOT_OK;
                    break;
                }
                
                if (AckStatus != 1)
                {
                    SeqCtx->Result = I2C_SEQ_NACK;
                    RetVal = E_NOT_OK;
                    break;
                }
            }
        }
        else
        {
            /* Read operation: nhận data vào RxBuffer */
            for (uint16 j = 0; j < JobCtx->Length; j++)
            {
                uint8 ReceivedData;
                boolean SendAck = (j < (JobCtx->Length - 1));  /* Gửi ACK cho tất cả trừ byte cuối */
                
                if (I2C_SimplifiedHw_ReceiveByte(ChannelId, &ReceivedData, SendAck) != E_OK)
                {
                    SeqCtx->Result = I2C_SEQ_FAILED;
                    RetVal = E_NOT_OK;
                    break;
                }
                
                if (JobCtx->RxBuffer != NULL_PTR)
                {
                    JobCtx->RxBuffer[j] = ReceivedData;
                }
            }
        }
        
        if (RetVal != E_OK)
        {
            break;  /* Dừng nếu có lỗi */
        }
        
        /* Kết thúc giao dịch cho Job này */
        I2C_SimplifiedHw_ManualStop(ChannelId);
    }
    
    /* Hoàn thành Sequence */
    if (RetVal == E_OK)
    {
        SeqCtx->Result = I2C_SEQ_OK;
    }
    
    SeqCtx->IsActive = FALSE;
    I2C_ChannelContext[ChannelId].IsTransmitting = FALSE;
    
    return RetVal;
}

/******************************************************************************
* Hàm: I2C_Simplified_GetSequenceResult
* Mô tả: Lấy kết quả của Sequence
******************************************************************************/
I2C_SequenceResultType I2C_Simplified_GetSequenceResult(I2C_SequenceType SequenceId)
{
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (!I2C_Simplified_ModuleInitialized)
    {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, I2C_E_UNINIT);
        return I2C_SEQ_FAILED;
    }
    
    if (SequenceId >= I2C_NUMBER_OF_SEQUENCES)
    {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, I2C_E_PARAM_SEQUENCE);
        return I2C_SEQ_FAILED;
    }
#endif
    
    return I2C_SequenceContext[SequenceId].Result;
}

/******************************************************************************
* Hàm: I2C_Simplified_GetStatus
* Mô tả: Lấy trạng thái hardware
******************************************************************************/
uint32 I2C_Simplified_GetStatus(uint8 ChannelId)
{
    if (ChannelId >= I2C_NUMBER_OF_CHANNELS)
    {
        return 0;
    }
    
    return I2C_SimplifiedHw_GetStatus(ChannelId);
}