#ifndef I2C_HW_H
#define I2C_HW_H

#include "Std_Types.h"

/* ========================== BAT32A2x9 I2C REGISTERS ========================== */

/* I2C0 Registers (Base: 0x40041800) */
typedef volatile struct
{
    uint8 IICA0;        /* 0x00: I2C Address Register 0 */
    uint8 IICS0;        /* 0x01: I2C Status Register 0 */
    uint8 IICF0;        /* 0x02: I2C Flag Register 0 */
    uint8 IICCTL00;     /* 0x03: I2C Control Register 00 */
    uint8 IICCTL01;     /* 0x04: I2C Control Register 01 */
    uint8 IICWL0;       /* 0x05: I2C Low-level Width Setting 0 */
    uint8 IICWH0;       /* 0x06: I2C High-level Width Setting 0 */
    uint8 SVA0;         /* 0x07: Slave Address Register 0 */
} I2C0_Registers_Type;

/* I2C1 Registers (Base: 0x40042C00) */
typedef volatile struct
{
    uint8 IICA1;        /* 0x00: I2C Address Register 1 */
    uint8 IICS1;        /* 0x01: I2C Status Register 1 */
    uint8 IICF1;        /* 0x02: I2C Flag Register 1 */
    uint8 IICCTL10;     /* 0x03: I2C Control Register 10 */
    uint8 IICCTL11;     /* 0x04: I2C Control Register 11 */
    uint8 IICWL1;       /* 0x05: I2C Low-level Width Setting 1 */
    uint8 IICWH1;       /* 0x06: I2C High-level Width Setting 1 */
    uint8 SVA1;         /* 0x07: Slave Address Register 1 */
} I2C1_Registers_Type;

/* Register Pointers */
#define I2C0_REGS       ((I2C0_Registers_Type*)0x40041800)
#define I2C1_REGS       ((I2C1_Registers_Type*)0x40042C00)

/* Peripheral Enable Register 0 */
#define PER0            (*(volatile uint8*)0x40038000)
#define PER0_IICA0EN    0x10    /* Bit 4: I2C0 Enable */
#define PER0_IICA1EN    0x20    /* Bit 5: I2C1 Enable */

/* Port Configuration Registers */
#define PM20            (*(volatile uint8*)0x40030020)  /* Port Mode P20 */
#define PMC20           (*(volatile uint8*)0x40030820)  /* Port Mode Control P20 */
#define P20CFG          (*(volatile uint8*)0x40032020)  /* Port Function Config P20 */

#define PM21            (*(volatile uint8*)0x40030021)
#define PMC21           (*(volatile uint8*)0x40030821)
#define P21CFG          (*(volatile uint8*)0x40032021)

#define PM22            (*(volatile uint8*)0x40030022)
#define PMC22           (*(volatile uint8*)0x40030822)
#define P22CFG          (*(volatile uint8*)0x40032022)

#define PM23            (*(volatile uint8*)0x40030023)
#define PMC23           (*(volatile uint8*)0x40030823)
#define P23CFG          (*(volatile uint8*)0x40032023)

/* Open Drain Control */
#define POM20           (*(volatile uint8*)0x40031020)  /* Port Open Drain P20 */
#define POM21           (*(volatile uint8*)0x40031021)  /* Port Open Drain P21 */
#define POM22           (*(volatile uint8*)0x40031022)  /* Port Open Drain P22 */
#define POM23           (*(volatile uint8*)0x40031023)  /* Port Open Drain P23 */

/* ========================== REGISTER BIT DEFINITIONS ========================== */

/* I2C Control Register 0 (IICCTL00/IICCTL10) */
#define IICCTL0_EN      0x01    /* I2C Enable */
#define IICCTL0_IICM    0x40    /* I2C Mode (0=Controller, 1=Target) */
#define IICCTL0_TRX     0x10    /* Transfer Direction (0=Write, 1=Read) */
#define IICCTL0_ACK     0x20    /* Acknowledge Enable */
#define IICCTL0_STO     0x40    /* Stop Condition Generation */
#define IICCTL0_STA     0x80    /* Start Condition Generation */

/* I2C Control Register 1 (IICCTL01/IICCTL11) */
#define IICCTL1_IICRS   0x01    /* I2C Reset */
#define IICCTL1_IICE    0x04    /* I2C Interrupt Enable */
#define IICCTL1_TRXE    0x40    /* Transfer Interrupt Enable */

/* I2C Status Register (IICS0/IICS1) */
#define IICS_TRC        0x01    /* Transfer Completed */
#define IICS_ACK        0x02    /* Acknowledge Status */
#define IICS_STD        0x08    /* Start Condition Detected */
#define IICS_SPD        0x10    /* Stop Condition Detected */
#define IICS_NACK       0x20    /* NACK Detected */
#define IICS_AL         0x40    /* Arbitration Lost */
#define IICS_BUSY       0x80    /* Bus Busy */

/* ========================== HARDWARE FUNCTIONS ========================== */
void I2C_Hw_InitChannel(uint8 channelId, uint32 baudrate, uint8 mode);
void I2C_Hw_DeinitChannel(uint8 channelId);
void I2C_Hw_StartTransmit(uint8 channelId, uint8 slaveAddr, const uint8* data, uint16 length);
void I2C_Hw_StartReceive(uint8 channelId, uint8 slaveAddr, uint8* buffer, uint16 length);
void I2C_Hw_WriteData(uint8 channelId, uint8 data);
uint8 I2C_Hw_ReadData(uint8 channelId);
void I2C_Hw_GenerateStart(uint8 channelId);
void I2C_Hw_GenerateStop(uint8 channelId);
void I2C_Hw_EnableAck(uint8 channelId);
void I2C_Hw_DisableAck(uint8 channelId);
boolean I2C_Hw_IsBusBusy(uint8 channelId);
boolean I2C_Hw_IsTransferComplete(uint8 channelId);
boolean I2C_Hw_IsNackReceived(uint8 channelId);
uint8 I2C_Hw_GetStatus(uint8 channelId);
void I2C_Hw_ClearStatus(uint8 channelId);

#endif /* I2C_HW_H */