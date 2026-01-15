/******************************************************************************
* File: I2c_Hw.h
* Description: Truy cập phần cứng cho I2C Driver
* MCU: BAT32A2x9 Series
******************************************************************************/

#ifndef I2C_HW_H
#define I2C_HW_H

#include "I2c_Types.h"
#include "Mcu.h"

/* Định nghĩa thanh ghi BAT32A2x9 I2C theo datasheet */
typedef struct
{
    volatile uint32 IICAn;      /* IICA shift register n */
    volatile uint32 SVAn;       /* Slave address register n */
    volatile uint32 IICCTLn0;   /* IICA control register n0 */
    volatile uint32 IICCTLn1;   /* IICA control register n1 */
    volatile uint32 IICSn;      /* IICA status register n */
    volatile uint32 IICFn;      /* IICA flag register n */
    volatile uint32 IICWLn;     /* IICA low-level width setting register n */
    volatile uint32 IICWHn;     /* IICA high-level width setting register n */
} I2C_HwRegType;

/* Địa chỉ base thanh ghi I2C */
#define I2C0_BASE_ADDRESS       (0x400A0000UL)
#define I2C1_BASE_ADDRESS       (0x400A1000UL)

/* Bit PER0 (Peripheral Enable Register 0) cho I2C */
#define PER0_IICA0_ENABLE       (0x00000010UL)  /* Bit 4: IICA0 enable */
#define PER0_IICA1_ENABLE       (0x00000020UL)  /* Bit 5: IICA1 enable */

/* Các cờ trong IICSn (Status Register) */
#define IICS_START_CONDITION    (0x00000008UL)  /* Bit 3: START condition generated */
#define IICS_STOP_CONDITION     (0x00000010UL)  /* Bit 4: STOP condition detected */
#define IICS_NACK_RECEIVED      (0x00000020UL)  /* Bit 5: NACK received */
#define IICS_ARBITRATION_LOST   (0x00000040UL)  /* Bit 6: Arbitration lost */
#define IICS_BUS_ERROR          (0x00000080UL)  /* Bit 7: Bus error */
#define IICS_TRANSFER_COMPLETE  (0x00000100UL)  /* Bit 8: Transfer complete */
#define IICS_RX_DATA_READY      (0x00000200UL)  /* Bit 9: Receive data ready */
#define IICS_TX_EMPTY           (0x00000400UL)  /* Bit 10: Transmit buffer empty */

/* Các bit trong IICCTLn0 (Control Register 0) */
#define IICCTL0_IICE            (0x00000080UL)  /* Bit 7: I2C enable */
#define IICCTL0_IICRST          (0x00000040UL)  /* Bit 6: I2C reset */
#define IICCTL0_IICACKT         (0x00000020UL)  /* Bit 5: Acknowledge bit */
#define IICCTL0_IICWM           (0x00000010UL)  /* Bit 4: Wait mode */
#define IICCTL0_IICCSC          (0x00000008UL)  /* Bit 3: Start condition */
#define IICCTL0_IICSP           (0x00000004UL)  /* Bit 2: Stop condition */
#define IICCTL0_IICRSV          (0x00000002UL)  /* Bit 1: Reserved */
#define IICCTL0_IICINT          (0x00000001UL)  /* Bit 0: Interrupt enable */

/* Chế độ hoạt động */
#define IICCTL1_IICM_MASK       (0x000000C0UL)  /* Bits 7-6: Mode selection */
#define IICCTL1_IICM_MASTER     (0x00000000UL)  /* Master mode */
#define IICCTL1_IICM_SLAVE      (0x00000040UL)  /* Slave mode */
#define IICCTL1_IICM_MASTER_ACK (0x00000080UL)  /* Master mode with ACK */

/* Tốc độ I2C */
#define I2C_STANDARD_MODE       100000U         /* 100 kHz */
#define I2C_FAST_MODE           400000U         /* 400 kHz */
#define I2C_FAST_MODE_PLUS      1000000U        /* 1 MHz */

/* Hàm phần cứng */
extern void I2C_Hw_InitChannel(uint8 ChannelId, const I2C_ConfigType* ConfigPtr);
extern void I2C_Hw_DeinitChannel(uint8 ChannelId);
extern void I2C_Hw_StartTransmit(uint8 ChannelId, uint8 SlaveAddress);
extern uint32 I2C_Hw_GetStatus(uint8 ChannelId);
extern void I2C_Hw_ClearStatus(uint8 ChannelId, uint32 StatusFlags);
extern void I2C_Hw_IRQHandler(uint8 ChannelId);
extern void I2C_Hw_WriteData(uint8 ChannelId, uint8 Data);
extern uint8 I2C_Hw_ReadData(uint8 ChannelId);
extern Std_ReturnType I2C_Hw_CalculateBaudRate(uint32 DesiredBaudRate, uint32 SystemClock, uint32* IICWLn, uint32* IICWHn);

#endif /* I2C_HW_H */