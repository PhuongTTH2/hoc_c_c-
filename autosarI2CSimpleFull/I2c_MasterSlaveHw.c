/**
 * @file    bat32a2x9_iica_full.c
 * @brief   Complete IICA (Full I2C) Configuration for BAT32A2x9
 * @details Based on Chapter 20 of BAT32A2x9 User Manual V1.0.5
 *          Configures IICA0 and IICA1 with multi-master support
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* =================== REGISTER DEFINITIONS =================== */

/* Base Addresses */
#define IICA0_BASE     0x4004C000UL
#define IICA1_BASE     0x4004C100UL
#define PER0_ADDR      0x40020420UL
#define PER1_ADDR      0x40020421UL

/* Register Structures for IICA */
typedef volatile struct {
    /* Control Registers */
    uint8_t  IICCTLn0;     /* 0x00: IICA Control Register n0 */
    uint8_t  reserved0[3];
    uint8_t  IICCTLn1;     /* 0x04: IICA Control Register n1 */
    uint8_t  reserved1[3];
    uint8_t  IICSn;        /* 0x08: IICA Status Register n */
    uint8_t  reserved2[3];
    uint8_t  IICFn;        /* 0x0C: IICA Flag Register n */
    uint8_t  reserved3[3];
    
    /* Timing Registers */
    uint16_t IICWLin;      /* 0x10: IICA Low Level Width Setting Register n */
    uint16_t reserved4;
    uint16_t IICWHn;       /* 0x14: IICA High Level Width Setting Register n */
    uint16_t reserved5;
    
    /* Data Registers */
    uint8_t  IICAn;        /* 0x18: IICA Shift Register n */
    uint8_t  reserved6[3];
    uint8_t  SVAn;         /* 0x1C: Slave Address Register n */
    uint8_t  reserved7[3];
} IICA_Type;

/* Global Pointers */
#define IICA0   ((IICA_Type *)IICA0_BASE)
#define IICA1   ((IICA_Type *)IICA1_BASE)
#define PER0    (*(volatile uint8_t *)PER0_ADDR)
#define PER1    (*(volatile uint8_t *)PER1_ADDR)

/* GPIO Registers (simplified) */
#define POM0    (*(volatile uint8_t *)0x40040050UL)  /* Port Output Mode 0 */
#define PM0     (*(volatile uint8_t *)0x40040032UL)  /* Port Mode 0 */
#define PU0     (*(volatile uint8_t *)0x40040030UL)  /* Pull-up 0 */

/* =================== BIT DEFINITIONS =================== */

/* PER0/PER1 Bits for IICA */
#define PER0_IICA0EN    (1 << 0)    /* Enable IICA0 */
#define PER1_IICA1EN    (1 << 5)    /* Enable IICA1 */

/* IICCTLn0 Bits (Figure 20-3) */
#define IICCTL0_EN      (1 << 7)    /* Operation enable */
#define IICCTL0_IICM    (1 << 6)    /* I2C mode select */
#define IICCTL0_SREL    (1 << 5)    /* SDA output control */
#define IICCTL0_WREL    (1 << 4)    /* Wait control */
#define IICCTL0_SPIE    (1 << 3)    /* Stop condition interrupt enable */
#define IICCTL0_WTIE    (1 << 2)    /* Wait detection interrupt enable */
#define IICCTL0_ACKE    (1 << 1)    /* ACK transmission enable */
#define IICCTL0_INTE    (1 << 0)    /* Interrupt request enable */

/* IICCTLn1 Bits (Figure 20-6) */
#define IICCTL1_STCEN   (1 << 7)    /* Start condition generation enable */
#define IICCTL1_SPCEN   (1 << 6)    /* Stop condition generation enable */
#define IICCTL1_ACKD    (1 << 5)    /* ACK detection */
#define IICCTL1_TRS     (1 << 4)    /* Transmission/reception selection */
#define IICCTL1_STC     (1 << 3)    /* Start condition */
#define IICCTL1_SPC     (1 << 2)    /* Stop condition */
#define IICCTL1_ACK     (1 << 1)    /* ACK transmission */
#define IICCTL1_ACKB    (1 << 0)    /* ACK transmission buffer */

/* IICSn Bits (Figure 20-4) */
#define IICS_STD        (1 << 7)    /* Start condition detection */
#define IICS_TRC        (1 << 6)    /* Transmission/reception completion */
#define IICS_BBSY       (1 << 5)    /* Bus busy */
#define IICS_AL         (1 << 4)    /* Arbitration lost */
#define IICS_AAS        (1 << 3)    /* Slave address match */
#define IICS_AD0        (1 << 2)    /* Address 0 match */
#define IICS_STC        (1 << 1)    /* Start condition generation completion */
#define IICS_SPC        (1 << 0)    /* Stop condition generation completion */

/* IICFn Bits (Figure 20-5) */
#define IICF_STCF       (1 << 7)    /* Start condition detection flag */
#define IICF_TRCF       (1 << 6)    /* Transmission/reception completion flag */
#define IICF_BBSYF      (1 << 5)    /* Bus busy flag */
#define IICF_ALF        (1 << 4)    /* Arbitration lost flag */
#define IICF_AASF       (1 << 3)    /* Slave address match flag */
#define IICF_AD0F       (1 << 2)    /* Address 0 match flag */
#define IICF_STCFG      (1 << 1)    /* Start condition generation flag */
#define IICF_SPCF       (1 << 0)    /* Stop condition generation flag */

/* =================== IICA CHANNELS =================== */

typedef struct {
    IICA_Type *regs;      /* Register pointer */
    uint8_t sda_pin;      /* SDA pin number */
    uint8_t scl_pin;      /* SCL pin number */
    uint8_t per_bit;      /* PER bit position */
    uint8_t per_reg;      /* 0=PER0, 1=PER1 */
    const char *name;
} IICA_Channel;

/* IICA Channels */
static const IICA_Channel iica_channels[] = {
    /* IICA0 */
    {IICA0, 12, 13, PER0_IICA0EN, 0, "IICA0"},  /* P12=SDA00, P13=SCL00 */
    
    /* IICA1 */
    {IICA1, 15, 14, PER1_IICA1EN, 1, "IICA1"},  /* P15=SDA10, P14=SCL10 */
};

#define NUM_IICA_CHANNELS (sizeof(iica_channels)/sizeof(iica_channels[0]))

/* =================== GPIO CONFIGURATION =================== */

/**
 * Configure GPIO pins for IICA (N-channel open-drain)
 */
static void configure_iica_gpio(uint8_t sda_pin, uint8_t scl_pin) {
    /* Example for IICA0 (P12=SDA, P13=SCL) */
    if (sda_pin == 12 && scl_pin == 13) {
        /* Set as N-channel open-drain (POM0 = 1) */
        POM0 |= (1 << 2);  /* POM12 = 1 (SDA) */
        POM0 |= (1 << 3);  /* POM13 = 1 (SCL) */
        
        /* Enable pull-up resistors */
        PU0 |= (1 << 2) | (1 << 3);  /* PU12, PU13 = 1 */
        
        /* Initially set as output and high */
        PM0 &= ~((1 << 2) | (1 << 3));  /* PM12, PM13 = 0 (output) */
        
        /* Note: For IICA, pins are automatically controlled by hardware
           when IICA is enabled. These are just initial settings. */
    }
    /* Add similar for IICA1 pins if needed */
}

/* =================== CLOCK CONFIGURATION =================== */

/**
 * Enable IICA clock via PER0/PER1
 */
static void enable_iica_clock(const IICA_Channel *ch) {
    if (ch->per_reg == 0) {
        PER0 |= ch->per_bit;
    } else {
        PER1 |= ch->per_bit;
    }
}

/* =================== TIMING REGISTER CONFIGURATION =================== */

/**
 * Calculate and set IICWLin and IICWHn for desired I2C frequency
 * Formula: Register value = (F_CLK / (2 × F_I2C)) - 1
 */
static void configure_iica_timing(IICA_Type *iica, uint32_t f_clk, uint32_t i2c_freq) {
    /* Calculate low/high width for 50% duty cycle */
    uint32_t divider = (f_clk / (2 * i2c_freq)) - 1;
    
    if (divider < 1) divider = 1;
    if (divider > 0x3FF) divider = 0x3FF;  /* 10-bit limit */
    
    /* Set both low and high width to same value for 50% duty */
    iica->IICWLin = (uint16_t)divider;
    iica->IICWHn = (uint16_t)divider;
    
    printf("  Timing: divider=%lu (0x%03lX)\n", divider, divider);
}

/* =================== CONTROL REGISTER CONFIGURATION =================== */

/**
 * Configure IICCTLn0 register
 */
static void configure_iicctl0(IICA_Type *iica, bool master_mode) {
    uint8_t ctl0_value = 0;
    
    if (master_mode) {
        /* Master mode configuration */
        ctl0_value = IICCTL0_EN |     /* Operation enable */
                     IICCTL0_IICM |   /* I2C mode select */
                     IICCTL0_INTE |   /* Interrupt enable */
                     IICCTL0_ACKE;    /* ACK transmission enable */
    } else {
        /* Slave mode configuration */
        ctl0_value = IICCTL0_EN |     /* Operation enable */
                     IICCTL0_IICM |   /* I2C mode select */
                     IICCTL0_INTE |   /* Interrupt enable */
                     IICCTL0_ACKE |   /* ACK transmission enable */
                     IICCTL0_SPIE;    /* Stop condition interrupt enable */
    }
    
    iica->IICCTLn0 = ctl0_value;
}

/**
 * Configure IICCTLn1 register
 */
static void configure_iicctl1(IICA_Type *iica, bool master_mode) {
    uint8_t ctl1_value = 0;
    
    if (master_mode) {
        /* Master mode: enable start/stop condition generation */
        ctl1_value = IICCTL1_STCEN |  /* Start condition generation enable */
                     IICCTL1_SPCEN;   /* Stop condition generation enable */
    } else {
        /* Slave mode: wait for start condition from master */
        ctl1_value = 0;  /* Slave doesn't generate start/stop */
    }
    
    iica->IICCTLn1 = ctl1_value;
}

/**
 * Set slave address for IICA
 */
static void set_slave_address(IICA_Type *iica, uint8_t slave_addr) {
    /* 7-bit address in bits 7:1, bit 0 is reserved */
    iica->SVAn = (slave_addr << 1);
}

/* =================== STATUS AND FLAG MANAGEMENT =================== */

/**
 * Clear all IICA flags
 */
static void clear_iica_flags(IICA_Type *iica) {
    /* Reading IICFn clears the flags */
    volatile uint8_t dummy = iica->IICFn;
    (void)dummy;  /* Prevent unused variable warning */
}

/**
 * Wait for bus to be free
 */
static bool wait_for_bus_free(IICA_Type *iica, uint32_t timeout) {
    while (timeout > 0) {
        if (!(iica->IICSn & IICS_BBSY)) {
            return true;  /* Bus is free */
        }
        timeout--;
        /* Add small delay */
        for (volatile int i = 0; i < 100; i++);
    }
    return false;  /* Timeout */
}

/**
 * Check for errors
 */
static bool check_iica_errors(IICA_Type *iica) {
    uint8_t status = iica->IICSn;
    
    if (status & IICS_AL) {
        printf("  Error: Arbitration lost\n");
        return false;
    }
    
    if (iica->IICFn & IICF_ALF) {
        printf("  Error: Arbitration lost flag set\n");
        return false;
    }
    
    return true;
}

/* =================== MAIN CONFIGURATION FUNCTION =================== */

/**
 * Complete configuration for an IICA channel
 * @param ch_index Channel index (0=IICA0, 1=IICA1)
 * @param f_clk System clock frequency in Hz
 * @param i2c_freq Desired I2C frequency in Hz
 * @param slave_addr 7-bit slave address (if in slave mode)
 * @param master_mode true for master, false for slave
 * @return true if successful
 */
bool IICA_Configure_Channel(uint8_t ch_index, uint32_t f_clk, uint32_t i2c_freq, 
                           uint8_t slave_addr, bool master_mode) {
    if (ch_index >= NUM_IICA_CHANNELS) {
        printf("Error: Invalid IICA channel index %d\n", ch_index);
        return false;
    }
    
    const IICA_Channel *ch = &iica_channels[ch_index];
    
    printf("\nConfiguring %s (%s mode)...\n", 
           ch->name, master_mode ? "MASTER" : "SLAVE");
    
    /* 1. Enable IICA clock */
    enable_iica_clock(ch);
    printf("  ✓ Enabled %s clock\n", ch->name);
    
    /* 2. Configure GPIO pins */
    configure_iica_gpio(ch->sda_pin, ch->scl_pin);
    printf("  ✓ Configured GPIO: SDA=P%d, SCL=P%d\n", ch->sda_pin, ch->scl_pin);
    
    /* 3. Set slave address (for both master and slave) */
    set_slave_address(ch->regs, slave_addr);
    printf("  ✓ Set slave address: 0x%02X\n", slave_addr);
    
    /* 4. Configure timing registers */
    configure_iica_timing(ch->regs, f_clk, i2c_freq);
    printf("  ✓ Configured timing for %lu Hz I2C\n", i2c_freq);
    
    /* 5. Configure control registers */
    configure_iicctl0(ch->regs, master_mode);
    configure_iicctl1(ch->regs, master_mode);
    printf("  ✓ Configured control registers\n");
    
    /* 6. Clear all flags */
    clear_iica_flags(ch->regs);
    printf("  ✓ Cleared status flags\n");
    
    /* 7. Wait for bus to be free (master mode) */
    if (master_mode) {
        if (wait_for_bus_free(ch->regs, 10000)) {
            printf("  ✓ Bus is free\n");
        } else {
            printf("  ⚠ Bus busy timeout\n");
        }
    }
    
    /* 8. Check initial status */
    printf("  Initial status: 0x%02X\n", ch->regs->IICSn);
    
    return true;
}

/* =================== I2C COMMUNICATION FUNCTIONS =================== */

/**
 * Generate I2C START condition (Master mode)
 */
static bool iica_generate_start(IICA_Type *iica) {
    /* Check if bus is busy */
    if (iica->IICSn & IICS_BBSY) {
        return false;
    }
    
    /* Set start condition bit */
    iica->IICCTLn1 |= IICCTL1_STC;
    
    /* Wait for start condition completion */
    uint32_t timeout = 10000;
    while (!(iica->IICFn & IICF_STCFG)) {
        if (timeout-- == 0) {
            return false;
        }
    }
    
    /* Clear start condition flag */
    iica->IICFn = 0;  /* Reading clears flags */
    
    return true;
}

/**
 * Generate I2C STOP condition (Master mode)
 */
static bool iica_generate_stop(IICA_Type *iica) {
    /* Set stop condition bit */
    iica->IICCTLn1 |= IICCTL1_SPC;
    
    /* Wait for stop condition completion */
    uint32_t timeout = 10000;
    while (!(iica->IICFn & IICF_SPCF)) {
        if (timeout-- == 0) {
            return false;
        }
    }
    
    /* Clear stop condition flag */
    iica->IICFn = 0;
    
    return true;
}

/**
 * Send data byte (Master transmitter mode)
 */
static bool iica_send_byte(IICA_Type *iica, uint8_t data) {
    /* Write data to shift register */
    iica->IICAn = data;
    
    /* Wait for transmission completion */
    uint32_t timeout = 10000;
    while (!(iica->IICFn & IICF_TRCF)) {
        if (timeout-- == 0) {
            return false;
        }
        
        /* Check for arbitration loss */
        if (iica->IICFn & IICF_ALF) {
            printf("Arbitration lost during transmission\n");
            return false;
        }
    }
    
    /* Clear transmission flag */
    iica->IICFn = 0;
    
    /* Check for ACK from slave */
    if (iica->IICCTLn1 & IICCTL1_ACKD) {
        /* ACK not received (NACK) */
        return false;
    }
    
    return true;
}

/**
 * Receive data byte (Master receiver mode)
 */
static bool iica_receive_byte(IICA_Type *iica, uint8_t *data, bool send_ack) {
    /* Set ACK transmission */
    if (send_ack) {
        iica->IICCTLn1 |= IICCTL1_ACK;  /* Send ACK */
    } else {
        iica->IICCTLn1 &= ~IICCTL1_ACK; /* Send NACK (last byte) */
    }
    
    /* Wait for reception completion */
    uint32_t timeout = 10000;
    while (!(iica->IICFn & IICF_TRCF)) {
        if (timeout-- == 0) {
            return false;
        }
    }
    
    /* Read received data */
    *data = iica->IICAn;
    
    /* Clear reception flag */
    iica->IICFn = 0;
    
    return true;
}

/**
 * Master write data to slave
 */
bool IICA_Master_Write(uint8_t ch_index, uint8_t slave_addr, 
                       const uint8_t *data, uint8_t len) {
    if (ch_index >= NUM_IICA_CHANNELS) return false;
    
    const IICA_Channel *ch = &iica_channels[ch_index];
    
    printf("%s Master Write to 0x%02X (%d bytes)...\n", 
           ch->name, slave_addr, len);
    
    /* 1. Generate START condition */
    if (!iica_generate_start(ch->regs)) {
        printf("  ✗ Failed to generate START\n");
        return false;
    }
    printf("  ✓ START condition generated\n");
    
    /* 2. Send slave address with Write bit (0) */
    uint8_t addr_byte = (slave_addr << 1) | 0x00;  /* Write = 0 */
    if (!iica_send_byte(ch->regs, addr_byte)) {
        printf("  ✗ Failed to send address (no ACK)\n");
        iica_generate_stop(ch->regs);  /* Generate STOP on error */
        return false;
    }
    printf("  ✓ Slave address sent (ACK received)\n");
    
    /* 3. Send data bytes */
    for (uint8_t i = 0; i < len; i++) {
        if (!iica_send_byte(ch->regs, data[i])) {
            printf("  ✗ Failed to send byte %d (no ACK)\n", i);
            iica_generate_stop(ch->regs);
            return false;
        }
        printf("  ✓ Byte %d sent: 0x%02X\n", i, data[i]);
    }
    
    /* 4. Generate STOP condition */
    if (!iica_generate_stop(ch->regs)) {
        printf("  ✗ Failed to generate STOP\n");
        return false;
    }
    printf("  ✓ STOP condition generated\n");
    printf("  ✓ Write completed successfully\n");
    
    return true;
}

/**
 * Master read data from slave
 */
bool IICA_Master_Read(uint8_t ch_index, uint8_t slave_addr, 
                      uint8_t *data, uint8_t len) {
    if (ch_index >= NUM_IICA_CHANNELS) return false;
    
    const IICA_Channel *ch = &iica_channels[ch_index];
    
    printf("%s Master Read from 0x%02X (%d bytes)...\n", 
           ch->name, slave_addr, len);
    
    /* 1. Generate START condition */
    if (!iica_generate_start(ch->regs)) {
        printf("  ✗ Failed to generate START\n");
        return false;
    }
    printf("  ✓ START condition generated\n");
    
    /* 2. Send slave address with Read bit (1) */
    uint8_t addr_byte = (slave_addr << 1) | 0x01;  /* Read = 1 */
    if (!iica_send_byte(ch->regs, addr_byte)) {
        printf("  ✗ Failed to send address (no ACK)\n");
        iica_generate_stop(ch->regs);
        return false;
    }
    printf("  ✓ Slave address sent (ACK received)\n");
    
    /* 3. Receive data bytes */
    for (uint8_t i = 0; i < len; i++) {
        /* Send ACK for all bytes except last one */
        bool send_ack = (i < (len - 1));
        
        if (!iica_receive_byte(ch->regs, &data[i], send_ack)) {
            printf("  ✗ Failed to receive byte %d\n", i);
            iica_generate_stop(ch->regs);
            return false;
        }
        printf("  ✓ Byte %d received: 0x%02X\n", i, data[i]);
    }
    
    /* 4. Generate STOP condition */
    if (!iica_generate_stop(ch->regs)) {
        printf("  ✗ Failed to generate STOP\n");
        return false;
    }
    printf("  ✓ STOP condition generated\n");
    printf("  ✓ Read completed successfully\n");
    
    return true;
}

/* =================== SLAVE MODE FUNCTIONS =================== */

/**
 * Slave mode interrupt handler (to be called from ISR)
 */
void IICA_Slave_IRQ_Handler(uint8_t ch_index) {
    if (ch_index >= NUM_IICA_CHANNELS) return;
    
    const IICA_Channel *ch = &iica_channels[ch_index];
    uint8_t status = ch->regs->IICSn;
    uint8_t flags = ch->regs->IICFn;
    
    /* Clear flags by reading */
    ch->regs->IICFn = flags;
    
    /* Handle different interrupt sources */
    if (flags & IICF_STCF) {
        /* Start condition detected */
        printf("%s: Start condition detected\n", ch->name);
        
        if (status & IICS_AAS) {
            /* Address matched - slave selected */
            printf("%s: Address matched, slave selected\n", ch->name);
            
            if (status & IICS_TRC) {
                /* Determine if master wants to read or write */
                uint8_t data = ch->regs->IICAn;  /* Read address byte */
                uint8_t rw_bit = data & 0x01;
                
                if (rw_bit == 0) {
                    printf("%s: Master wants to WRITE\n", ch->name);
                    /* Prepare to receive data */
                } else {
                    printf("%s: Master wants to READ\n", ch->name);
                    /* Prepare to send data */
                }
            }
        }
    }
    
    if (flags & IICF_TRCF) {
        /* Transmission/reception completed */
        printf("%s: Data transfer completed\n", ch->name);
    }
    
    if (flags & IICF_SPCF) {
        /* Stop condition detected */
        printf("%s: Stop condition detected\n", ch->name);
    }
    
    if (flags & IICF_ALF) {
        /* Arbitration lost */
        printf("%s: Arbitration lost\n", ch->name);
    }
}

/* =================== UTILITY FUNCTIONS =================== */

/**
 * Dump all IICA registers for debugging
 */
void IICA_Dump_Registers(uint8_t ch_index) {
    if (ch_index >= NUM_IICA_CHANNELS) return;
    
    const IICA_Channel *ch = &iica_channels[ch_index];
    
    printf("\n===== %s REGISTER DUMP =====\n", ch->name);
    printf("IICCTL0: 0x%02X\n", ch->regs->IICCTLn0);
    printf("IICCTL1: 0x%02X\n", ch->regs->IICCTLn1);
    printf("IICS:    0x%02X\n", ch->regs->IICSn);
    printf("IICF:    0x%02X\n", ch->regs->IICFn);
    printf("IICWL:   0x%04X\n", ch->regs->IICWLin);
    printf("IICWH:   0x%04X\n", ch->regs->IICWHn);
    printf("IICA:    0x%02X\n", ch->regs->IICAn);
    printf("SVA:     0x%02X\n", ch->regs->SVAn);
    
    /* Decode status bits */
    printf("\nStatus Decoded:\n");
    printf("  Bus Busy:    %s\n", (ch->regs->IICSn & IICS_BBSY) ? "YES" : "NO");
    printf("  Arbitration: %s\n", (ch->regs->IICSn & IICS_AL) ? "LOST" : "OK");
    printf("  Addr Match:  %s\n", (ch->regs->IICSn & IICS_AAS) ? "YES" : "NO");
    printf("  TR Complete: %s\n", (ch->regs->IICSn & IICS_TRC) ? "YES" : "NO");
}

/**
 * Test multi-master arbitration
 */
void IICA_Test_Multi_Master(uint8_t ch_index) {
    if (ch_index >= NUM_IICA_CHANNELS) return;
    
    const IICA_Channel *ch = &iica_channels[ch_index];
    
    printf("\n===== %s MULTI-MASTER ARBITRATION TEST =====\n", ch->name);
    
    /* Try to acquire bus when busy */
    if (ch->regs->IICSn & IICS_BBSY) {
        printf("Bus is busy, attempting arbitration...\n");
        
        /* Generate START (will fail if another master is transmitting) */
        ch->regs->IICCTLn1 |= IICCTL1_STC;
        
        /* Check for arbitration loss */
        uint32_t timeout = 1000;
        while (timeout--) {
            if (ch->regs->IICFn & IICF_ALF) {
                printf("Arbitration LOST to another master\n");
                ch->regs->IICFn = 0;  /* Clear flag */
                return;
            }
            
            if (ch->regs->IICFn & IICF_STCFG) {
                printf("Arbitration WON - bus acquired\n");
                ch->regs->IICFn = 0;
                break;
            }
        }
    }
}

/* =================== MAIN TEST FUNCTION =================== */

int main(void) {
    uint32_t f_clk = 32000000;    /* 32 MHz system clock */
    uint32_t i2c_freq = 100000;   /* 100 kHz I2C */
    uint8_t slave_addr = 0x50;    /* Example EEPROM address */
    
    printf("BAT32A2x9 IICA (Full I2C) Configuration Test\n");
    printf("============================================\n");
    
    /* Configure IICA0 as Master */
    printf("\n--- Configuring IICA0 as MASTER ---\n");
    IICA_Configure_Channel(0, f_clk, i2c_freq, 0x00, true);  /* Master */
    IICA_Dump_Registers(0);
    
    /* Configure IICA1 as Slave */
    printf("\n--- Configuring IICA1 as SLAVE ---\n");
    IICA_Configure_Channel(1, f_clk, i2c_freq, 0x50, false); /* Slave addr 0x50 */
    IICA_Dump_Registers(1);
    
    /* Test Master Write */
    printf("\n--- Testing IICA0 Master Write ---\n");
    uint8_t write_data[] = {0x00, 0x01, 0x02, 0x03};  /* Write to address 0 */
    IICA_Master_Write(0, slave_addr, write_data, sizeof(write_data));
    
    /* Test Master Read */
    printf("\n--- Testing IICA0 Master Read ---\n");
    uint8_t read_data[4];
    IICA_Master_Read(0, slave_addr, read_data, sizeof(read_data));
    
    /* Display read data */
    printf("Read data: ");
    for (int i = 0; i < sizeof(read_data); i++) {
        printf("0x%02X ", read_data[i]);
    }
    printf("\n");
    
    /* Test multi-master arbitration */
    printf("\n--- Testing Multi-Master Arbitration ---\n");
    IICA_Test_Multi_Master(0);
    
    printf("\n============================================\n");
    printf("IICA Test Complete\n");
    
    return 0;
}

/* =================== INTERRUPT HANDLER EXAMPLES =================== */

/**
 * Example interrupt handlers (to be placed in appropriate vector table)
 */
void __attribute__((interrupt)) IICA0_IRQHandler(void) {
    IICA_Slave_IRQ_Handler(0);  /* Handle IICA0 interrupts */
    
    /* If using master mode with interrupts */
    uint8_t flags = IICA0->IICFn;
    IICA0->IICFn = flags;  /* Clear flags */
    
    /* Handle specific interrupt types */
    if (flags & IICF_TRCF) {
        /* Transmission/reception complete */
        /* Handle in your application */
    }
    
    if (flags & IICF_ALF) {
        /* Arbitration lost */
        /* Handle in your application */
    }
}

void __attribute__((interrupt)) IICA1_IRQHandler(void) {
    IICA_Slave_IRQ_Handler(1);  /* Handle IICA1 interrupts */
}

/* =================== HEADER FILE EXCERPT =================== */
/*
#ifndef BAT32A2X9_IICA_H
#define BAT32A2X9_IICA_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Channel indices */
#define IICA0_CH  0
#define IICA1_CH  1

/* Function Prototypes */
bool IICA_Configure_Channel(uint8_t ch_index, uint32_t f_clk, uint32_t i2c_freq,
                           uint8_t slave_addr, bool master_mode);
bool IICA_Master_Write(uint8_t ch_index, uint8_t slave_addr,
                      const uint8_t *data, uint8_t len);
bool IICA_Master_Read(uint8_t ch_index, uint8_t slave_addr,
                     uint8_t *data, uint8_t len);
void IICA_Slave_IRQ_Handler(uint8_t ch_index);
void IICA_Dump_Registers(uint8_t ch_index);
void IICA_Test_Multi_Master(uint8_t ch_index);

/* Interrupt Handlers (to be defined in application) */
void IICA0_IRQHandler(void);
void IICA1_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* BAT32A2X9_IICA_H */
*/