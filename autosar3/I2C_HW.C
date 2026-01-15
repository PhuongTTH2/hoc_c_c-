/******************************************************************************
* File: I2c_Hw.c
* Description: Triển khai truy cập phần cứng I2C cho BAT32A2x9
******************************************************************************/

#include "I2c_Hw.h"
#include "I2c_Cfg.h"
#include "Det.h"
#include "Dem.h"

/* Con trỏ đến thanh ghi I2C */
static I2C_HwRegType* I2C_HwRegs[I2C_NUM_OF_CHANNELS] = 
{
    (I2C_HwRegType*)I2C0_BASE_ADDRESS,
    (I2C_HwRegType*)I2C1_BASE_ADDRESS
};

/* Cấu trúc context cho mỗi kênh I2C */
typedef struct
{
    boolean Initialized;
    boolean IsTransmitting;
    uint8 CurrentSlaveAddress;
    uint32 BaudRate;
} I2C_HwChannelContextType;

static I2C_HwChannelContextType I2C_HwContext[I2C_NUM_OF_CHANNELS];

/******************************************************************************
* Hàm: I2C_Hw_InitChannel
* Mô tả: Khởi tạo kênh I2C phần cứng
******************************************************************************/
void I2C_Hw_InitChannel(uint8 ChannelId, const I2C_ConfigType* ConfigPtr)
{
    I2C_HwRegType* Regs;
    uint32 IICWLn_Value, IICWHn_Value;
    uint32 SystemClock = 16000000UL; /* Giả sử hệ thống 16MHz */
    
    if (ChannelId >= I2C_NUM_OF_CHANNELS)
    {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, I2C_E_PARAM_JOB);
        return;
    }
    
    Regs = I2C_HwRegs[ChannelId];
    
    /* 1. Bật clock cho I2C peripheral trong PER0 */
    if (ChannelId == 0)
    {
        MCU_PER0 |= PER0_IICA0_ENABLE;
    }
    else
    {
        MCU_PER0 |= PER0_IICA1_ENABLE;
    }
    
    /* 2. Cấu hình chân I2C (SCL, SDA) */
    /* Giả sử chân P10/P11 cho I2C0, P12/P13 cho I2C1 */
    switch (ChannelId)
    {
        case 0:
            /* P10: SCL0, P11: SDA0 */
            PM10 = 0; /* Chế độ output ban đầu */
            PM11 = 0;
            P10CFG = 0x02; /* Chức năng phụ: SCL0 */
            P11CFG = 0x02; /* Chức năng phụ: SDA0 */
            break;
            
        case 1:
            /* P12: SCL1, P13: SDA1 */
            PM12 = 0;
            PM13 = 0;
            P12CFG = 0x02; /* SCL1 */
            P13CFG = 0x02; /* SDA1 */
            break;
    }
    
    /* 3. Reset I2C peripheral */
    Regs->IICCTLn0 = IICCTL0_IICRST;
    
    /* 4. Tính toán giá trị baud rate */
    if (I2C_Hw_CalculateBaudRate(ConfigPtr->BaudRate, SystemClock, 
                                 &IICWLn_Value, &IICWHn_Value) != E_OK)
    {
        /* Sử dụng giá trị mặc định nếu tính toán thất bại */
        IICWLn_Value = 78;  /* Cho 100kHz với 16MHz */
        IICWHn_Value = 78;
    }
    
    Regs->IICWLn = IICWLn_Value;
    Regs->IICWHn = IICWHn_Value;
    
    /* 5. Cấu hình chế độ hoạt động */
    if (ConfigPtr->HwUnitMode == I2C_HW_UNIT_MODE_CONTROLLER)
    {
        Regs->IICCTLn1 = (Regs->IICCTLn1 & ~IICCTL1_IICM_MASK) | IICCTL1_IICM_MASTER;
    }
    else
    {
        Regs->IICCTLn1 = (Regs->IICCTLn1 & ~IICCTL1_IICM_MASK) | IICCTL1_IICM_SLAVE;
        Regs->SVAn = ConfigPtr->TargetAddress;
    }
    
    /* 6. Bật I2C và ngắt */
    Regs->IICCTLn0 = IICCTL0_IICE | IICCTL0_IICINT;
    
    /* 7. Lưu context */
    I2C_HwContext[ChannelId].Initialized = TRUE;
    I2C_HwContext[ChannelId].BaudRate = ConfigPtr->BaudRate;
    
    Det_ReportInformation(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, 
                         "I2C Channel %d initialized", ChannelId);
}

/******************************************************************************
* Hàm: I2C_Hw_CalculateBaudRate
* Mô tả: Tính toán giá trị IICWLn và IICWHn cho tốc độ baud mong muốn
******************************************************************************/
Std_ReturnType I2C_Hw_CalculateBaudRate(uint32 DesiredBaudRate, uint32 SystemClock, 
                                        uint32* IICWLn, uint32* IICWHn)
{
    uint32 Period;
    
    if (DesiredBaudRate == 0 || SystemClock == 0)
    {
        return E_NOT_OK;
    }
    
    /* Công thức: BaudRate = SystemClock / (2 * (IICWLn + IICWHn + 4)) */
    Period = SystemClock / (2 * DesiredBaudRate);
    
    if (Period < 4)
    {
        return E_NOT_OK; /* Period quá nhỏ */
    }
    
    Period -= 4; /* Trừ offset */
    
    /* Chia đều cho low và high period */
    *IICWLn = Period / 2;
    *IICWHn = Period / 2;
    
    /* Điều chỉnh nếu lẻ */
    if (Period % 2)
    {
        (*IICWLn)++;
    }
    
    /* Kiểm tra giới hạn (8-bit register) */
    if (*IICWLn > 0xFF || *IICWHn > 0xFF)
    {
        return E_NOT_OK;
    }
    
    return E_OK;
}

/******************************************************************************
* Hàm: I2C_Hw_DeinitChannel
* Mô tả: Hủy khởi tạo kênh I2C
******************************************************************************/
void I2C_Hw_DeinitChannel(uint8 ChannelId)
{
    I2C_HwRegType* Regs;
    
    if (ChannelId >= I2C_NUM_OF_CHANNELS)
    {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, I2C_E_PARAM_JOB);
        return;
    }
    
    Regs = I2C_HwRegs[ChannelId];
    
    /* 1. Tắt I2C */
    Regs->IICCTLn0 = 0;
    
    /* 2. Reset thanh ghi */
    Regs->IICAn = 0;
    Regs->SVAn = 0;
    Regs->IICWLn = 0;
    Regs->IICWHn = 0;
    
    /* 3. Tắt clock peripheral */
    if (ChannelId == 0)
    {
        MCU_PER0 &= ~PER0_IICA0_ENABLE;
    }
    else
    {
        MCU_PER0 &= ~PER0_IICA1_ENABLE;
    }
    
    /* 4. Reset chân GPIO về mặc định */
    switch (ChannelId)
    {
        case 0:
            P10CFG = 0;
            P11CFG = 0;
            break;
        case 1:
            P12CFG = 0;
            P13CFG = 0;
            break;
    }
    
    /* 5. Xóa context */
    I2C_HwContext[ChannelId].Initialized = FALSE;
    I2C_HwContext[ChannelId].IsTransmitting = FALSE;
}

/******************************************************************************
* Hàm: I2C_Hw_StartTransmit
* Mô tả: Bắt đầu truyền dữ liệu
******************************************************************************/
void I2C_Hw_StartTransmit(uint8 ChannelId, uint8 SlaveAddress)
{
    I2C_HwRegType* Regs;
    
    if (ChannelId >= I2C_NUM_OF_CHANNELS || 
        !I2C_HwContext[ChannelId].Initialized)
    {
        return;
    }
    
    Regs = I2C_HwRegs[ChannelId];
    
    /* 1. Đặt địa chỉ slave */
    Regs->IICAn = (SlaveAddress << 1); /* Bit 0 là R/W bit */
    
    /* 2. Tạo điều kiện START */
    Regs->IICCTLn0 |= IICCTL0_IICCSC;
    
    /* 3. Lưu context */
    I2C_HwContext[ChannelId].CurrentSlaveAddress = SlaveAddress;
    I2C_HwContext[ChannelId].IsTransmitting = TRUE;
}

/******************************************************************************
* Hàm: I2C_Hw_GetStatus
* Mô tả: Đọc trạng thái I2C
******************************************************************************/
uint32 I2C_Hw_GetStatus(uint8 ChannelId)
{
    if (ChannelId >= I2C_NUM_OF_CHANNELS)
    {
        return 0;
    }
    
    return I2C_HwRegs[ChannelId]->IICSn;
}

/******************************************************************************
* Hàm: I2C_Hw_ClearStatus
* Mô tả: Xóa cờ trạng thái
******************************************************************************/
void I2C_Hw_ClearStatus(uint8 ChannelId, uint32 StatusFlags)
{
    I2C_HwRegType* Regs;
    
    if (ChannelId >= I2C_NUM_OF_CHANNELS)
    {
        return;
    }
    
    Regs = I2C_HwRegs[ChannelId];
    
    /* Xóa cờ bằng cách ghi 1 */
    Regs->IICFn = StatusFlags;
}

/******************************************************************************
* Hàm: I2C_Hw_IRQHandler
* Mô tả: Xử lý ngắt I2C
******************************************************************************/
void I2C_Hw_IRQHandler(uint8 ChannelId)
{
    I2C_HwRegType* Regs;
    uint32 Status;
    
    if (ChannelId >= I2C_NUM_OF_CHANNELS)
    {
        return;
    }
    
    Regs = I2C_HwRegs[ChannelId];
    Status = Regs->IICSn;
    
    /* Xử lý lỗi */
    if (Status & IICS_NACK_RECEIVED)
    {
        Dem_SetEventStatus(DEM_EVENT_ID_I2C_NACK, DEM_EVENT_STATUS_FAILED);
        I2C_Hw_ClearStatus(ChannelId, IICS_NACK_RECEIVED);
    }
    
    if (Status & IICS_ARBITRATION_LOST)
    {
        Dem_SetEventStatus(DEM_EVENT_ID_I2C_ARBITRATION, DEM_EVENT_STATUS_FAILED);
        I2C_Hw_ClearStatus(ChannelId, IICS_ARBITRATION_LOST);
    }
    
    if (Status & IICS_BUS_ERROR)
    {
        Dem_SetEventStatus(DEM_EVENT_ID_I2C_BUS_ERROR, DEM_EVENT_STATUS_FAILED);
        I2C_Hw_ClearStatus(ChannelId, IICS_BUS_ERROR);
    }
    
    /* Xử lý truyền thành công */
    if (Status & IICS_TRANSFER_COMPLETE)
    {
        I2C_HwContext[ChannelId].IsTransmitting = FALSE;
        I2C_Hw_ClearStatus(ChannelId, IICS_TRANSFER_COMPLETE);
    }
}

/******************************************************************************
* Hàm: I2C_Hw_WriteData
* Mô tả: Ghi dữ liệu vào thanh ghi I2C
******************************************************************************/
void I2C_Hw_WriteData(uint8 ChannelId, uint8 Data)
{
    if (ChannelId >= I2C_NUM_OF_CHANNELS)
    {
        return;
    }
    
    I2C_HwRegs[ChannelId]->IICAn = Data;
}

/******************************************************************************
* Hàm: I2C_Hw_ReadData
* Mô tả: Đọc dữ liệu từ thanh ghi I2C
******************************************************************************/
uint8 I2C_Hw_ReadData(uint8 ChannelId)
{
    if (ChannelId >= I2C_NUM_OF_CHANNELS)
    {
        return 0;
    }
    
    return (uint8)(I2C_HwRegs[ChannelId]->IICAn);
}