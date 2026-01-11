/*
 * CDD_I2C - AUTOSAR Classic Platform R24-11
 * Target MCU: Renesas BAT32A259 (RL78 family)
 * Scope: Reference-quality FULL driver skeleton
 * - Conforms to AUTOSAR_CP_SWS_I2CDriver (Doc ID 1101, R24-11)
 * - CDD style (not official MCAL)
 * - Includes DET / DEM hooks
 * - EB mode only
 *
 * NOTE:
 *  - Hardware register names are representative (RL78 IICA)
 *  - Timing/bitrate values must be tuned with Renesas HW manual
 */

/* =================== [INCLUDES] =================== */
#include "Std_Types.h"
#include "Det.h"
#include "Dem.h"
#include "I2c.h"

/* =================== [VERSION INFO] =================== */
#define I2C_VENDOR_ID        0x1234
#define I2C_MODULE_ID        0x5A
#define I2C_SW_MAJOR_VERSION 24
#define I2C_SW_MINOR_VERSION 11
#define I2C_SW_PATCH_VERSION 0

/* =================== [DEV ERROR SWITCH] =================== */
#define I2C_DEV_ERROR_DETECT STD_ON

/* =================== [DET ERROR CODES] =================== */
#define I2C_E_PARAM_JOB          0x00
#define I2C_E_PARAM_SEQUENCE     0x01
#define I2C_E_PARAM_POINTER      0x02
#define I2C_E_UNINIT             0x03
#define I2C_E_WRONG_CONDITION    0x04

/* =================== [RUNTIME ERRORS -> DEM] =================== */
#define I2C_E_NACK_RECEIVED        0x00
#define I2C_E_ARBITRATION_FAILURE 0x01
#define I2C_E_FIFO_HANDLING       0x02
#define I2C_E_BUS_FAILURE         0x03
#define I2C_E_WRONG_MODE          0x04

/* =================== [LOCAL TYPES] =================== */
typedef enum {
    I2C_UNINIT = 0,
    I2C_IDLE,
    I2C_BUSY
} I2C_DriverStateType;

/* =================== [STATIC DATA] =================== */
static const I2C_ConfigType* I2c_ConfigPtr = NULL_PTR;
static I2C_DriverStateType I2c_State = I2C_UNINIT;
static I2C_SequenceResultType I2c_SeqResult[8];

/* EB buffers */
static I2C_DataConstPtrType* I2c_TxBuf[8];
static I2C_DataPtrType*      I2c_RxBuf[8];
static I2C_NumberOfDataType  I2c_Length[8];
static I2C_AddressType       I2c_NodeAddr[8];

/* =================== [LOW LEVEL HW - RL78 IICA] =================== */

#define IICA0_CTL   (*(volatile uint8*)0xFFF10)
#define IICA0_STAT  (*(volatile uint8*)0xFFF11)
#define IICA0_TX    (*(volatile uint8*)0xFFF12)
#define IICA0_RX    (*(volatile uint8*)0xFFF13)
#define IICA0_CLK   (*(volatile uint8*)0xFFF14)

static void HW_I2C_Init(uint32 bitrate)
{
    /* Clock & pin setup done by MCU driver */
    IICA0_CTL = 0x00;
    IICA0_CLK = (uint8)(bitrate & 0xFF);
    IICA0_CTL = 0x80; /* enable */
}

static Std_ReturnType HW_I2C_Start(uint8 addr, boolean read)
{
    IICA0_TX = (addr << 1) | (read ? 1 : 0);
    while (!(IICA0_STAT & 0x01));
    if (IICA0_STAT & 0x08) return E_NOT_OK; /* NACK */
    return E_OK;
}

static Std_ReturnType HW_I2C_Write(uint8 data)
{
    IICA0_TX = data;
    while (!(IICA0_STAT & 0x01));
    if (IICA0_STAT & 0x08) return E_NOT_OK;
    return E_OK;
}

static uint8 HW_I2C_Read(boolean last)
{
    if (last) IICA0_CTL |= 0x02; /* NACK */
    while (!(IICA0_STAT & 0x01));
    return IICA0_RX;
}

static void HW_I2C_Stop(void)
{
    IICA0_CTL |= 0x04;
}

/* =================== [API IMPLEMENTATION] =================== */

void I2C_Init(const I2C_ConfigType* ConfigPtr)
{
#if I2C_DEV_ERROR_DETECT
    if (ConfigPtr == NULL_PTR)
    {
        Det_ReportError(I2C_MODULE_ID, 0, 0x00, I2C_E_PARAM_POINTER);
        return;
    }
#endif

    I2c_ConfigPtr = ConfigPtr;
    HW_I2C_Init(ConfigPtr->I2cBaudRate);

    for (uint8 i = 0; i < 8; i++)
    {
        I2c_SeqResult[i] = I2C_SEQ_OK;
    }

    I2c_State = I2C_IDLE;
}

void I2C_DeInit(void)
{
#if I2C_DEV_ERROR_DETECT
    if (I2c_State == I2C_UNINIT)
    {
        Det_ReportError(I2C_MODULE_ID, 0, 0x01, I2C_E_UNINIT);
        return;
    }
#endif

    IICA0_CTL = 0x00;
    I2c_State = I2C_UNINIT;
}

Std_ReturnType I2C_SetupEB(I2C_JobType JobId,
                           I2C_AddressType NodeAddress,
                           I2C_DataConstPtrType* TxDataBufferPtr,
                           I2C_DataPtrType* RxDataBufferPtr,
                           I2C_NumberOfDataType Length)
{
#if I2C_DEV_ERROR_DETECT
    if ((TxDataBufferPtr == NULL_PTR && RxDataBufferPtr == NULL_PTR) ||
        (TxDataBufferPtr != NULL_PTR && RxDataBufferPtr != NULL_PTR))
    {
        Det_ReportError(I2C_MODULE_ID, 0, 0x02, I2C_E_PARAM_POINTER);
        return E_NOT_OK;
    }
#endif

    I2c_TxBuf[JobId]   = TxDataBufferPtr;
    I2c_RxBuf[JobId]   = RxDataBufferPtr;
    I2c_Length[JobId]  = Length;
    I2c_NodeAddr[JobId] = NodeAddress;

    return E_OK;
}

Std_ReturnType I2C_SyncTransmit(I2C_SequenceType SequenceId)
{
#if I2C_DEV_ERROR_DETECT
    if (I2c_State == I2C_UNINIT)
    {
        Det_ReportError(I2C_MODULE_ID, 0, 0x04, I2C_E_UNINIT);
        return E_NOT_OK;
    }
#endif

    I2c_State = I2C_BUSY;
    I2c_SeqResult[SequenceId] = I2C_SEQ_PENDING;

    if (HW_I2C_Start(I2c_NodeAddr[SequenceId], (I2c_RxBuf[SequenceId] != NULL_PTR)) != E_OK)
    {
        Dem_ReportErrorStatus(I2C_E_NACK_RECEIVED, DEM_EVENT_STATUS_FAILED);
        I2c_SeqResult[SequenceId] = I2C_SEQ_NACK;
        HW_I2C_Stop();
        I2c_State = I2C_IDLE;
        return E_NOT_OK;
    }

    if (I2c_TxBuf[SequenceId] != NULL_PTR)
    {
        for (uint16 i = 0; i < I2c_Length[SequenceId]; i++)
        {
            if (HW_I2C_Write((*I2c_TxBuf[SequenceId])[i]) != E_OK)
            {
                I2c_SeqResult[SequenceId] = I2C_SEQ_FAILED;
                break;
            }
        }
    }
    else
    {
        for (uint16 i = 0; i < I2c_Length[SequenceId]; i++)
        {
            (*I2c_RxBuf[SequenceId])[i] = HW_I2C_Read(i == (I2c_Length[SequenceId] - 1));
        }
    }

    HW_I2C_Stop();
    I2c_SeqResult[SequenceId] = I2C_SEQ_OK;
    I2c_State = I2C_IDLE;

    return E_OK;
}

I2C_SequenceResultType I2C_GetSequenceResult(I2C_SequenceType SequenceId)
{
#if I2C_DEV_ERROR_DETECT
    if (SequenceId >= 8)
    {
        Det_ReportError(I2C_MODULE_ID, 0, 0x09, I2C_E_PARAM_SEQUENCE);
        return I2C_SEQ_FAILED;
    }
#endif

    return I2c_SeqResult[SequenceId];
}

void I2C_GetVersionInfo(Std_VersionInfoType* VersionInfo)
{
#if I2C_DEV_ERROR_DETECT
    if (VersionInfo == NULL_PTR)
    {
        Det_ReportError(I2C_MODULE_ID, 0, 0x07, I2C_E_PARAM_POINTER);
        return;
    }
#endif

    VersionInfo->vendorID = I2C_VENDOR_ID;
    VersionInfo->moduleID = I2C_MODULE_ID;
    VersionInfo->sw_major_version = I2C_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version = I2C_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version = I2C_SW_PATCH_VERSION;
}


#define PER0   (*(volatile uint8*)0x40020420U)
#define PIOR0  (*(volatile uint8*)0x40040877U)

/* Port registers */
#define P1     (*(volatile uint8*)0x40040301U)
#define PM1    (*(volatile uint8*)0x40041301U)
#define P6     (*(volatile uint8*)0x40040306U)
#define PM6    (*(volatile uint8*)0x40041306U)

/* IICA0 registers (base = 0xFFF10U ví dụ) */
#define IICA0CTL0 (*(volatile uint8*)0xFFF10U)
#define IICA0CTL1 (*(volatile uint8*)0xFFF11U)
#define IICA0SVA  (*(volatile uint8*)0xFFF12U)
#define IICA0WL   (*(volatile uint8*)0xFFF13U)
#define IICA0WH   (*(volatile uint8*)0xFFF14U)
#define IICA0DR   (*(volatile uint8*)0xFFF15U)
#define IICA0STR  (*(volatile uint8*)0xFFF16U)

//IICA0EN
PER0 |= (1U << 4);   /* Enable IICA0 clock */

PIOR0 &= ~(1U << 2);   /* PIOR02 = 0 → P60/P61 */

/* Px = 0 */
P6 &= ~((1U << 0) | (1U << 1));

/* Output mode */
PM6 &= ~((1U << 0) | (1U << 1));

PIOR0 |= (1U << 2);    /* PIOR02 = 1 → P14/P15 */

/* Px = 0 */
P1 &= ~((1U << 4) | (1U << 5));

/* Output mode */
PM1 &= ~((1U << 4) | (1U << 5));

/* Peripheral mode */
PMC14 |= (1U << 4);    /* P14 = SCL */
PMC15 |= (1U << 5);    /* P15 = SDA */


3️⃣ Chọn mode IICA (Master / Slave)
IICA0CTL1 = 0x00;   /* Controller mode */

4️⃣ Set timing (WL / WH)
IICA0WL = 0x1F;   /* Low period */
IICA0WH = 0x1F;   /* High period */

IICA0SVA = (uint8)(SlaveAddress & 0x7FU);


#include <stdint.h>

typedef uint8_t uint8;

/* ========= PER0 ========= */
// 11.3.1 Peripheral enable register 0 (PER0)
#define PER0 (*(volatile uint8*)0x40020420U)
#define PER0_IICA0EN (1U << 4)

/* ========= PIOR ========= */
// 2.3.10 Peripheral I/O redirect register 0 (PIOR0)
#define PIOR0 (*(volatile uint8*)0x40040877U)
#define PIOR02 (1U << 2)   /* 1=P14/P15, 0=P60/P61 */
#define PIOR00 (~(1U << 2))   /* 0=P60/P61 */

/* ========= PORT ========= */
#define P1   (*(volatile uint8*)(0x40040000U + 0x301))
#define PM1  (*(volatile uint8*)(0x40040000U + 0x321))
#define PMC1 (*(volatile uint8*)(0x40040000U + 0x361))

/* ========= IICA0 ========= */
typedef struct
{
    volatile uint8 IICCTL00;
    volatile uint8 IICCTL01;
    volatile uint8 IICS0;
    volatile uint8 IICWL0;
    volatile uint8 IICWH0;
    volatile uint8 IICD0;
    volatile uint8 SVA0;
} IICA_RegType;

#define IICA0_BASE 0xFFF10U
#define IICA0 ((IICA_RegType*)IICA0_BASE)

/* ========= IICCTL00 bits ========= */
#define IICE (1U << 7)
#define WREL (1U << 6)
#define STT  (1U << 5)
#define SPT  (1U << 4)
#define ACKE (1U << 3)

/* ========= IICS0 bits ========= */
#define TEND (1U << 7)
#define ACKD (1U << 2)

static void I2C_PinInit(void)
{
    /* Select P14/P15 for SCLA0/SDAA0 */
    PIOR0 |= PIOR02;
    // PIOR0 &= PIOR00;

    /* Px must be 0 */
    P1 &= ~((1U << 4) | (1U << 5));

    /* Output mode */
    PM1 &= ~((1U << 4) | (1U << 5));

    /* Enable peripheral function */
    PMC1 |=  (1U << 4) | (1U << 5);
}

static void I2C_EnableClock(void)
{
    PER0 |= PER0_IICA0EN;
}
// 20.3 Registers for controlling serial interface IICA
void I2C_Hw_Init(void)
{
    I2C_EnableClock();
    I2C_PinInit();

    /* Disable IICA before config */
    IICA0->IICCTL00 = 0x00;

    /* Master mode */
    IICA0->IICCTL01 = 0x00;

    /* Timing (example value, ~100kHz depending on fCLK) */
    IICA0->IICWL0 = 0x1F;
    IICA0->IICWH0 = 0x1F;

    /* Enable IICA */
    IICA0->IICCTL00 |= IICE;
}

void I2C_Hw_Start(void)
{
    /* Generate START */
    IICA0->IICCTL00 |= STT;

    /* Wait for START complete (bus active) */
    while (!(IICA0->IICS0 & TEND));
}

int I2C_Hw_Write(uint8 data)
{
    /* Write data */
    IICA0->IICD0 = data;

    /* Wait transmission end */
    while (!(IICA0->IICS0 & TEND));

    /* Check ACK */
    if (IICA0->IICS0 & ACKD)
    {
        return -1; /* NACK */
    }

    return 0; /* OK */
}

uint8 I2C_Hw_Read(uint8 ack)
{
    if (ack)
    {
        /* Send ACK */
        IICA0->IICCTL00 |= ACKE;
    }
    else
    {
        /* Send NACK */
        IICA0->IICCTL00 &= ~ACKE;
    }

    /* Release clock to receive */
    IICA0->IICCTL00 |= WREL;

    /* Wait receive complete */
    while (!(IICA0->IICS0 & TEND));

    return IICA0->IICD0;
}
void I2C_WriteOneByte(uint8 slave, uint8 data)
{
    I2C_Hw_Init();

    I2C_Hw_Start();
    I2C_Hw_Write(slave << 1); /* write */
    I2C_Hw_Write(data);
    I2C_Hw_Stop();
}


void I2C_Init_MasterSimple(uint32 fCLK_Hz)
{
    /* Enable clock for IICA0 */
    PER0 |= PER0_IICA0EN;

    /* Port config */
    I2C_PortInit_P14_P15();

    /* Disable IICA before config */
    IICA0->CTL0 = 0x00;

    /* Controller (Master), polling */
    IICA0->CTL1 = 0x00;

    /* ---- Timing calculation ----
       I2C 100kHz → T = 10µs → TH = TL = 5µs
       WL = WH = (fCLK × 5µs) - 1
    */
    uint32 ticks = (fCLK_Hz / 1000000U) * 5U;
    IICA0->WL = (uint8)(ticks - 1U);
    IICA0->WH = (uint8)(ticks - 1U);

    /* Enable IICA */
    IICA0->CTL0 |= IICE;
}


typedef enum
{
    I2C_MODE_MASTER = 0,
    I2C_MODE_SLAVE
} I2C_ModeType;

void I2C_Init_MasterSlave(uint32 fCLK_Hz,
                          uint32 busSpeed_Hz,
                          I2C_ModeType mode,
                          uint8 slaveAddr)
{
    /* Enable clock */
    PER0 |= PER0_IICA0EN;

    /* Port */
    I2C_PortInit_P14_P15();

    /* Disable IICA */
    IICA0->CTL0 = 0x00;

    /* Mode */
    if (mode == I2C_MODE_MASTER)
    {
        IICA0->CTL1 = 0x00;  /* Controller */
    }
    else
    {
        IICA0->CTL1 = 0x01;  /* Target (Slave) – bit theo manual */
        IICA0->SVA  = (slaveAddr & 0x7FU);
    }

    /* Timing */
    uint32 halfPeriod_us = (1000000U / busSpeed_Hz) / 2U;
    uint32 ticks = (fCLK_Hz / 1000000U) * halfPeriod_us;

    IICA0->WL = (uint8)(ticks - 1U);
    IICA0->WH = (uint8)(ticks - 1U);

    /* Enable IICA */
    IICA0->CTL0 |= IICE;
}
typedef struct
{
    volatile uint8 IICCTL00;   /* Control 0 */
    volatile uint8 IICCTL01;   /* Control 1 */
    volatile uint8 IICWL0;     /* Low width */
    volatile uint8 IICWH0;     /* High width */
    volatile uint8 IICIE0;     /* Interrupt enable */
    volatile uint8 IICIF0;     /* Interrupt flag */
    volatile uint8 IICCL0;     /* Status */
    volatile uint8 IICD0;      /* Data register */
    volatile uint8 IICS0;      /* Status register */
    volatile uint8 IICA0;      /* Slave address */
} IICA_Type;
#define IICA0   ((IICA_Type*)0xF0180)

/* IICCTL00 */
#define IICE    (1u << 7)   /* IICA enable */
#define LREL    (1u << 6)   /* Release clock stretching */
#define MST     (1u << 5)   /* Master status */
#define STT     (1u << 1)   /* START */
#define SPT     (1u << 0)   /* STOP */

/* IICCTL01 */
#define ACKE    (1u << 2)
#define WTIM    (1u << 1)

/* IICS */
#define TDRE    (1u << 7)   /* Tx buffer empty */
#define RDRF    (1u << 6)   /* Rx buffer full */
#define NACKF   (1u << 4)   /* NACK detected */
#define ALD     (1u << 3)   /* Arbitration lost */

void I2C_Hw_Init(const I2c_ChannelType* Ch)
{
    IICA0->IICCTL00 = 0x00;          /* Disable */
    IICA0->IICCTL01 = 0x00;

    /* Timing: fCLK / (2*(WL+WH+2)) */
    IICA0->IICWL0 = 0x1F;            /* example */
    IICA0->IICWH0 = 0x1F;

    /* Enable ACK */
    IICA0->IICCTL01 |= ACKE;

    /* Enable IICA */
    IICA0->IICCTL00 |= IICE;
}

void I2C_Hw_Start(uint16 Addr, boolean Read)
{
    uint8 address = (Addr << 1) | (Read ? 1 : 0);

    IICA0->IICCTL00 |= STT;      /* Generate START */
    while (!(IICA0->IICS0 & TDRE));

    IICA0->IICD0 = address;     /* Send address */
}

Std_ReturnType I2C_Hw_Write(uint8 Data)
{
    while (!(IICA0->IICS0 & TDRE));

    if (IICA0->IICS0 & NACKF) return E_NOT_OK;
    if (IICA0->IICS0 & ALD)   return E_NOT_OK;

    IICA0->IICD0 = Data;
    return E_OK;
}

Std_ReturnType I2C_Hw_Read(uint8* Data, boolean Last)
{
    while (!(IICA0->IICS0 & RDRF));

    if (IICA0->IICS0 & NACKF) return E_NOT_OK;

    *Data = IICA0->IICD0;

    if (Last)
        IICA0->IICCTL01 &= ~ACKE;   /* NACK on last byte */

    return E_OK;
}
void I2C_Hw_Stop(void)
{
    IICA0->IICCTL00 |= SPT;
    while (IICA0->IICCTL00 & SPT);
}

static void I2C_HandleHwError(I2c_SequenceRuntimeType* SeqRt)
{
    if (IICA0->IICS0 & NACKF)
        SeqRt->Result = I2C_SEQ_NACK;
    else
        SeqRt->Result = I2C_SEQ_FAILED;

    I2C_Hw_Stop();
    SeqRt->Active = FALSE;
}
void I2C_Hw_EnableIrq(void)
{
    IICA0->IICIE0 = 1;
}
__interrupt void IICA0_ISR(void)
{
    I2c_SequenceRuntimeType* rt = &I2c_SeqRt[gActiveSeq];
    I2c_JobRuntimeType* jrt = &I2c_JobRt[gActiveJob];

    if (IICA0->IICS0 & NACKF) {
        I2C_HandleHwError(rt);
        return;
    }

    if (IICA0->IICS0 & TDRE) {
        if (jrt->Index < jrt->Length) {
            IICA0->IICD0 = jrt->TxBuf[jrt->Index++];
        } else {
            I2C_Hw_Stop();
            rt->Result = I2C_SEQ_OK;
            rt->Active = FALSE;
        }
    }
}

#define I2C_E_UNINIT           0x01
#define I2C_E_PARAM_SEQUENCE  0x02
#define I2C_E_WRONG_MODE      0x03

#if (I2cDevErrorDetect == STD_ON)
#define I2C_DET_REPORT(err) \
    Det_ReportError(I2C_MODULE_ID, 0, I2C_SERVICE_ID, err)
#else
#define I2C_DET_REPORT(err)
#endif

static void I2C_CalcTiming(uint32 fclk, uint32 baud,
                           uint8* wl, uint8* wh)
{
    uint32 sum = (fclk / (2u * baud)) - 2u;
    *wl = (uint8)(sum / 2u);
    *wh = (uint8)(sum / 2u);
}
