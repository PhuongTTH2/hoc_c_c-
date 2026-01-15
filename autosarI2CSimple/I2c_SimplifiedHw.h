#ifndef I2C_SIMPLIFIED_HW_H
#define I2C_SIMPLIFIED_HW_H

#include "Std_Types.h"

/* ================= REGISTER DEFINITIONS ================= */
/* USCU Registers cho Simplified I2C (Chapter 19) */
typedef struct
{
    volatile uint8 SMRmn;    /* Serial Mode Register mn */
    volatile uint8 SCRmn;    /* Serial Communication Run Setting Register mn */
    volatile uint8 SSRmn;    /* Serial Status Register mn */
    volatile uint8 SIRmn;    /* Serial Flag Clear Trigger Register mn */
    volatile uint16 SDRmn;   /* Serial Data Register mn (16-bit) */
} I2C_SimplifiedRegType;

/* Control Registers (Table 19-1) */
#define PER0_REG                            (*((volatile uint8*)0x40030000))
#define SPS0_REG                            (*((volatile uint8*)0x40030004))
#define SE0_REG                             (*((volatile uint8*)0x40030008))
#define SS0_REG                             (*((volatile uint8*)0x4003000C))
#define ST0_REG                             (*((volatile uint8*)0x40030010))
#define SOE0_REG                            (*((volatile uint8*)0x40030014))
#define SO0_REG                             (*((volatile uint8*)0x40030018))
#define SOL0_REG                            (*((volatile uint8*)0x4003001C))

/* Port Registers */
#define PM14_REG                            (*((volatile uint8*)0x4000020E))
#define PM15_REG                            (*((volatile uint8*)0x4000020F))
#define P14CFG_REG                          (*((volatile uint8*)0x4000038E))
#define P15CFG_REG                          (*((volatile uint8*)0x4000038F))

/* ================= REGISTER BITS ================= */
/* SMRmn bits - Operation Mode Selection */
#define SMRMN_MODE_MASK                     0x07
#define SMRMN_MODE_I2C                      0x02      /* Simplified I2C mode */

/* SSRmn bits - Status Flags */
#define SSRMN_TEND                          (1 << 0)  /* Transfer end */
#define SSRMN_ACKD                          (1 << 1)  /* ACK detected */
#define SSRMN_ACK                           (1 << 2)  /* ACK error */
#define SSRMN_OVR                           (1 << 3)  /* Overflow error */

/* SCRmn bits - Communication Control */
#define SCRMN_IICE                          (1 << 7)  /* I2C Enable */

/* ================= FUNCTION PROTOTYPES ================= */
/* Hardware Abstraction Functions */
void I2C_SimplifiedHw_Init(uint8 ChannelId, uint32 BaudRate);
void I2C_SimplifiedHw_Deinit(uint8 ChannelId);
void I2C_SimplifiedHw_ManualStart(uint8 ChannelId);
void I2C_SimplifiedHw_ManualStop(uint8 ChannelId);
Std_ReturnType I2C_SimplifiedHw_SendByte(uint8 ChannelId, uint8 Data, uint8* AckStatus);
Std_ReturnType I2C_SimplifiedHw_ReceiveByte(uint8 ChannelId, uint8* Data, boolean SendAck);
uint32 I2C_SimplifiedHw_GetStatus(uint8 ChannelId);
void I2C_SimplifiedHw_ClearStatus(uint8 ChannelId, uint32 StatusFlags);

/* Baud Rate Calculation */
Std_ReturnType I2C_SimplifiedHw_CalculateBaudRate(uint32 DesiredBaudRate, uint32* SpsValue);

#endif /* I2C_SIMPLIFIED_HW_H */