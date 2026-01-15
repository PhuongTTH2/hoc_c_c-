#include "I2c_SimplifiedHw.h"
#include "I2c_Cfg.h"
#include "Det.h"

/* Base addresses cho Simplified I2C channels */
static I2C_SimplifiedRegType* I2C_SimplifiedRegs[] = 
{
    (I2C_SimplifiedRegType*)0x40030100,  /* Channel 00 (IIC00) */
    (I2C_SimplifiedRegType*)0x40030110,  /* Channel 01 (IIC01) */
    (I2C_SimplifiedRegType*)0x40030200,  /* Channel 10 (IIC10) */
    (I2C_SimplifiedRegType*)0x40030210   /* Channel 11 (IIC11) */
};

/******************************************************************************
* Hàm: I2C_SimplifiedHw_Init
* Mô tả: Khởi tạo phần cứng Simplified I2C
******************************************************************************/
void I2C_SimplifiedHw_Init(uint8 ChannelId, uint32 BaudRate)
{
    I2C_SimplifiedRegType* Regs;
    uint8 SpsValue;
    
    if (ChannelId >= I2C_NUMBER_OF_CHANNELS)
    {
        Det_ReportError(I2C_MODULE_ID, 0, I2C_INSTANCE_ID, I2C_E_PARAM_CHANNEL);
        return;
    }
    
    Regs = I2C_SimplifiedRegs[ChannelId];
    
    /* 1. Bật clock cho USCU peripheral */
    PER0_REG |= (1 << ChannelId);
    
    /* 2. Cấu hình chân I2C (SCL, SDA) - MANUAL CONTROL */
    switch (ChannelId)
    {
        case 0: /* IIC00 - SCL00/P14, SDA00/P15 */
            /* Cấu hình chế độ GPIO thủ công */
            PM14_REG = 0;  /* Output mode */
            PM15_REG = 0;
            P14CFG_REG = 0x01;  /* Alternative function: SCL00 */
            P15CFG_REG = 0x01;  /* Alternative function: SDA00 */
            
            /* Set initial state: SDA = 1, SCL = 1 */
            SO0_REG = (1 << 4) | (1 << 5);  /* SCL=1, SDA=1 */
            break;
            
        case 1: /* IIC01 - SCL01/P16, SDA01/P17 */
            PM16_REG = 0;
            PM17_REG = 0;
            P16CFG_REG = 0x01;
            P17CFG_REG = 0x01;
            SO0_REG = (1 << 6) | (1 << 7);  /* SCL=1, SDA=1 */
            break;
    }
    
    /* 3. Tính toán và set baud rate */
    if (I2C_SimplifiedHw_CalculateBaudRate(BaudRate, &SpsValue) == E_OK)
    {
        SPS0_REG = SpsValue;
    }
    else
    {
        /* Default: 100kHz */
        SPS0_REG = SPS_DIV_64;
    }
    
    /* 4. Set mode = Simplified I2C */
    Regs->SMRmn = SMRMN_MODE_I2C;
    
    /* 5. Enable I2C và interrupt */
    Regs->SCRmn = SCRMN_IICE;
    
    /* 6. Enable channel */
    SE0_REG |= (1 << ChannelId);
}

/******************************************************************************
* Hàm: I2C_SimplifiedHw_ManualStart
* Mô tả: Tạo START condition THỦ CÔNG (theo datasheet 19.1.3)
******************************************************************************/
void I2C_SimplifiedHw_ManualStart(uint8 ChannelId)
{
    uint8 SclBit, SdaBit;
    
    /* Xác định bit vị trí cho SCL và SDA */
    switch (ChannelId)
    {
        case 0:
            SclBit = 4;  /* SCL00 */
            SdaBit = 5;  /* SDA00 */
            break;
        case 1:
            SclBit = 6;  /* SCL01 */
            SdaBit = 7;  /* SDA01 */
            break;
        default:
            return;
    }
    
    /* [START Condition Manual Generation - Datasheet 19.1.3] */
    /* 1. SDA = 1, SCL = 1 (Idle state) */
    SO0_REG |= (1 << SdaBit) | (1 << SclBit);
    SOE0_REG |= (1 << SdaBit) | (1 << SclBit);  /* Enable output */
    Delay_us(5);  /* Giữ trạng thái ít nhất 4.7µs */
    
    /* 2. SDA = 0 (Start condition) */
    SO0_REG &= ~(1 << SdaBit);
    Delay_us(5);
    
    /* 3. SCL = 0 (Chuẩn bị cho data transfer) */
    SO0_REG &= ~(1 << SclBit);
    Delay_us(5);
}

/******************************************************************************
* Hàm: I2C_SimplifiedHw_ManualStop
* Mô tả: Tạo STOP condition THỦ CÔNG
******************************************************************************/
void I2C_SimplifiedHw_ManualStop(uint8 ChannelId)
{
    uint8 SclBit, SdaBit;
    
    switch (ChannelId)
    {
        case 0:
            SclBit = 4;
            SdaBit = 5;
            break;
        case 1:
            SclBit = 6;
            SdaBit = 7;
            break;
        default:
            return;
    }
    
    /* [STOP Condition Manual Generation] */
    /* 1. SDA = 0, SCL = 0 (Current state) */
    SO0_REG &= ~((1 << SdaBit) | (1 << SclBit));
    SOE0_REG |= (1 << SdaBit) | (1 << SclBit);
    Delay_us(5);
    
    /* 2. SCL = 1 */
    SO0_REG |= (1 << SclBit);
    Delay_us(5);
    
    /* 3. SDA = 1 (Stop condition) */
    SO0_REG |= (1 << SdaBit);
    Delay_us(5);
    
    /* 4. Disable output (high impedance) */
    SOE0_REG &= ~((1 << SdaBit) | (1 << SclBit));
}

/******************************************************************************
* Hàm: I2C_SimplifiedHw_SendByte
* Mô tả: Gửi 1 byte với kiểm tra ACK
******************************************************************************/
Std_ReturnType I2C_SimplifiedHw_SendByte(uint8 ChannelId, uint8 Data, uint8* AckStatus)
{
    I2C_SimplifiedRegType* Regs = I2C_SimplifiedRegs[ChannelId];
    uint8 i;
    uint32 Timeout = 10000;
    
    if (ChannelId >= I2C_NUMBER_OF_CHANNELS)
    {
        return E_NOT_OK;
    }
    
    /* 1. Ghi data vào thanh ghi dịch */
    Regs->SDRmn = Data;
    
    /* 2. Bắt đầu truyền */
    SS0_REG |= (1 << ChannelId);
    
    /* 3. Chờ transfer complete với timeout */
    while (!(Regs->SSRmn & SSRMN_TEND) && Timeout > 0)
    {
        Timeout--;
    }
    
    if (Timeout == 0)
    {
        return E_NOT_OK;  /* Timeout */
    }
    
    /* 4. Kiểm tra ACK status */
    if (AckStatus != NULL_PTR)
    {
        if (Regs->SSRmn & SSRMN_ACKD)
        {
            *AckStatus = 1;  /* ACK received */
        }
        else if (Regs->SSRmn & SSRMN_ACK)
        {
            *AckStatus = 0;  /* ACK error (NACK) */
        }
        else
        {
            *AckStatus = 0xFF;  /* Unknown */
        }
    }
    
    /* 5. Xóa cờ transfer end */
    Regs->SIRmn = SSRMN_TEND;
    
    return E_OK;
}

/******************************************************************************
* Hàm: I2C_SimplifiedHw_ReceiveByte
* Mô tả: Nhận 1 byte và gửi ACK/NACK
******************************************************************************/
Std_ReturnType I2C_SimplifiedHw_ReceiveByte(uint8 ChannelId, uint8* Data, boolean SendAck)
{
    I2C_SimplifiedRegType* Regs = I2C_SimplifiedRegs[ChannelId];
    uint32 Timeout = 10000;
    
    if (ChannelId >= I2C_NUMBER_OF_CHANNELS || Data == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    /* 1. Cấu hình cho reception */
    /* Note: Simplified I2C cần cấu hình thêm cho receive mode */
    
    /* 2. Bắt đầu nhận */
    SS0_REG |= (1 << ChannelId);
    
    /* 3. Chờ transfer complete */
    while (!(Regs->SSRmn & SSRMN_TEND) && Timeout > 0)
    {
        Timeout--;
    }
    
    if (Timeout == 0)
    {
        return E_NOT_OK;
    }
    
    /* 4. Đọc dữ liệu */
    *Data = (uint8)(Regs->SDRmn & 0xFF);
    
    /* 5. Xử lý ACK (nếu là byte cuối, không gửi ACK) */
    if (!SendAck)
    {
        /* Disable ACK output (theo datasheet Note) */
        SOE0_REG &= ~(1 << (ChannelId + 4));  /* Disable SDA output */
    }
    
    /* 6. Xóa cờ */
    Regs->SIRmn = SSRMN_TEND;
    
    return E_OK;
}

/******************************************************************************
* Hàm: I2C_SimplifiedHw_CalculateBaudRate
* Mô tả: Tính toán giá trị SPS cho baud rate mong muốn
******************************************************************************/
Std_ReturnType I2C_SimplifiedHw_CalculateBaudRate(uint32 DesiredBaudRate, uint32* SpsValue)
{
    uint32 SystemClock = 16000000UL;  /* Giả sử 16MHz */
    uint32 ClockDivider;
    
    if (DesiredBaudRate == 0 || SpsValue == NULL_PTR)
    {
        return E_NOT_OK;
    }
    
    /* Công thức đơn giản hóa cho Simplified I2C */
    /* fSCL = fCLK / (Prescaler * Divider) */
    
    if (DesiredBaudRate <= 100000)  /* Standard mode */
    {
        /* fCLK/64 cho 100kHz với 16MHz */
        *SpsValue = SPS_DIV_64;
    }
    else if (DesiredBaudRate <= 400000)  /* Fast mode */
    {
        /* fCLK/16 cho 400kHz với 16MHz */
        *SpsValue = SPS_DIV_16;
    }
    else
    {
        return E_NOT_OK;  /* Không hỗ trợ */
    }
    
    return E_OK;
}