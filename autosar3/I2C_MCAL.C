/******************************************************************************
* File: I2c.c
* Description: Triển khai chính I2C Driver theo AUTOSAR - ĐẦY ĐỦ REQUIREMENTS
******************************************************************************/

#include "I2c.h"
#include "I2c_Cfg.h"
#include "I2c_Hw.h"
#include "Det.h"
#include "Dem.h"
#include "Rte_I2c.h"

/* Cấu trúc Job Context */
typedef struct
{
    boolean IsConfigured;
    I2C_DataConstPtrType TxBuffer;
    I2C_DataPtrType RxBuffer;
    I2C_NumberOfDataType Length;
    I2C_NumberOfDataType CurrentIndex;
    I2C_AddressType NodeAddress;
    uint8 AssignedChannel;
    boolean IsWriteOperation; /* TRUE = write, FALSE = read */
} I2C_JobContextType;

/* Cấu trúc Sequence Context */
typedef struct
{
    I2C_SequenceResultType Result;
    uint8* AssignedJobs; /* Mảng Job IDs */
    uint8 NumJobs;
    uint8 CurrentJobIndex;
    boolean IsActive;
    boolean IsQueued;
    uint8 AssignedChannel;
} I2C_SequenceContextType;

/* Cấu trúc FIFO Queue */
typedef struct
{
    I2C_SequenceType Queue[I2C_FIFO_QUEUE_SIZE];
    uint8 Front;
    uint8 Rear;
    uint8 Count;
} I2C_FifoQueueType;

/* Cấu trúc Channel Context */
typedef struct
{
    boolean IsInitialized;
    boolean IsTransmitting;
    boolean IsListening;
    I2C_HwUnitType HwUnitMode;
    boolean TargetListeningEnabled;
} I2C_ChannelContextType;

/* Biến toàn cục */
static boolean I2C_ModuleInitialized = FALSE;
static I2C_JobContextType I2C_JobContext[I2C_NUM_OF_JOBS];
static I2C_SequenceContextType I2C_SequenceContext[I2C_NUM_OF_SEQUENCES];
static I2C_FifoQueueType I2C_FifoQueues[I2C_NUM_OF_CHANNELS];
static I2C_ChannelContextType I2C_ChannelContext[I2C_NUM_OF_CHANNELS];

/* Cấu hình tĩnh (từ file cấu hình) */
extern const I2C_ConfigType I2C_ConfigSet[];
extern const uint8 I2C_SequenceJobMapping[I2C_NUM_OF_SEQUENCES][I2C_MAX_JOBS_PER_SEQUENCE];

/* Prototype các hàm nội bộ */
static Std_ReturnType I2C_ValidateJobId(I2C_JobType JobId);
static Std_ReturnType I2C_ValidateSequenceId(I2C_SequenceType SequenceId);
static Std_ReturnType I2C_ValidateChannelId(uint8 ChannelId);
static void I2C_ReportDevError(uint8 ErrorCode);
static void I2C_ReportRuntimeError(uint8 ErrorCode);
static void I2C_FifoEnqueue(uint8 ChannelId, I2C_SequenceType SequenceId);
static I2C_SequenceType I2C_FifoDequeue(uint8 ChannelId);
static boolean I2C_FifoIsEmpty(uint8 ChannelId);
static boolean I2C_FifoIsFull(uint8 ChannelId);
static void I2C_ProcessNextJob(uint8 ChannelId);
static void I2C_HandleSequenceCompletion(uint8 ChannelId, I2C_SequenceType SequenceId, I2C_SequenceResultType Result);
static void I2C_HandleTargetModeData(uint8 ChannelId);

/* Biến lưu trữ callback function */
static I2C_SeqEndNotificationType I2C_SeqEndNotification = NULL_PTR;

/******************************************************************************
* Hàm: I2C_Init
* Mô tả: Khởi tạo I2C Driver - [CP_SWS_I2C_82002]
******************************************************************************/
void I2C_Init(const I2C_ConfigType* ConfigPtr)
{
    uint8 i, j;
    
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR)
    {
        I2C_ReportDevError(I2C_E_PARAM_POINTER);
        return;
    }
#endif
    
    /* [CP_SWS_I2C_82002] - Khởi tạo I2C hardware cho mỗi I2cChannel */
    for (i = 0; i < I2C_NUM_OF_CHANNELS; i++)
    {
        /* Khởi tạo phần cứng cho mỗi channel */
        I2C_Hw_InitChannel(i, &ConfigPtr[i]);
        
        /* Lưu context channel */
        I2C_ChannelContext[i].IsInitialized = TRUE;
        I2C_ChannelContext[i].HwUnitMode = ConfigPtr[i].HwUnitMode;
        I2C_ChannelContext[i].TargetListeningEnabled = ConfigPtr[i].TargetListening;
        
        /* Khởi tạo hàng đợi FIFO */
        I2C_FifoQueues[i].Front = 0;
        I2C_FifoQueues[i].Rear = 0;
        I2C_FifoQueues[i].Count = 0;
    }
    
    /* Khởi tạo context Jobs */
    for (i = 0; i < I2C_NUM_OF_JOBS; i++)
    {
        I2C_JobContext[i].IsConfigured = FALSE;
        I2C_JobContext[i].TxBuffer = NULL_PTR;
        I2C_JobContext[i].RxBuffer = NULL_PTR;
        I2C_JobContext[i].Length = 0;
        I2C_JobContext[i].CurrentIndex = 0;
        I2C_JobContext[i].NodeAddress = 0;
    }
    
    /* [CP_SWS_I2C_82002] - Set sequence result to I2C_SEQ_OK cho mỗi I2cSequence */
    for (i = 0; i < I2C_NUM_OF_SEQUENCES; i++)
    {
        I2C_SequenceContext[i].Result = I2C_SEQ_OK;
        I2C_SequenceContext[i].IsActive = FALSE;
        I2C_SequenceContext[i].IsQueued = FALSE;
        I2C_SequenceContext[i].CurrentJobIndex = 0;
        
        /* Gán Jobs cho Sequence từ cấu hình */
        for (j = 0; j < I2C_MAX_JOBS_PER_SEQUENCE; j++)
        {
            if (I2C_SequenceJobMapping[i][j] != 0xFF)
            {
                I2C_SequenceContext[i].AssignedJobs[j] = I2C_SequenceJobMapping[i][j];
            }
        }
    }
    
    I2C_ModuleInitialized = TRUE;
    
    Det_ReportInformation(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, 
                         "I2C Driver initialized with %d channels, %d jobs, %d sequences",
                         I2C_NUM_OF_CHANNELS, I2C_NUM_OF_JOBS, I2C_NUM_OF_SEQUENCES);
}

/******************************************************************************
* Hàm: I2C_DeInit
* Mô tả: Hủy khởi tạo I2C Driver
******************************************************************************/
void I2C_DeInit(void)
{
    uint8 i;
    
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    /* [CP_SWS_I2C_82108] - Phát hiện lỗi nếu driver chưa khởi tạo */
    if (!I2C_ModuleInitialized)
    {
        I2C_ReportDevError(I2C_E_UNINIT);
        return;
    }
#endif
    
    /* [CP_SWS_I2C_82105] - Hủy khởi tạo I2C peripheral về trạng thái Power On Reset */
    for (i = 0; i < I2C_NUM_OF_CHANNELS; i++)
    {
        I2C_Hw_DeinitChannel(i);
        I2C_ChannelContext[i].IsInitialized = FALSE;
        I2C_ChannelContext[i].IsTransmitting = FALSE;
        I2C_ChannelContext[i].IsListening = FALSE;
    }
    
    I2C_ModuleInitialized = FALSE;
    
    Det_ReportInformation(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, 
                         "I2C Driver deinitialized");
}

/******************************************************************************
* Hàm: I2C_SetupEB
* Mô tả: Cài đặt buffer ngoài cho Job
******************************************************************************/
Std_ReturnType I2C_SetupEB(
    I2C_JobType JobId,
    I2C_AddressType NodeAddress,
    I2C_DataConstPtrType TxDataBufferPtr,
    I2C_DataPtrType RxDataBufferPtr,
    I2C_NumberOfDataType Length
)
{
    I2C_JobContextType* JobCtx;
    
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    /* [CP_SWS_I2C_00104] - Kiểm tra JobId hợp lệ */
    if (I2C_ValidateJobId(JobId) != E_OK)
    {
        I2C_ReportDevError(I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_00101] - Kiểm tra con trỏ buffer */
    if ((TxDataBufferPtr == NULL_PTR && RxDataBufferPtr == NULL_PTR) ||
        (TxDataBufferPtr != NULL_PTR && RxDataBufferPtr != NULL_PTR))
    {
        I2C_ReportDevError(I2C_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#endif
    
    JobCtx = &I2C_JobContext[JobId];
    
    /* Cập nhật Job context */
    JobCtx->TxBuffer = TxDataBufferPtr;
    JobCtx->RxBuffer = RxDataBufferPtr;
    JobCtx->Length = Length;
    JobCtx->CurrentIndex = 0;
    
    /* [CP_SWS_I2C_00103] - Ghi đè địa chỉ nếu khác 0 */
    if (NodeAddress != 0)
    {
        JobCtx->NodeAddress = NodeAddress;
    }
    
    /* [CP_SWS_I2C_00102] và [CP_SWS_I2C_00105], [CP_SWS_I2C_00106] - Xác định loại operation */
    if (TxDataBufferPtr == NULL_PTR)
    {
        JobCtx->IsWriteOperation = FALSE; /* Read operation */
    }
    else if (RxDataBufferPtr == NULL_PTR)
    {
        JobCtx->IsWriteOperation = TRUE; /* Write operation */
    }
    
    JobCtx->IsConfigured = TRUE;
    
    return E_OK;
}

/******************************************************************************
* Hàm: I2C_AsyncTransmit
* Mô tả: Truyền dữ liệu bất đồng bộ
******************************************************************************/
Std_ReturnType I2C_AsyncTransmit(I2C_SequenceType SequenceId)
{
    I2C_SequenceContextType* SeqCtx;
    uint8 ChannelId;
    
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (!I2C_ModuleInitialized)
    {
        I2C_ReportDevError(I2C_E_UNINIT);
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_00310] - Kiểm tra SequenceId hợp lệ */
    if (I2C_ValidateSequenceId(SequenceId) != E_OK)
    {
        I2C_ReportDevError(I2C_E_PARAM_SEQUENCE);
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_82309] - Kiểm tra I2C_SetupEB đã được gọi cho tất cả Jobs */
    SeqCtx = &I2C_SequenceContext[SequenceId];
    for (uint8 i = 0; i < SeqCtx->NumJobs; i++)
    {
        uint8 JobId = SeqCtx->AssignedJobs[i];
        if (!I2C_JobContext[JobId].IsConfigured)
        {
            I2C_ReportDevError(I2C_E_PARAM_SEQUENCE);
            return E_NOT_OK;
        }
    }
#endif
    
    SeqCtx = &I2C_SequenceContext[SequenceId];
    ChannelId = SeqCtx->AssignedChannel;
    
    /* [CP_SWS_I2C_82305] - Kiểm tra nếu Sequence đã đang chạy */
    if (SeqCtx->IsActive)
    {
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_82303] - Không có truyền nào đang diễn ra trên cùng channel */
    if (!I2C_ChannelContext[ChannelId].IsTransmitting && I2C_FifoIsEmpty(ChannelId))
    {
        /* Bắt đầu truyền ngay lập tức */
        SeqCtx->Result = I2C_SEQ_PENDING;
        SeqCtx->IsActive = TRUE;
        SeqCtx->CurrentJobIndex = 0;
        I2C_ChannelContext[ChannelId].IsTransmitting = TRUE;
        
        /* Bắt đầu truyền Job đầu tiên */
        I2C_ProcessNextJob(ChannelId);
        
        return E_OK;
    }
    
    /* [CP_SWS_I2C_82304] - Có truyền khác đang diễn ra - thêm vào hàng đợi */
    if (!I2C_FifoIsFull(ChannelId))
    {
        I2C_FifoEnqueue(ChannelId, SequenceId);
        SeqCtx->Result = I2C_SEQ_QUEUED;
        SeqCtx->IsQueued = TRUE;
        return E_OK;
    }
    
    /* Hàng đợi đầy */
    return E_NOT_OK;
}

/******************************************************************************
* Hàm: I2C_SyncTransmit
* Mô tả: Truyền dữ liệu đồng bộ
******************************************************************************/
Std_ReturnType I2C_SyncTransmit(I2C_SequenceType SequenceId)
{
    I2C_SequenceContextType* SeqCtx;
    uint8 ChannelId;
    uint8 i, j;
    
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (!I2C_ModuleInitialized)
    {
        I2C_ReportDevError(I2C_E_UNINIT);
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_00410] - Kiểm tra SequenceId hợp lệ */
    if (I2C_ValidateSequenceId(SequenceId) != E_OK)
    {
        I2C_ReportDevError(I2C_E_PARAM_SEQUENCE);
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_82409] - Kiểm tra I2C_SetupEB đã được gọi cho tất cả Jobs */
    SeqCtx = &I2C_SequenceContext[SequenceId];
    for (i = 0; i < SeqCtx->NumJobs; i++)
    {
        uint8 JobId = SeqCtx->AssignedJobs[i];
        if (!I2C_JobContext[JobId].IsConfigured)
        {
            I2C_ReportDevError(I2C_E_PARAM_SEQUENCE);
            return E_NOT_OK;
        }
    }
#endif
    
    SeqCtx = &I2C_SequenceContext[SequenceId];
    ChannelId = SeqCtx->AssignedChannel;
    
    /* [CP_SWS_I2C_82404] - Kiểm tra có truyền bất đồng bộ đang diễn ra */
    for (i = 0; i < I2C_NUM_OF_SEQUENCES; i++)
    {
        if (i != SequenceId && 
            I2C_SequenceContext[i].IsActive &&
            I2C_SequenceContext[i].AssignedChannel == ChannelId)
        {
            return E_NOT_OK;
        }
    }
    
    /* [CP_SWS_I2C_82403] - Bắt đầu truyền đồng bộ */
    SeqCtx->Result = I2C_SEQ_PENDING;
    SeqCtx->IsActive = TRUE;
    I2C_ChannelContext[ChannelId].IsTransmitting = TRUE;
    
    /* [CP_SWS_I2C_82407] - Thực hiện tất cả Jobs trong Sequence */
    for (i = 0; i < SeqCtx->NumJobs; i++)
    {
        uint8 JobId = SeqCtx->AssignedJobs[i];
        I2C_JobContextType* JobCtx = &I2C_JobContext[JobId];
        
        /* Bắt đầu truyền cho Job này */
        I2C_Hw_StartTransmit(ChannelId, JobCtx->NodeAddress);
        
        /* Chờ truyền hoàn thành (polling) */
        boolean TransferComplete = FALSE;
        uint32 Timeout = 100000; /* Timeout counter */
        
        while (!TransferComplete && Timeout > 0)
        {
            uint32 Status = I2C_Hw_GetStatus(ChannelId);
            
            /* [CP_SWS_I2C_00703] - Kiểm tra NACK */
            if (Status & IICS_NACK_RECEIVED)
            {
                SeqCtx->Result = I2C_SEQ_NACK;
                I2C_ReportRuntimeError(I2C_E_NACK_RECEIVED);
                break;
            }
            
            /* [CP_SWS_I2C_00704] - Kiểm tra Arbitration lost */
            if (Status & IICS_ARBITRATION_LOST)
            {
                SeqCtx->Result = I2C_SEQ_FAILED;
                I2C_ReportRuntimeError(I2C_E_ARBITRATION_FAILURE);
                break;
            }
            
            /* [CP_SWS_I2C_00705] - Kiểm tra Bus failure */
            if (Status & IICS_BUS_ERROR)
            {
                SeqCtx->Result = I2C_SEQ_FAILED;
                I2C_ReportRuntimeError(I2C_E_BUS_FAILURE);
                break;
            }
            
            /* Kiểm tra transfer complete */
            if (Status & IICS_TRANSFER_COMPLETE)
            {
                TransferComplete = TRUE;
                I2C_Hw_ClearStatus(ChannelId, IICS_TRANSFER_COMPLETE);
            }
            
            /* Xử lý dữ liệu nếu cần */
            if (JobCtx->IsWriteOperation)
            {
                /* Write operation: ghi dữ liệu từ TxBuffer */
                for (j = 0; j < JobCtx->Length; j++)
                {
                    I2C_Hw_WriteData(ChannelId, JobCtx->TxBuffer[j]);
                    
                    /* Chờ TX buffer empty */
                    while (!(I2C_Hw_GetStatus(ChannelId) & IICS_TX_EMPTY) && Timeout > 0)
                    {
                        Timeout--;
                    }
                    
                    if (Timeout == 0)
                    {
                        SeqCtx->Result = I2C_SEQ_FAILED;
                        break;
                    }
                }
            }
            else
            {
                /* Read operation: đọc dữ liệu vào RxBuffer */
                for (j = 0; j < JobCtx->Length; j++)
                {
                    /* Chờ RX data ready */
                    while (!(I2C_Hw_GetStatus(ChannelId) & IICS_RX_DATA_READY) && Timeout > 0)
                    {
                        Timeout--;
                    }
                    
                    if (Timeout == 0)
                    {
                        SeqCtx->Result = I2C_SEQ_FAILED;
                        break;
                    }
                    
                    JobCtx->RxBuffer[j] = I2C_Hw_ReadData(ChannelId);
                }
            }
            
            Timeout--;
        }
        
        if (Timeout == 0 || SeqCtx->Result != I2C_SEQ_PENDING)
        {
            break; /* Timeout hoặc lỗi */
        }
    }
    
    /* Hoàn thành Sequence */
    if (SeqCtx->Result == I2C_SEQ_PENDING)
    {
        SeqCtx->Result = I2C_SEQ_OK;
    }
    
    SeqCtx->IsActive = FALSE;
    I2C_ChannelContext[ChannelId].IsTransmitting = FALSE;
    
    return (SeqCtx->Result == I2C_SEQ_OK) ? E_OK : E_NOT_OK;
}

/******************************************************************************
* Hàm: I2C_GetSequenceResult
* Mô tả: Lấy kết quả Sequence
******************************************************************************/
I2C_SequenceResultType I2C_GetSequenceResult(I2C_SequenceType SequenceId)
{
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (!I2C_ModuleInitialized)
    {
        I2C_ReportDevError(I2C_E_UNINIT);
        return I2C_SEQ_FAILED;
    }
    
    /* [CP_SWS_I2C_80702] - Kiểm tra SequenceId hợp lệ */
    if (I2C_ValidateSequenceId(SequenceId) != E_OK)
    {
        I2C_ReportDevError(I2C_E_PARAM_SEQUENCE);
        return I2C_SEQ_FAILED;
    }
#endif
    
    /* [CP_SWS_I2C_80701] - Trả về kết quả Sequence */
    return I2C_SequenceContext[SequenceId].Result;
}

/******************************************************************************
* Hàm: I2C_StartListening
* Mô tả: Bắt đầu chế độ nghe (Target mode)
******************************************************************************/
Std_ReturnType I2C_StartListening(I2C_SequenceType SequenceId)
{
    I2C_SequenceContextType* SeqCtx;
    uint8 ChannelId;
    
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    if (!I2C_ModuleInitialized)
    {
        I2C_ReportDevError(I2C_E_UNINIT);
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_82806] - Kiểm tra I2C_SetupEB đã được gọi */
    SeqCtx = &I2C_SequenceContext[SequenceId];
    for (uint8 i = 0; i < SeqCtx->NumJobs; i++)
    {
        uint8 JobId = SeqCtx->AssignedJobs[i];
        if (!I2C_JobContext[JobId].IsConfigured)
        {
            I2C_ReportDevError(I2C_E_PARAM_SEQUENCE);
            return E_NOT_OK;
        }
    }
#endif
    
    SeqCtx = &I2C_SequenceContext[SequenceId];
    ChannelId = SeqCtx->AssignedChannel;
    
    /* [CP_SWS_I2C_80804] - Kiểm tra chế độ Target */
    if (I2C_ChannelContext[ChannelId].HwUnitMode != I2C_HW_UNIT_MODE_TARGET)
    {
        I2C_ReportDevError(I2C_E_WRONG_CONDITION);
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_80805] - Kiểm tra Target listening mode */
    if (!I2C_ChannelContext[ChannelId].TargetListeningEnabled)
    {
        I2C_ReportRuntimeError(I2C_E_WRONG_MODE);
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_80802] - Kiểm tra driver đã ở chế độ listening */
    if (I2C_ChannelContext[ChannelId].IsListening)
    {
        return E_NOT_OK;
    }
    
    /* [CP_SWS_I2C_80801] - Bắt đầu chế độ nghe */
    SeqCtx->Result = I2C_SEQ_PENDING;
    SeqCtx->IsActive = TRUE;
    I2C_ChannelContext[ChannelId].IsListening = TRUE;
    
    /* Cấu hình phần cứng cho chế độ Target listening */
    I2C_Hw_StartListening(ChannelId);
    
    return E_OK;
}

/******************************************************************************
* Hàm: I2C_GetVersionInfo
* Mô tả: Lấy thông tin phiên bản
******************************************************************************/
void I2C_GetVersionInfo(Std_VersionInfoType* VersionInfo)
{
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    /* [CP_SWS_I2C_82601] - Kiểm tra con trỏ */
    if (VersionInfo == NULL_PTR)
    {
        I2C_ReportDevError(I2C_E_PARAM_POINTER);
        return;
    }
#endif
    
#if (I2C_VERSION_INFO_API == STD_ON)
    VersionInfo->vendorID = I2C_VENDOR_ID;
    VersionInfo->moduleID = I2C_MODULE_ID;
    VersionInfo->sw_major_version = I2C_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version = I2C_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version = I2C_SW_PATCH_VERSION;
#endif
}

/******************************************************************************
* Hàm: I2C_MainFunction
* Mô tả: Hàm chính được gọi định kỳ
******************************************************************************/
void I2C_MainFunction(void)
{
    uint8 i;
    
    if (!I2C_ModuleInitialized)
    {
        return;
    }
    
    for (i = 0; i < I2C_NUM_OF_CHANNELS; i++)
    {
        /* [CP_SWS_I2C_80901] - Xử lý chế độ nghe vĩnh viễn (Target mode) */
        if (I2C_ChannelContext[i].HwUnitMode == I2C_HW_UNIT_MODE_TARGET &&
            I2C_ChannelContext[i].TargetListeningEnabled &&
            I2C_ChannelContext[i].IsListening)
        {
            I2C_HandleTargetModeData(i);
        }
        
        /* Kiểm tra và xử lý lỗi FIFO */
        uint32 Status = I2C_Hw_GetStatus(i);
        if (Status & IICFn_FIFO_ERROR) /* Giả sử có cờ FIFO error */
        {
            /* [CP_SWS_I2C_00702] - Báo cáo lỗi FIFO handling */
            I2C_ReportRuntimeError(I2C_E_FIFO_HANDLING);
            I2C_Hw_ClearStatus(i, IICFn_FIFO_ERROR);
        }
        
        /* [CP_SWS_I2C_82308] - Tiếp tục với các phần tử trong hàng đợi */
        if (!I2C_ChannelContext[i].IsTransmitting && !I2C_FifoIsEmpty(i))
        {
            I2C_SequenceType NextSeq = I2C_FifoDequeue(i);
            I2C_SequenceContext[NextSeq].IsQueued = FALSE;
            I2C_SequenceContext[NextSeq].IsActive = TRUE;
            I2C_SequenceContext[NextSeq].Result = I2C_SEQ_PENDING;
            I2C_ChannelContext[i].IsTransmitting = TRUE;
            
            I2C_ProcessNextJob(i);
        }
    }
}

/******************************************************************************
* Hàm nội bộ: I2C_ProcessNextJob
* Mô tả: Xử lý Job tiếp theo trong Sequence
******************************************************************************/
static void I2C_ProcessNextJob(uint8 ChannelId)
{
    /* Tìm Sequence đang active trên channel này */
    I2C_SequenceType ActiveSeq = 0xFF;
    uint8 i;
    
    for (i = 0; i < I2C_NUM_OF_SEQUENCES; i++)
    {
        if (I2C_SequenceContext[i].IsActive && 
            I2C_SequenceContext[i].AssignedChannel == ChannelId)
        {
            ActiveSeq = i;
            break;
        }
    }
    
    if (ActiveSeq == 0xFF)
    {
        return; /* Không tìm thấy Sequence active */
    }
    
    I2C_SequenceContextType* SeqCtx = &I2C_SequenceContext[ActiveSeq];
    
    /* [CP_SWS_I2C_82307] - Xử lý multiple Jobs */
    if (SeqCtx->CurrentJobIndex < SeqCtx->NumJobs)
    {
        uint8 JobId = SeqCtx->AssignedJobs[SeqCtx->CurrentJobIndex];
        I2C_JobContextType* JobCtx = &I2C_JobContext[JobId];
        
        /* Bắt đầu truyền Job này */
        I2C_Hw_StartTransmit(ChannelId, JobCtx->NodeAddress);
        
        /* Lưu Job context cho xử lý ngắt */
        /* (Trong thực tế, cần lưu trạng thái để xử lý trong ISR) */
        
        SeqCtx->CurrentJobIndex++;
    }
    else
    {
        /* Đã hoàn thành tất cả Jobs */
        I2C_HandleSequenceCompletion(ChannelId, ActiveSeq, I2C_SEQ_OK);
    }
}

/******************************************************************************
* Hàm nội bộ: I2C_HandleSequenceCompletion
* Mô tả: Xử lý khi Sequence hoàn thành
******************************************************************************/
static void I2C_HandleSequenceCompletion(uint8 ChannelId, I2C_SequenceType SequenceId, I2C_SequenceResultType Result)
{
    I2C_SequenceContextType* SeqCtx = &I2C_SequenceContext[SequenceId];
    
    /* Cập nhật kết quả */
    SeqCtx->Result = Result;
    SeqCtx->IsActive = FALSE;
    I2C_ChannelContext[ChannelId].IsTransmitting = FALSE;
    
    /* [CP_SWS_I2C_80803] - Xử lý khi nhận tin nhắn cuối hoặc lỗi (Target mode) */
    if (I2C_ChannelContext[ChannelId].HwUnitMode == I2C_HW_UNIT_MODE_TARGET)
    {
        /* Xử lý dữ liệu Target mode */
        I2C_HandleTargetModeData(ChannelId);
    }
    
    /* Gọi callback notification nếu được cấu hình */
    if (I2C_SeqEndNotification != NULL_PTR)
    {
        I2C_SeqEndNotification(SequenceId, Result);
    }
    
    /* [CP_SWS_I2C_82308] - Tiếp tục với phần tử tiếp theo trong hàng đợi */
    if (!I2C_FifoIsEmpty(ChannelId))
    {
        I2C_SequenceType NextSeq = I2C_FifoDequeue(ChannelId);
        I2C_SequenceContext[NextSeq].IsQueued = FALSE;
        I2C_SequenceContext[NextSeq].IsActive = TRUE;
        I2C_SequenceContext[NextSeq].Result = I2C_SEQ_PENDING;
        I2C_ChannelContext[ChannelId].IsTransmitting = TRUE;
        
        I2C_ProcessNextJob(ChannelId);
    }
}

/******************************************************************************
* Hàm nội bộ: I2C_HandleTargetModeData
* Mô tả: Xử lý dữ liệu trong chế độ Target
******************************************************************************/
static void I2C_HandleTargetModeData(uint8 ChannelId)
{
    uint32 Status = I2C_Hw_GetStatus(ChannelId);
    
    if (Status & IICS_RX_DATA_READY)
    {
        /* [CP_SWS_I2C_80806] - Đã nhận tin nhắn read */
        /* Tìm Sequence đang listening trên channel này */
        for (uint8 i = 0; i < I2C_NUM_OF_SEQUENCES; i++)
        {
            if (I2C_SequenceContext[i].IsActive && 
                I2C_SequenceContext[i].AssignedChannel == ChannelId)
            {
                uint8 JobId = I2C_SequenceContext[i].AssignedJobs[0]; /* Giả sử 1 Job */
                I2C_JobContextType* JobCtx = &I2C_JobContext[JobId];
                
                if (JobCtx->RxBuffer != NULL_PTR)
                {
                    /* Copy dữ liệu vào RxBuffer */
                    uint8 Data = I2C_Hw_ReadData(ChannelId);
                    
                    if (JobCtx->CurrentIndex < JobCtx->Length)
                    {
                        JobCtx->RxBuffer[JobCtx->CurrentIndex] = Data;
                        JobCtx->CurrentIndex++;
                    }
                }
                break;
            }
        }
        
        I2C_Hw_ClearStatus(ChannelId, IICS_RX_DATA_READY);
    }
    
    if (Status & IICS_TX_EMPTY)
    {
        /* [CP_SWS_I2C_80806] - Cần gửi dữ liệu (write response) */
        /* Tìm Sequence đang listening trên channel này */
        for (uint8 i = 0; i < I2C_NUM_OF_SEQUENCES; i++)
        {
            if (I2C_SequenceContext[i].IsActive && 
                I2C_SequenceContext[i].AssignedChannel == ChannelId)
            {
                uint8 JobId = I2C_SequenceContext[i].AssignedJobs[0];
                I2C_JobContextType* JobCtx = &I2C_JobContext[JobId];
                
                if (JobCtx->TxBuffer != NULL_PTR && 
                    JobCtx->CurrentIndex < JobCtx->Length)
                {
                    /* Gửi dữ liệu từ TxBuffer */
                    I2C_Hw_WriteData(ChannelId, JobCtx->TxBuffer[JobCtx->CurrentIndex]);
                    JobCtx->CurrentIndex++;
                }
                break;
            }
        }
        
        I2C_Hw_ClearStatus(ChannelId, IICS_TX_EMPTY);
    }
}

/******************************************************************************
* Hàm nội bộ: I2C_ReportDevError
* Mô tả: Báo cáo lỗi phát triển
******************************************************************************/
static void I2C_ReportDevError(uint8 ErrorCode)
{
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    Det_ReportError(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, ErrorCode);
#endif
}

/******************************************************************************
* Hàm nội bộ: I2C_ReportRuntimeError
* Mô tả: Báo cáo lỗi runtime
******************************************************************************/
static void I2C_ReportRuntimeError(uint8 ErrorCode)
{
    /* Report to DEM */
    Dem_SetEventStatus(DEM_EVENT_ID_I2C_BASE + ErrorCode, DEM_EVENT_STATUS_FAILED);
    
    /* Report to DET if enabled */
#if (I2C_DEV_ERROR_DETECT == STD_ON)
    Det_ReportRuntimeError(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, ErrorCode);
#endif
}

/******************************************************************************
* Các hàm hỗ trợ FIFO Queue
******************************************************************************/
static void I2C_FifoEnqueue(uint8 ChannelId, I2C_SequenceType SequenceId)
{
    I2C_FifoQueueType* Queue = &I2C_FifoQueues[ChannelId];
    
    if (Queue->Count < I2C_FIFO_QUEUE_SIZE)
    {
        Queue->Queue[Queue->Rear] = SequenceId;
        Queue->Rear = (Queue->Rear + 1) % I2C_FIFO_QUEUE_SIZE;
        Queue->Count++;
    }
    else
    {
        /* [CP_SWS_I2C_00702] - FIFO full error */
        I2C_ReportRuntimeError(I2C_E_FIFO_HANDLING);
    }
}

static I2C_SequenceType I2C_FifoDequeue(uint8 ChannelId)
{
    I2C_FifoQueueType* Queue = &I2C_FifoQueues[ChannelId];
    I2C_SequenceType SeqId = 0xFF;
    
    if (Queue->Count > 0)
    {
        SeqId = Queue->Queue[Queue->Front];
        Queue->Front = (Queue->Front + 1) % I2C_FIFO_QUEUE_SIZE;
        Queue->Count--;
    }
    
    return SeqId;
}

static boolean I2C_FifoIsEmpty(uint8 ChannelId)
{
    return (I2C_FifoQueues[ChannelId].Count == 0);
}

static boolean I2C_FifoIsFull(uint8 ChannelId)
{
    return (I2C_FifoQueues[ChannelId].Count >= I2C_FIFO_QUEUE_SIZE);
}

/******************************************************************************
* Hàm nội bộ: Validation functions
******************************************************************************/
static Std_ReturnType I2C_ValidateJobId(I2C_JobType JobId)
{
    return (JobId < I2C_NUM_OF_JOBS) ? E_OK : E_NOT_OK;
}

static Std_ReturnType I2C_ValidateSequenceId(I2C_SequenceType SequenceId)
{
    return (SequenceId < I2C_NUM_OF_SEQUENCES) ? E_OK : E_NOT_OK;
}

static Std_ReturnType I2C_ValidateChannelId(uint8 ChannelId)
{
    return (ChannelId < I2C_NUM_OF_CHANNELS) ? E_OK : E_NOT_OK;
}