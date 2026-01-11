#include "I2c_Hw.h"
#include "I2c_Regs.h"

/* IICA Register Structure */
typedef struct {
    volatile uint8_t IICCTL0;
    volatile uint8_t IICCTL1;
    volatile uint8_t IICS;
    volatile uint8_t IICF;
    volatile uint8_t IICWL;
    volatile uint8_t IICWH;
    volatile uint8_t SVA0;
    volatile uint8_t SVA1;
    volatile uint8_t IICA;
    uint8_t RESERVED[7];
} I2c_HwRegType;

#define I2C0_BASE   ((I2c_HwRegType*)0x40041800U)
#define I2C1_BASE   ((I2c_HwRegType*)0x40046000U)

/* I2C_HwInit */
Std_ReturnType I2c_HwInit(uint8 Channel, const I2c_ChannelConfigType* Config) {
    I2c_HwRegType* reg;
    
    if (Channel == 0) {
        reg = I2C0_BASE;
        /* Enable clock */
        *(volatile uint8_t*)0x40020410 |= (1 << 7);
        /* Configure pins P60/P61 */
        *(volatile uint8_t*)0x40040060 = 0x03; /* PM60/PM61 = 1 */
        *(volatile uint8_t*)0x40040050 = 0x03; /* POM60/POM61 = 1 */
    } else if (Channel == 1) {
        reg = I2C1_BASE;
        /* Enable clock */
        *(volatile uint8_t*)0x40020410 |= (1 << 6);
        /* Configure pins P62/P63 */
        *(volatile uint8_t*)0x40040062 = 0x03;
        *(volatile uint8_t*)0x40040052 = 0x03;
    } else {
        return E_NOT_OK;
    }
    
    /* Reset I2C */
    reg->IICCTL0 = 0x40;
    
    /* Set baud rate */
    uint32 period = 64000000U / (Config->I2cBaudRate * 1000) / 2;
    reg->IICWL = (uint8)period;
    reg->IICWH = (uint8)period;
    
    /* Enable I2C */
    reg->IICCTL0 = 0x80;
    
    /* Clear flags */
    reg->IICF = 0xFF;
    
    return E_OK;
}

/* I2c_HwDeInit */
void I2c_HwDeInit(uint8 Channel) {
    I2c_HwRegType* reg = (Channel == 0) ? I2C0_BASE : I2C1_BASE;
    reg->IICCTL0 = 0x00;
}

/* I2c_HwStartTransmit */
Std_ReturnType I2c_HwStartTransmit(uint8 Channel, uint8 Address, boolean IsWrite) {
    I2c_HwRegType* reg = (Channel == 0) ? I2C0_BASE : I2C1_BASE;
    
    /* Wait if busy */
    while (reg->IICS & 0x01);
    
    /* Set address */
    reg->SVA0 = (Address << 1);
    
    /* Set direction */
    if (IsWrite) {
        reg->IICCTL0 &= ~0x04;
    } else {
        reg->IICCTL0 |= 0x04;
    }
    
    /* Generate start */
    reg->IICCTL0 |= 0x01;
    
    return E_OK;
}

/* I2c_HwSendByte */
Std_ReturnType I2c_HwSendByte(uint8 Channel, uint8 Data) {
    I2c_HwRegType* reg = (Channel == 0) ? I2C0_BASE : I2C1_BASE;
    
    /* Wait TX ready */
    while (!(reg->IICF & 0x02));
    
    /* Send data */
    reg->IICA = Data;
    
    /* Wait completion */
    while (!(reg->IICF & 0x04));
    
    /* Check NACK */
    if (reg->IICS & 0x10) {
        return E_NOT_OK;
    }
    
    return E_OK;
}

/* I2c_HwReceiveByte */
Std_ReturnType I2c_HwReceiveByte(uint8 Channel, uint8* Data, boolean SendAck) {
    I2c_HwRegType* reg = (Channel == 0) ? I2C0_BASE : I2C1_BASE;
    
    /* Configure ACK */
    if (SendAck) {
        reg->IICCTL0 |= 0x20;
    } else {
        reg->IICCTL0 &= ~0x20;
    }
    
    /* Wait RX ready */
    while (!(reg->IICF & 0x08));
    
    /* Read data */
    *Data = reg->IICA;
    
    return E_OK;
}

/* I2c_HwGenerateStop */
Std_ReturnType I2c_HwGenerateStop(uint8 Channel) {
    I2c_HwRegType* reg = (Channel == 0) ? I2C0_BASE : I2C1_BASE;
    reg->IICCTL0 |= 0x02;
    return E_OK;
}

/* I2c_HwIsBusBusy */
boolean I2c_HwIsBusBusy(uint8 Channel) {
    I2c_HwRegType* reg = (Channel == 0) ? I2C0_BASE : I2C1_BASE;
    return (reg->IICS & 0x01) ? TRUE : FALSE;
}

/* I2c_HwIsTransferComplete */
boolean I2c_HwIsTransferComplete(uint8 Channel) {
    I2c_HwRegType* reg = (Channel == 0) ? I2C0_BASE : I2C1_BASE;
    return (reg->IICF & 0x04) ? TRUE : FALSE;
}

/* Interrupt Handlers */
void I2C0_IRQHandler(void) {
    /* Handle interrupts */
}

void I2C1_IRQHandler(void) {
    /* Handle interrupts */
}

/**
 * @file    I2c_Hw.c
 * @brief   Hardware Implementation for BAT32A2x9 I2C - FIXED
 */

#include "I2c_Hw.h"
#include "I2c_Regs.h"

/* Register Definitions */
#define PER0_ADDR        ((volatile uint8_t*)0x40020410)
#define PM0_ADDR         ((volatile uint8_t*)0x40040030)
#define PM1_ADDR         ((volatile uint8_t*)0x40040031)
#define PM2_ADDR         ((volatile uint8_t*)0x40040032)
#define PM3_ADDR         ((volatile uint8_t*)0x40040033)
#define PM4_ADDR         ((volatile uint8_t*)0x40040034)
#define PM5_ADDR         ((volatile uint8_t*)0x40040035)
#define PM6_ADDR         ((volatile uint8_t*)0x40040036)
#define PM7_ADDR         ((volatile uint8_t*)0x40040037)
#define PM14_ADDR        ((volatile uint8_t*)0x4004003E)

#define POM3_ADDR        ((volatile uint8_t*)0x40040053)  /* For P30 */
#define POM5_ADDR        ((volatile uint8_t*)0x40040055)  /* For P50-P55 */
#define POM14_ADDR       ((volatile uint8_t*)0x4004005E)  /* For P142-P144 */

#define PMC6_ADDR        ((volatile uint8_t*)0x40040066)  /* For P60-P67 */
#define PMC14_ADDR       ((volatile uint8_t*)0x4004006E)  /* For P140-P147 */

#define PIOR0_ADDR       ((volatile uint8_t*)0x40040877)

/**
 * @brief Configure I2C pins - METHOD 1: Use default pins (P60/P61)
 */
static void I2c_ConfigurePins_Default(uint8 Channel) {
    if (Channel == 0) {
        /* I2C0: P60(SCL), P61(SDA) */
        
        /* 1. Set as INPUT mode (required for I2C) */
        *PM6_ADDR |= 0x03;      /* PM60=1, PM61=1 */
        
        /* 2. CANNOT set open-drain (no POM register for Port 6)
           Workaround: Use normal output and control in software */
        
        /* 3. Set digital mode */
        *PMC6_ADDR &= ~0x03;    /* PMC60=0, PMC61=0 */
        
        /* 4. IMPORTANT: Set output latches HIGH for open-drain emulation */
        /* P6 register address: 0x40040006 */
        *(volatile uint8_t*)0x40040006 |= 0x03;  /* P60=1, P61=1 */
        
        /* 5. NO internal pull-up for Port 6! Use external 4.7kΩ resistors */
        
        /* 6. Set PIOR for default pins */
        *PIOR0_ADDR &= ~0x0C;   /* PIOR02=0, PIOR03=0: SCL=P60, SDA=P61 */
        
    } else if (Channel == 1) {
        /* I2C1: P62(SCL), P63(SDA) */
        *PM6_ADDR |= 0x0C;      /* PM62=1, PM63=1 */
        *PMC6_ADDR &= ~0x0C;    /* PMC62=0, PMC63=0 */
        *(volatile uint8_t*)0x40040006 |= 0x0C;  /* P62=1, P63=1 */
    }
}

/**
 * @brief Configure I2C pins - METHOD 2: Use alternate pins (P14/P15)
 * RECOMMENDED: Vì có đầy đủ POM register cho open-drain
 */
static void I2c_ConfigurePins_Alternate(uint8 Channel) {
    if (Channel == 0) {
        /* I2C0 Alternate: P14(SCL), P15(SDA) */
        
        /* 1. Set as INPUT mode */
        *PM14_ADDR |= 0x06;     /* PM14=1 (bit1), PM15=1 (bit2) */
        
        /* 2. SET OPEN-DRAIN (possible with POM14 register!) */
        *POM14_ADDR |= 0x06;    /* POM14=1 (bit1), POM15=1 (bit2) */
        
        /* 3. Set digital mode */
        *PMC14_ADDR &= ~0x06;   /* PMC14=0, PMC15=0 */
        
        /* 4. No internal pull-up (external required) */
        /* PU14 register: 0x4004003E - bit1, bit2 */
        *(volatile uint8_t*)0x4004003E &= ~0x06;
        
        /* 5. Set PIOR for alternate pins */
        *PIOR0_ADDR |= 0x0C;    /* PIOR02=1, PIOR03=1: SCL=P14, SDA=P15 */
        
        /* 6. Set output latches HIGH */
        /* P1 register: 0x40040001 - bit4=P14, bit5=P15 */
        *(volatile uint8_t*)0x40040001 |= 0x30;
    }
}

/**
 * @brief Configure I2C pins - METHOD 3: Use pins with POM (P30/P50)
 */
static void I2c_ConfigurePins_WithPOM(uint8 Channel) {
    if (Channel == 0) {
        /* Option: Use P30(SCL), P50(SDA) - both have POM registers */
        
        /* Configure P30 (SCL) */
        *PM3_ADDR |= 0x01;      /* PM30=1 */
        *POM3_ADDR |= 0x01;     /* POM30=1 (open-drain) */
        
        /* Configure P50 (SDA) */
        *PM5_ADDR |= 0x01;      /* PM50=1 */
        *POM5_ADDR |= 0x01;     /* POM50=1 (open-drain) */
        
        /* Set PIOR accordingly */
        /* Need to check PIOR1 register for P30/P50 assignment */
        volatile uint8_t* PIOR1 = (volatile uint8_t*)0x40040879;
        *PIOR1 = 0x01;          /* Example: TAO=P30, depends on your need */
    }
}

/* I2C_HwInit - UPDATED */
Std_ReturnType I2c_HwInit(uint8 Channel, const I2c_ChannelConfigType* Config) {
    /* 1. Enable peripheral clock */
    if (Channel == 0) {
        *PER0_ADDR |= (1 << 7);  /* Enable IICA0 clock */
    } else if (Channel == 1) {
        *PER0_ADDR |= (1 << 6);  /* Enable IICA1 clock */
    } else {
        return E_NOT_OK;
    }
    
    /* 2. Configure pins - CHOOSE ONE METHOD: */
    
    /* METHOD A: Default pins (P60/P61) - NO OPEN-DRAIN HARDWARE SUPPORT */
    // I2c_ConfigurePins_Default(Channel);
    
    /* METHOD B: Alternate pins (P14/P15) - HAS OPEN-DRAIN SUPPORT (RECOMMENDED) */
    I2c_ConfigurePins_Alternate(Channel);
    
    /* METHOD C: Other pins with POM (P30/P50) */
    // I2c_ConfigurePins_WithPOM(Channel);
    
    /* 3. Initialize I2C hardware registers */
    I2c_HwRegType* reg;
    if (Channel == 0) {
        reg = (I2c_HwRegType*)0x40041800;  /* IICA0 */
    } else {
        reg = (I2c_HwRegType*)0x40046000;  /* IICA1 */
    }
    
    /* Reset I2C */
    reg->IICCTL0 = 0x40;  /* Reset bit */
    for (volatile uint32 i = 0; i < 100; i++); /* Wait */
    
    /* Set baud rate */
    uint32 period = 64000000UL / (Config->I2cBaudRate * 1000) / 2;
    if (period > 255) period = 255;
    if (period < 2) period = 2;
    
    reg->IICWL = (uint8)period;
    reg->IICWH = (uint8)period;
    
    /* Enable I2C */
    reg->IICCTL0 = 0x80;  /* IICE=1 */
    
    /* Clear flags */
    reg->IICF = 0xFF;
    
    return E_OK;
}

/**
 * @brief Emulate open-drain for pins without POM register
 * @note For P60/P61 which don't have POM register
 */
void I2c_EmulateOpenDrain(uint8 Channel, uint8 Pin, boolean High) {
    if (Channel == 0) {
        volatile uint8_t* PM6 = (volatile uint8_t*)0x40040066;
        volatile uint8_t* P6  = (volatile uint8_t*)0x40040006;
        
        if (Pin == 0) {  /* P60 */
            if (High) {
                /* Release bus: set as INPUT (high-Z) */
                *PM6 |= 0x01;   /* PM60=1 (input) */
            } else {
                /* Pull low: set as OUTPUT LOW */
                *PM6 &= ~0x01;  /* PM60=0 (output) */
                *P6 &= ~0x01;   /* P60=0 (low) */
            }
        } else if (Pin == 1) {  /* P61 */
            if (High) {
                *PM6 |= 0x02;   /* PM61=1 (input) */
            } else {
                *PM6 &= ~0x02;  /* PM61=0 (output) */
                *P6 &= ~0x02;   /* P61=0 (low) */
            }
        }
    }
}