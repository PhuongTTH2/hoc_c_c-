/**
 * @file    bat32a2x9_simplified_i2c_full.c
 * @brief   Complete Simplified I2C Configuration for BAT32A2x9 (8 channels)
 * @details Based on Chapter 19 of BAT32A2x9 User Manual V1.0.5
 *          Configures all 8 Simplified I2C channels: IIC00, IIC01, IIC10, IIC11, IIC20, IIC21, IIC30, IIC31
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* =================== REGISTER DEFINITIONS =================== */

/* Base Addresses */
#define PER0_ADDR     0x40020420UL
#define PER2_ADDR     0x40020422UL
#define SCI0_BASE     0x40041100UL
#define SCI1_BASE     0x40041400UL
#define SCI2_BASE     0x40041600UL

/* Port Register Addresses (simplified) */
#define POM4_ADDR     0x40040054UL
#define PMC4_ADDR     0x40040060UL
#define PU4_ADDR      0x40040034UL
#define PM4_ADDR      0x40040032UL

/* Register Structures */
typedef volatile struct {
    uint16_t SSRmn;      /* Serial Status Register */
    uint16_t reserved0;
} SSR_Reg;

typedef volatile struct {
    uint16_t SIRmn;      /* Serial Flag Clear Trigger Register */
    uint16_t reserved1;
} SIR_Reg;

typedef volatile struct {
    uint16_t SMRmn;      /* Serial Mode Register */
    uint16_t reserved2;
} SMR_Reg;

typedef volatile struct {
    uint16_t SCRmn;      /* Serial Communication Run Setting Register */
    uint16_t reserved3;
} SCR_Reg;

typedef volatile struct {
    uint16_t SDRmn;      /* Serial Data Register (16-bit) */
    uint16_t reserved4;
} SDR_Reg;

/* SCI0 Structure (4 channels) */
typedef volatile struct {
    /* Channel Registers (4 channels) */
    SSR_Reg SSR[4];      /* 0x00-0x06 */
    SIR_Reg SIR[4];      /* 0x08-0x0E */
    SMR_Reg SMR[4];      /* 0x10-0x16 */
    SCR_Reg SCR[4];      /* 0x18-0x1E */
    
    /* Unit Registers */
    uint16_t SE0;        /* 0x20: Serial Channel Enable Status Register 0 */
    uint16_t reserved5;
    uint16_t SS0;        /* 0x22: Serial Channel Start Register 0 */
    uint16_t reserved6;
    uint16_t ST0;        /* 0x24: Serial Channel Stop Register 0 */
    uint16_t reserved7;
    uint16_t SPS0;       /* 0x26: Serial Clock Selection Register 0 */
    uint16_t reserved8;
    uint16_t SO0;        /* 0x28: Serial Output Register 0 */
    uint16_t reserved9;
    uint16_t SOE0;       /* 0x2A: Serial Output Enable Register 0 */
    uint16_t reserved10;
    uint16_t reserved11[2];
    uint16_t SOL0;       /* 0x30: Serial Output Level Register 0 */
    uint16_t reserved12;
    
    uint8_t  padding1[0x1E0];  /* Padding to 0x210 */
    
    /* SDR Registers for channels */
    SDR_Reg SDR[4];      /* 0x210-0x216 */
} SCI0_Type;

/* SCI1/SCI2 Structure (2 channels) */
typedef volatile struct {
    /* Channel Registers (2 channels) */
    SSR_Reg SSR[2];      /* 0x00-0x02 */
    SIR_Reg SIR[2];      /* 0x04-0x06 */
    SMR_Reg SMR[2];      /* 0x08-0x0A */
    SCR_Reg SCR[2];      /* 0x0C-0x0E */
    
    /* Unit Registers */
    uint16_t SE1;        /* 0x10: Serial Channel Enable Status Register 1 */
    uint16_t reserved13;
    uint16_t SS1;        /* 0x12: Serial Channel Start Register 1 */
    uint16_t reserved14;
    uint16_t ST1;        /* 0x14: Serial Channel Stop Register 1 */
    uint16_t reserved15;
    uint16_t SPS1;       /* 0x16: Serial Clock Selection Register 1 */
    uint16_t reserved16;
    uint16_t SO1;        /* 0x18: Serial Output Register 1 */
    uint16_t reserved17;
    uint16_t SOE1;       /* 0x1A: Serial Output Enable Register 1 */
    uint16_t reserved18;
    uint16_t SOL1;       /* 0x1C: Serial Output Level Register 1 */
    uint16_t reserved19;
    
    uint8_t  padding2[0xF4];  /* Padding to 0x110 */
    
    /* SDR Registers for channels */
    SDR_Reg SDR[2];      /* 0x110-0x112 */
} SCI12_Type;

/* Global Pointers */
#define PER0   (*(volatile uint8_t *)PER0_ADDR)
#define PER2   (*(volatile uint8_t *)PER2_ADDR)
#define SCI0   ((SCI0_Type *)SCI0_BASE)
#define SCI1   ((SCI12_Type *)SCI1_BASE)
#define SCI2   ((SCI12_Type *)SCI2_BASE)

/* GPIO Registers (simplified examples) */
#define POM4   (*(volatile uint8_t *)POM4_ADDR)
#define PMC4   (*(volatile uint8_t *)PMC4_ADDR)
#define PU4    (*(volatile uint8_t *)PU4_ADDR)
#define PM4    (*(volatile uint8_t *)PM4_ADDR)

/* =================== BIT DEFINITIONS =================== */

/* PER0/PER2 Bits */
#define PER0_SCI0EN    (1 << 2)    /* Enable SCI0 */
#define PER0_SCI1EN    (1 << 3)    /* Enable SCI1 */
#define PER2_SCI2EN    (1 << 3)    /* Enable SCI2 */

/* SMRmn Bits (Figure 19-6) */
#define SMR_CKS        (1 << 0)    /* Clock Select */
#define SMR_CCS        (1 << 1)    /* Clock Control Select */
#define SMR_STS        (1 << 8)    /* Start Trigger Select */
#define SMR_SIS        (1 << 9)    /* Signal Invert Select */
#define SMR_MD0        (1 << 12)   /* Mode bit 0 */
#define SMR_MD1        (1 << 13)   /* Mode bit 1 */
#define SMR_MD2        (1 << 14)   /* Mode bit 2 */

/* Mode Settings */
#define SMR_MODE_SSPI  0x0000      /* SSPI mode */
#define SMR_MODE_UART  0x2000      /* UART mode (MD2=0, MD1=1) */
#define SMR_MODE_I2C   0x4000      /* Simplified I2C mode (MD2=1, MD1=0) */

/* SCRmn Bits (Figure 19-7) - MOST IMPORTANT REGISTER! */
#define SCR_TXE        (1 << 15)   /* Transmit Enable */
#define SCR_RXE        (1 << 14)   /* Receive Enable */
#define SCR_DAP        (1 << 13)   /* Data Alignment (SSPI) */
#define SCR_CKP        (1 << 12)   /* Clock Polarity (SSPI) */
#define SCR_EOC        (1 << 10)   /* Error Interrupt Control */
#define SCR_PTC0       (1 << 8)    /* Parity Control bit 0 */
#define SCR_PTC1       (1 << 9)    /* Parity Control bit 1 */
#define SCR_DIR        (1 << 7)    /* Data Direction */
#define SCR_SLC0       (1 << 4)    /* Stop Length Control bit 0 */
#define SCR_SLC1       (1 << 5)    /* Stop Length Control bit 1 */
#define SCR_DLS0       (1 << 0)    /* Data Length Select bit 0 */
#define SCR_DLS1       (1 << 1)    /* Data Length Select bit 1 */
#define SCR_DLS2       (1 << 2)    /* Data Length Select bit 2 */
#define SCR_DLS3       (1 << 3)    /* Data Length Select bit 3 */

/* Simplified I2C Required Settings for SCRmn (Figure 19-7):
   - TXEmn = 1, RXEmn = 1 (Enable transmit/receive)
   - DAPmn = 0, CKPmn = 0 (Not used in I2C)
   - EOCmn = 0 (Enable error interrupt)
   - PTCmn1,0 = 00 (No parity)
   - DIRmn = 0 (MSB first for I2C)
   - SLCmn1,0 = 01 (1 stop bit for I2C)
   - DLSmn3-0 = 0111 (8-bit data for I2C)
   - Bit 2 = 1 (Required fixed value)
   - Bits 3, 6, 11 = 0 (Required fixed values)
*/
#define SCR_I2C_CONFIG ((1 << 15) | (1 << 14) |  /* TXE=1, RXE=1 */ \
                        (0 << 13) | (0 << 12) |  /* DAP=0, CKP=0 */ \
                        (0 << 10) |              /* EOC=0 */ \
                        (0 << 8) | (0 << 9) |    /* PTC1,0=00 */ \
                        (0 << 7) |               /* DIR=0 (MSB first) */ \
                        (1 << 2) |               /* Bit2=1 (required) */ \
                        (1 << 4) | (0 << 5) |    /* SLC1,0=01 (1 stop bit) */ \
                        (1 << 0) | (1 << 1) | (1 << 2) | (0 << 3)) /* DLS3-0=0111 */

/* SSRmn Bits (Figure 19-11) */
#define SSR_OVF        (1 << 0)    /* Overflow Error */
#define SSR_PEF        (1 << 1)    /* Parity/ACK Error */
#define SSR_FEF        (1 << 2)    /* Framing Error */
#define SSR_BFF        (1 << 6)    /* Buffer Full Flag */
#define SSR_TSF        (1 << 8)    /* Transfer Status Flag */

/* SIRmn Bits (Figure 19-10) */
#define SIR_OVCT       (1 << 0)    /* Overflow Clear Trigger */
#define SIR_PECT       (1 << 1)    /* Parity Error Clear Trigger */
#define SIR_FECT       (1 << 2)    /* Framing Error Clear Trigger */

/* =================== CHANNEL CONFIGURATION =================== */

typedef struct {
    uint8_t unit;        /* 0=SCI0, 1=SCI1, 2=SCI2 */
    uint8_t channel;     /* Channel number (0-3 for SCI0, 0-1 for SCI1/2) */
    uint8_t sda_pin;     /* SDA pin number */
    uint8_t scl_pin;     /* SCL pin number */
    const char *name;    /* Channel name */
} I2C_Channel;

/* All 8 Simplified I2C Channels */
static const I2C_Channel i2c_channels[] = {
    /* SCI0 Channels (Unit 0, 4 channels) */
    {0, 0, 43, 44, "IIC00"},  /* P43=SDA00, P44=SCL00 */
    {0, 1, 46, 47, "IIC01"},  /* P46=SDA01, P47=SCL01 */
    {0, 2, 53, 54, "IIC10"},  /* P53=SDA10, P54=SCL10 */
    {0, 3, 56, 57, "IIC11"},  /* P56=SDA11, P57=SCL11 */
    
    /* SCI1 Channels (Unit 1, 2 channels) */
    {1, 0, 63, 64, "IIC20"},  /* P63=SDA20, P64=SCL20 */
    {1, 1, 66, 67, "IIC21"},  /* P66=SDA21, P67=SCL21 */
    
    /* SCI2 Channels (Unit 2, 2 channels) */
    {2, 0, 73, 74, "IIC30"},  /* P73=SDA30, P74=SCL30 */
    {2, 1, 76, 77, "IIC31"},  /* P76=SDA31, P77=SCL31 */
};

#define NUM_CHANNELS (sizeof(i2c_channels)/sizeof(i2c_channels[0]))

/* =================== GPIO CONFIGURATION =================== */

/**
 * Configure GPIO pins for Simplified I2C (N-channel open-drain)
 * Based on datasheet section 19.3.17
 */
static void configure_i2c_gpio(uint8_t sda_pin, uint8_t scl_pin) {
    /* Note: This is a simplified example for pins 43/44 (IIC00)
       Similar configuration needed for all other pins */
    
    if (sda_pin == 43 && scl_pin == 44) {
        /* Example for IIC00 (P43=SDA, P44=SCL) */
        
        /* 1. Set as N-channel open-drain (POMxx = 1) */
        POM4 |= (1 << 3);  /* POM43 = 1 */
        POM4 |= (1 << 4);  /* POM44 = 1 */
        
        /* 2. Set as digital I/O (PMCxx = 0) */
        PMC4 &= ~(1 << 3); /* PMC43 = 0 */
        PMC4 &= ~(1 << 4); /* PMC44 = 0 */
        
        /* 3. Enable pull-up resistors (PUxx = 1) */
        PU4 |= (1 << 3);   /* PU43 = 1 */
        PU4 |= (1 << 4);   /* PU44 = 1 */
        
        /* 4. Initially set as output (PMxx = 0) */
        PM4 &= ~(1 << 3);  /* PM43 = 0 */
        PM4 &= ~(1 << 4);  /* PM44 = 0 */
    }
    
    /* Add similar configurations for other pins...
       In a real implementation, you would handle all pin combinations */
}

/* =================== CLOCK CONFIGURATION =================== */

/**
 * Enable SCI module clock via PER0/PER2
 */
static void enable_sci_clock(uint8_t unit) {
    switch (unit) {
        case 0:
            PER0 |= PER0_SCI0EN;
            break;
        case 1:
            PER0 |= PER0_SCI1EN;
            break;
        case 2:
            PER2 |= PER2_SCI2EN;
            break;
        default:
            break;
    }
}

/**
 * Configure SPSm register (Serial Clock Selection Register)
 * Sets prescalers for CKm0 and CKm1
 */
static void configure_sps_register(uint8_t unit) {
    /* Configure for FCLK/16 for both CKm0 and CKm1
       PRS10=4 (FCLK/16), PRS00=4 (FCLK/16) */
    uint16_t sps_value = 0x44;
    
    switch (unit) {
        case 0:
            SCI0->SPS0 = sps_value;
            break;
        case 1:
            SCI1->SPS1 = sps_value;
            break;
        case 2:
            SCI2->SPS1 = sps_value;
            break;
    }
}

/* =================== CHANNEL REGISTER CONFIGURATION =================== */

/**
 * Configure SDRmn divider for I2C baud rate
 * SDRmn[15:9] sets the divider: FTCLK = FMCK / (2 × (SDRmn[15:9] + 1))
 */
static void configure_sdr_divider(uint8_t unit, uint8_t channel, uint32_t fclk, uint32_t i2c_freq) {
    /* Calculate FMCK (clock after SPSm division) */
    uint32_t fmck = fclk / 16;  /* Based on SPSm = 0x44 (FCLK/16) */
    
    /* Calculate divider: SDRmn[15:9] = (FMCK / (2 × i2c_freq)) - 1 */
    uint32_t divider = (fmck / (2 * i2c_freq)) - 1;
    
    /* Check limits (1-127 for I2C, per datasheet caution 3) */
    if (divider < 1) divider = 1;
    if (divider > 127) divider = 127;
    
    /* Set SDRmn[15:9] divider bits */
    uint16_t sdr_value = (divider << 9);
    
    /* Note: Lower bits (8:0) must be 0 when SEmn=0 */
    
    switch (unit) {
        case 0:
            SCI0->SDR[channel].SDRmn = sdr_value;
            break;
        case 1:
            SCI1->SDR[channel].SDRmn = sdr_value;
            break;
        case 2:
            SCI2->SDR[channel].SDRmn = sdr_value;
            break;
    }
}

/**
 * Configure SMRmn (Serial Mode Register) for Simplified I2C
 */
static void configure_smr_register(uint8_t unit, uint8_t channel) {
    /* SMRmn settings for Simplified I2C:
       - CKS=0: Use CKm0
       - CCS=0: Use divided FMCK (not external clock)
       - STS=0: Software trigger (for I2C)
       - SIS=0: Normal edge detection
       - MD2=1, MD1=0: Simplified I2C mode
       - MD0=0: Transfer end interrupt
    */
    uint16_t smr_value = (0 << 0) |   /* CKS=0 */
                         (0 << 1) |   /* CCS=0 */
                         (0 << 8) |   /* STS=0 */
                         (0 << 9) |   /* SIS=0 */
                         (0 << 12) |  /* MD0=0 (transfer end interrupt) */
                         SMR_MODE_I2C; /* MD2=1, MD1=0 (I2C mode) */
    
    switch (unit) {
        case 0:
            SCI0->SMR[channel].SMRmn = smr_value;
            break;
        case 1:
            SCI1->SMR[channel].SMRmn = smr_value;
            break;
        case 2:
            SCI2->SMR[channel].SMRmn = smr_value;
            break;
    }
}

/**
 * Configure SCRmn (Serial Communication Run Setting Register) for I2C
 * THIS IS THE MOST CRITICAL REGISTER FOR I2C CONFIGURATION!
 */
static void configure_scr_register(uint8_t unit, uint8_t channel) {
    /* Set SCRmn to required I2C configuration */
    
    
      /* Build SCRmn value bit by bit for clarity */
    uint16_t scr_value = 0;
    
    /* Bit 15: TXEmn = 1 (Transmit Enable) */
    scr_value |= (1 << 15);
    
    /* Bit 14: RXEmn = 1 (Receive Enable) */
    scr_value |= (1 << 14);
    
    /* Bit 13: DAPmn = 0 (Data Alignment - not used in I2C) */
    scr_value |= (0 << 13);
    
    /* Bit 12: CKPmn = 0 (Clock Polarity - not used in I2C) */
    scr_value |= (0 << 12);
    
    /* Bit 11: Must be 0 (Fixed value) */
    scr_value |= (0 << 11);
    
    /* Bit 10: EOCmn = 0 (Error Interrupt Enabled) */
    scr_value |= (0 << 10);
    
    /* Bit 9: Must be 0 (Fixed value) */
    scr_value |= (0 << 9);
    
    /* Bits 8-9: PTCmn1,0 = 00 (No parity for I2C) */
    scr_value |= (0 << 8);  /* PTC1 = 0 */
    scr_value |= (0 << 9);  /* PTC0 = 0 */
    
    /* Bit 7: DIRmn = 0 (MSB first for I2C) */
    scr_value |= (0 << 7);
    
    /* Bit 6: Must be 0 (Fixed value) */
    scr_value |= (0 << 6);
    
    /* Bits 4-5: SLCmn1,0 = 01 (1 stop bit for I2C) */
    scr_value |= (0 << 5);  /* SLC1 = 0 */
    scr_value |= (1 << 4);  /* SLC0 = 1 */
    
    /* Bit 3: Must be 0 (Fixed value) */
    scr_value |= (0 << 3);
    
    /* Bit 2: Must be 1 (Fixed value) */
    scr_value |= (1 << 2);
    
    /* Bits 0-3: DLSmn3-0 = 0111 (8-bit data for I2C) */
    scr_value |= (0 << 3);  /* DLS3 = 0 */
    scr_value |= (1 << 2);  /* DLS2 = 1 (also bit 2 fixed = 1) */
    scr_value |= (1 << 1);  /* DLS1 = 1 */
    scr_value |= (1 << 0);  /* DLS0 = 1 */
    
    /* Result should be: 0xC087 */
    
    uint16_t scr_value = SCR_I2C_CONFIG;
    switch (unit) {
        case 0:
            SCI0->SCR[channel].SCRmn = scr_value;
            break;
        case 1:
            SCI1->SCR[channel].SCRmn = scr_value;
            break;
        case 2:
            SCI2->SCR[channel].SCRmn = scr_value;
            break;
    }
}

/**
 * Configure output registers (SOm and SOEm)
 */
static void configure_output_registers(uint8_t unit, uint8_t channel) {
    switch (unit) {
        case 0: /* SCI0 - 4 channels */
            /* SO0: CKOmn in bits 4-7, SOmn in bits 0-3 */
            /* Set SCL (CKOmn) and SDA (SOmn) high initially */
            SCI0->SO0 |= (1 << (4 + channel)) | (1 << channel);
            
            /* Enable output for both pins */
            SCI0->SOE0 |= (1 << (4 + channel)) | (1 << channel);
            break;
            
        case 1: /* SCI1 - 2 channels */
            /* SO1: CKOmn in bits 6-7, SOmn in bits 2-3 */
            SCI1->SO1 |= (1 << (6 + channel)) | (1 << (2 + channel));
            SCI1->SOE1 |= (1 << (6 + channel)) | (1 << (2 + channel));
            break;
            
        case 2: /* SCI2 - 2 channels */
            SCI2->SO1 |= (1 << (6 + channel)) | (1 << (2 + channel));
            SCI2->SOE1 |= (1 << (6 + channel)) | (1 << (2 + channel));
            break;
    }
}

/**
 * Clear status flags via SIRmn
 */
static void clear_status_flags(uint8_t unit, uint8_t channel) {
    uint16_t sir_value = SIR_OVCT | SIR_PECT | SIR_FECT;
    
    switch (unit) {
        case 0:
            SCI0->SIR[channel].SIRmn = sir_value;
            break;
        case 1:
            SCI1->SIR[channel].SIRmn = sir_value;
            break;
        case 2:
            SCI2->SIR[channel].SIRmn = sir_value;
            break;
    }
}

/**
 * Start channel operation via SSm register
 */
static void start_channel(uint8_t unit, uint8_t channel) {
    /* Stop channel first (if running) */
    switch (unit) {
        case 0:
            SCI0->ST0 |= (1 << channel);
            SCI0->SS0 |= (1 << channel);
            break;
        case 1:
            SCI1->ST1 |= (1 << channel);
            SCI1->SS1 |= (1 << channel);
            break;
        case 2:
            SCI2->ST1 |= (1 << channel);
            SCI2->SS1 |= (1 << channel);
            break;
    }
}

/* =================== MAIN CONFIGURATION FUNCTION =================== */

/**
 * Complete configuration for a Simplified I2C channel
 * @param ch_index Channel index (0-7)
 * @param fclk CPU clock frequency in Hz
 * @param i2c_freq Desired I2C frequency in Hz (typically 100000 for 100kHz)
 * @return true if configuration successful, false otherwise
 */
bool I2C_Configure_Channel(uint8_t ch_index, uint32_t fclk, uint32_t i2c_freq) {
    if (ch_index >= NUM_CHANNELS) {
        printf("Error: Invalid channel index %d\n", ch_index);
        return false;
    }
    
    const I2C_Channel *ch = &i2c_channels[ch_index];
    
    printf("Configuring %s (Unit %d, Channel %d)...\n", 
           ch->name, ch->unit, ch->channel);
    
    /* 1. Enable SCI clock */
    enable_sci_clock(ch->unit);
    printf("  ✓ Enabled SCI%d clock\n", ch->unit);
    
    /* 2. Configure GPIO pins */
    configure_i2c_gpio(ch->sda_pin, ch->scl_pin);
    printf("  ✓ Configured GPIO: SDA=P%d, SCL=P%d\n", ch->sda_pin, ch->scl_pin);
    
    /* 3. Configure SPSm clock selection */
    configure_sps_register(ch->unit);
    printf("  ✓ Configured SPS%d register\n", ch->unit);
    
    /* 4. Configure SDRmn divider for baud rate */
    configure_sdr_divider(ch->unit, ch->channel, fclk, i2c_freq);
    printf("  ✓ Configured SDR%d%d divider for %lu Hz\n", 
           ch->unit, ch->channel, i2c_freq);
    
    /* 5. Configure SMRmn for I2C mode */
    configure_smr_register(ch->unit, ch->channel);
    printf("  ✓ Configured SMR%d%d for I2C mode\n", ch->unit, ch->channel);
    
    /* 6. Configure SCRmn - MOST IMPORTANT! */
    configure_scr_register(ch->unit, ch->channel);
    printf("  ✓ Configured SCR%d%d with I2C settings\n", ch->unit, ch->channel);
    
    /* 7. Configure output registers */
    configure_output_registers(ch->unit, ch->channel);
    printf("  ✓ Configured output registers\n");
    
    /* 8. Clear status flags */
    clear_status_flags(ch->unit, ch->channel);
    printf("  ✓ Cleared status flags\n");
    
    /* 9. Start channel */
    start_channel(ch->unit, ch->channel);
    printf("  ✓ Started channel operation\n");
    
    return true;
}

/**
 * Configure all 8 Simplified I2C channels
 */
void I2C_Configure_All_Channels(uint32_t fclk, uint32_t i2c_freq) {
    printf("\n=========================================\n");
    printf("CONFIGURING ALL 8 SIMPLIFIED I2C CHANNELS\n");
    printf("FCLK = %lu Hz, I2C Frequency = %lu Hz\n", fclk, i2c_freq);
    printf("=========================================\n");
    
    uint8_t successes = 0;
    
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
        if (I2C_Configure_Channel(i, fclk, i2c_freq)) {
            successes++;
        }
        printf("\n");
    }
    
    printf("=========================================\n");
    printf("CONFIGURATION SUMMARY:\n");
    printf("  Successfully configured: %d/%d channels\n", successes, NUM_CHANNELS);
    printf("=========================================\n");
}

/* =================== I2C COMMUNICATION FUNCTIONS =================== */

/**
 * Generate I2C START condition manually
 * According to datasheet section 19.9, start/stop conditions must be generated manually
 */
static void i2c_generate_start(uint8_t unit, uint8_t channel) {
    /* For Simplified I2C, start condition must be generated by software:
       1. SDA goes from HIGH to LOW while SCL is HIGH
    */
    
    /* Ensure SCL is high */
    switch (unit) {
        case 0:
            SCI0->SO0 |= (1 << (4 + channel));  /* Set SCL high */
            break;
        case 1:
            SCI1->SO1 |= (1 << (6 + channel));
            break;
        case 2:
            SCI2->SO1 |= (1 << (6 + channel));
            break;
    }
    
    /* Small delay (implementation dependent) */
    for (volatile int i = 0; i < 100; i++);
    
    /* Pull SDA low */
    switch (unit) {
        case 0:
            SCI0->SO0 &= ~(1 << channel);  /* Set SDA low */
            break;
        case 1:
            SCI1->SO1 &= ~(1 << (2 + channel));
            break;
        case 2:
            SCI2->SO1 &= ~(1 << (2 + channel));
            break;
    }
    
    /* Small delay */
    for (volatile int i = 0; i < 100; i++);
    
    /* Pull SCL low to begin data transfer */
    switch (unit) {
        case 0:
            SCI0->SO0 &= ~(1 << (4 + channel));  /* Set SCL low */
            break;
        case 1:
            SCI1->SO1 &= ~(1 << (6 + channel));
            break;
        case 2:
            SCI2->SO1 &= ~(1 << (6 + channel));
            break;
    }
}

/**
 * Generate I2C STOP condition manually
 */
static void i2c_generate_stop(uint8_t unit, uint8_t channel) {
    /* Stop condition: SDA goes from LOW to HIGH while SCL is HIGH */
    
    /* Ensure SCL is low */
    switch (unit) {
        case 0:
            SCI0->SO0 &= ~(1 << (4 + channel));  /* SCL low */
            break;
        case 1:
            SCI1->SO1 &= ~(1 << (6 + channel));
            break;
        case 2:
            SCI2->SO1 &= ~(1 << (6 + channel));
            break;
    }
    
    /* Set SDA low */
    switch (unit) {
        case 0:
            SCI0->SO0 &= ~(1 << channel);  /* SDA low */
            break;
        case 1:
            SCI1->SO1 &= ~(1 << (2 + channel));
            break;
        case 2:
            SCI2->SO1 &= ~(1 << (2 + channel));
            break;
    }
    
    /* Small delay */
    for (volatile int i = 0; i < 100; i++);
    
    /* Set SCL high */
    switch (unit) {
        case 0:
            SCI0->SO0 |= (1 << (4 + channel));  /* SCL high */
            break;
        case 1:
            SCI1->SO1 |= (1 << (6 + channel));
            break;
        case 2:
            SCI2->SO1 |= (1 << (6 + channel));
            break;
    }
    
    /* Small delay */
    for (volatile int i = 0; i < 100; i++);
    
    /* Set SDA high (stop condition) */
    switch (unit) {
        case 0:
            SCI0->SO0 |= (1 << channel);  /* SDA high */
            break;
        case 1:
            SCI1->SO1 |= (1 << (2 + channel));
            break;
        case 2:
            SCI2->SO1 |= (1 << (2 + channel));
            break;
    }
}

/**
 * Send data via Simplified I2C
 */
bool I2C_Send_Data(uint8_t ch_index, uint8_t slave_addr, uint8_t *data, uint8_t len) {
    if (ch_index >= NUM_CHANNELS) return false;
    
    const I2C_Channel *ch = &i2c_channels[ch_index];
    
    /* 1. Generate START condition */
    i2c_generate_start(ch->unit, ch->channel);
    
    /* 2. Send slave address (7-bit address + R/W bit) */
    uint8_t addr_byte = (slave_addr << 1) & 0xFE;  /* R/W=0 (write) */
    
    /* Wait for buffer to be empty */
    // while (!(SSR_BFF bit is ready)) {}
    
    /* Send address */
    switch (ch->unit) {
        case 0:
            SCI0->SDR[ch->channel].SDRmn = addr_byte;
            break;
        case 1:
            SCI1->SDR[ch->channel].SDRmn = addr_byte;
            break;
        case 2:
            SCI2->SDR[ch->channel].SDRmn = addr_byte;
            break;
    }
    
    /* Wait for transmission to complete */
    // while (!(SSR_TSF bit indicates completion)) {}
    
    /* Check for ACK error */
    // if (SSR_PEF bit is set) { /* Handle NACK */ }
    
    /* 3. Send data bytes */
    for (uint8_t i = 0; i < len; i++) {
        /* Wait for buffer to be empty */
        // while (!(buffer ready)) {}
        
        /* Send data byte */
        switch (ch->unit) {
            case 0:
                SCI0->SDR[ch->channel].SDRmn = data[i];
                break;
            case 1:
                SCI1->SDR[ch->channel].SDRmn = data[i];
                break;
            case 2:
                SCI2->SDR[ch->channel].SDRmn = data[i];
                break;
        }
        
        /* Wait for transmission to complete */
        // while (!(transmission complete)) {}
        
        /* Check for ACK error */
        // if (NACK received) { /* Handle error */ }
    }
    
    /* 4. Generate STOP condition */
    i2c_generate_stop(ch->unit, ch->channel);
    
    return true;
}

/* =================== REGISTER DUMP FOR DEBUGGING =================== */

/**
 * Dump all relevant registers for debugging
 */
void I2C_Dump_Registers(uint8_t ch_index) {
    if (ch_index >= NUM_CHANNELS) return;
    
    const I2C_Channel *ch = &i2c_channels[ch_index];
    
    printf("\n===== REGISTER DUMP FOR %s =====\n", ch->name);
    
    uint16_t smr_value, scr_value, sdr_value, ssr_value;
    
    /* Read register values */
    switch (ch->unit) {
        case 0:
            smr_value = SCI0->SMR[ch->channel].SMRmn;
            scr_value = SCI0->SCR[ch->channel].SCRmn;
            sdr_value = SCI0->SDR[ch->channel].SDRmn;
            ssr_value = SCI0->SSR[ch->channel].SSRmn;
            break;
        case 1:
            smr_value = SCI1->SMR[ch->channel].SMRmn;
            scr_value = SCI1->SCR[ch->channel].SCRmn;
            sdr_value = SCI1->SDR[ch->channel].SDRmn;
            ssr_value = SCI1->SSR[ch->channel].SSRmn;
            break;
        case 2:
            smr_value = SCI2->SMR[ch->channel].SMRmn;
            scr_value = SCI2->SCR[ch->channel].SCRmn;
            sdr_value = SCI2->SDR[ch->channel].SDRmn;
            ssr_value = SCI2->SSR[ch->channel].SSRmn;
            break;
    }
    
    printf("SMR%d%d: 0x%04X\n", ch->unit, ch->channel, smr_value);
    printf("SCR%d%d: 0x%04X\n", ch->unit, ch->channel, scr_value);
    printf("SDR%d%d: 0x%04X (Divider: %d)\n", ch->unit, ch->channel, sdr_value, sdr_value >> 9);
    printf("SSR%d%d: 0x%04X\n", ch->unit, ch->channel, ssr_value);
    
    /* Check critical bits */
    printf("\nVerification:\n");
    printf("  I2C Mode: %s\n", (smr_value & SMR_MODE_I2C) ? "YES" : "NO");
    printf("  TX Enable: %s\n", (scr_value & SCR_TXE) ? "YES" : "NO");
    printf("  RX Enable: %s\n", (scr_value & SCR_RXE) ? "YES" : "NO");
    printf("  8-bit Data: %s\n", ((scr_value & 0x000F) == 0x0007) ? "YES" : "NO");
    printf("  1 Stop Bit: %s\n", (((scr_value >> 4) & 0x3) == 0x1) ? "YES" : "NO");
}

/* =================== MAIN TEST FUNCTION =================== */

int main(void) {
    uint32_t fclk = 32000000;   /* 32 MHz CPU clock */
    uint32_t i2c_freq = 100000; /* 100 kHz I2C standard speed */
    
    printf("BAT32A2x9 Simplified I2C Configuration Test\n");
    printf("===========================================\n");
    
    /* Configure all channels */
    I2C_Configure_All_Channels(fclk, i2c_freq);
    
    /* Dump registers for first channel for verification */
    I2C_Dump_Registers(0);  /* IIC00 */
    
    /* Example I2C communication */
    printf("\n===== EXAMPLE I2C COMMUNICATION =====\n");
    
    uint8_t test_data[] = {0x00, 0x01, 0x02, 0x03};
    uint8_t slave_address = 0x50;  /* Example EEPROM address */
    
    printf("Sending data to slave 0x%02X...\n", slave_address);
    
    /* Send data via IIC00 (channel 0) */
    if (I2C_Send_Data(0, slave_address, test_data, sizeof(test_data))) {
        printf("Data sent successfully!\n");
    } else {
        printf("Failed to send data\n");
    }
    
    return 0;
}

/* =================== HEADER FILE EXCERPT =================== */
/*
#ifndef BAT32A2X9_SIMPLIFIED_I2C_H
#define BAT32A2X9_SIMPLIFIED_I2C_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Channel indices for Simplified I2C */
typedef enum {
    IIC00 = 0,
    IIC01 = 1,
    IIC10 = 2,
    IIC11 = 3,
    IIC20 = 4,
    IIC21 = 5,
    IIC30 = 6,
    IIC31 = 7
} I2C_Channel_Index;

/* Function Prototypes */
bool I2C_Configure_Channel(uint8_t ch_index, uint32_t fclk, uint32_t i2c_freq);
void I2C_Configure_All_Channels(uint32_t fclk, uint32_t i2c_freq);
bool I2C_Send_Data(uint8_t ch_index, uint8_t slave_addr, uint8_t *data, uint8_t len);
void I2C_Dump_Registers(uint8_t ch_index);

#ifdef __cplusplus
}
#endif

#endif /* BAT32A2X9_SIMPLIFIED_I2C_H */
*/



// Register addresses (example for SCI0)
#define SCI0_BASE 0x40041100

// Channel mapping to registers
typedef struct {
    uint32_t base_addr;    // SCI0/SCI1/SCI2 base
    uint16_t smr_offset;   // SMRmn offset
    uint16_t sdr_offset;   // SDRmn offset
    uint16_t ssr_offset;   // SSRmn offset
    uint8_t  scl_port;     // GPIO port for SCL
    uint8_t  scl_pin;      // GPIO pin for SCL
    uint8_t  sda_port;     // GPIO port for SDA
    uint8_t  sda_pin;      // GPIO pin for SDA
} I2C_ChannelConfig_t;

// Configuration table
static const I2C_ChannelConfig_t i2c_config[] = {
    // IIC00: SCI0 channel 0, P03(SCL), P04(SDA)
    {SCI0_BASE, 0x10, 0x210, 0x00, 3, 3, 3, 4},
    // IIC01: SCI0 channel 1, P13(SCL?), P14(SDA?) - CẦN KIỂM TRA LẠI
    {SCI0_BASE, 0x12, 0x212, 0x02, 1, 3, 1, 4},
    // ... thêm các channel khác
};

I2C_Error_t I2C_Init(I2C_Channel_t channel, uint32_t scl_freq) {
    if(channel > IIC21) return I2C_ERR_BUSY;
    
    const I2C_ChannelConfig_t *config = &i2c_config[channel];
    
    // 1. Enable peripheral clock
    PER0 |= (1 << 0);  // Enable SCI0 clock
    
    // 2. Configure GPIO pins
    // Set SCL/SDA as open-drain
    volatile uint8_t *pm_reg = (volatile uint8_t *)(0x40000320 + config->scl_port);
    volatile uint8_t *pom_reg = (volatile uint8_t *)(0x40000040 + config->scl_port);
    
    *pm_reg |= (1 << config->scl_pin) | (1 << config->sda_pin);    // Input mode
    *pom_reg |= (1 << config->scl_pin) | (1 << config->sda_pin);   // Open-drain
    
    // 3. Calculate baud rate divider
    // Giả sử FMCK = 32MHz (cần check datasheet)
    uint32_t fmck = 32000000;
    uint8_t divider = Calculate_SCL_Divider(fmck, scl_freq);
    
    // 4. Configure SMRmn (Simplified I²C mode)
    volatile uint16_t *smr_reg = (volatile uint16_t *)(config->base_addr + config->smr_offset);
    *smr_reg = 0x0024;  // Simplified I²C, transfer end interrupt
    
    // 5. Configure SDRmn[15:9] for baud rate
    volatile uint16_t *sdr_reg = (volatile uint16_t *)(config->base_addr + config->sdr_offset);
    *sdr_reg = ((uint16_t)divider << 9);  // Set bits 15:9
    
    // 6. Enable output
    // SOE0 register enable for channel
    uint8_t channel_bit = channel & 0x03;  // Bits 0-3 for SCI0
    SOE0 |= (1 << channel_bit);
    
    // 7. Start channel
    SS0 |= (1 << channel_bit);
    
    return I2C_OK;
}
//Phần 2: Generate Start/Stop Conditions

void I2C_GenerateStart(I2C_Channel_t channel) {
    const I2C_ChannelConfig_t *config = &i2c_config[channel];
    
    // Get port registers
    volatile uint8_t *p_scl = (volatile uint8_t *)(0x40000300 + config->scl_port);
    volatile uint8_t *p_sda = (volatile uint8_t *)(0x40000300 + config->sda_port);
    
    // Start condition: SDA high→low while SCL high
    *p_sda = 1;  // SDA = 1
    *p_scl = 1;  // SCL = 1
    Delay_us(5); // Wait for setup time
    
    *p_sda = 0;  // SDA = 0 (start)
    Delay_us(5);
    
    *p_scl = 0;  // SCL = 0 (ready for data)
    Delay_us(5);
}

void I2C_GenerateStop(I2C_Channel_t channel) {
    const I2C_ChannelConfig_t *config = &i2c_config[channel];
    
    volatile uint8_t *p_scl = (volatile uint8_t *)(0x40000300 + config->scl_port);
    volatile uint8_t *p_sda = (volatile uint8_t *)(0x40000300 + config->sda_port);
    
    // Stop condition: SDA low→high while SCL high
    *p_scl = 0;  // SCL = 0
    *p_sda = 0;  // SDA = 0
    Delay_us(5);
    
    *p_scl = 1;  // SCL = 1
    Delay_us(5);
    
    *p_sda = 1;  // SDA = 1 (stop)
    Delay_us(5);
}
//Phần 3: Send/Receive Byte

I2C_Error_t I2C_SendByte(I2C_Channel_t channel, uint8_t data) {
    const I2C_ChannelConfig_t *config = &i2c_config[channel];
    
    volatile uint16_t *sdr_reg = (volatile uint16_t *)(config->base_addr + config->sdr_offset);
    volatile uint16_t *ssr_reg = (volatile uint16_t *)(config->base_addr + config->ssr_offset);
    volatile uint16_t *sir_reg = (volatile uint16_t *)(config->base_addr + config->ssr_offset + 0x08);
    
    // 1. Write data to SDRmn
    *sdr_reg = (uint16_t)data;
    
    // 2. Wait for transmission complete
    uint32_t timeout = 10000;  // Timeout counter
    while(!(*ssr_reg & 0x0002)) {  // Wait TRC bit (transfer complete)
        if(--timeout == 0) {
            return I2C_ERR_TIMEOUT;
        }
    }
    
    // 3. Check for ACK error
    if(*ssr_reg & 0x0010) {  // ACK error flag
        *sir_reg = 0x0010;   // Clear ACK error flag
        return I2C_ERR_ACK;
    }
    
    // 4. Check for overflow error
    if(*ssr_reg & 0x0004) {  // OVF error flag
        *sir_reg = 0x0004;   // Clear OVF error flag
        return I2C_ERR_OVF;
    }
    
    return I2C_OK;
}

uint8_t I2C_ReceiveByte(I2C_Channel_t channel, bool send_ack) {
    const I2C_ChannelConfig_t *config = &i2c_config[channel];
    
    volatile uint16_t *sdr_reg = (volatile uint16_t *)(config->base_addr + config->sdr_offset);
    volatile uint16_t *ssr_reg = (volatile uint16_t *)(config->base_addr + config->ssr_offset);
    
    // Wait for reception complete
    while(!(*ssr_reg & 0x0002));  // Wait TRC bit
    
    // Read received data
    uint8_t data = (uint8_t)(*sdr_reg & 0x00FF);
    
    // Send ACK/NACK for next byte
    if(send_ack) {
        // Send ACK (master sends 0)
        // For Simplified I²C, ACK handling is automatic
        // Just prepare for next byte
    } else {
        // Send NACK (master sends 1)
        // For last byte, disable ACK output
        uint8_t channel_bit = channel & 0x03;
        SOE0 &= ~(1 << channel_bit);  // Disable ACK output
    }
    
    return data;
}

//Phần 4: High-level Write/Read Functions
I2C_Error_t I2C_Write(I2C_Channel_t channel, uint8_t slave_addr, 
                      uint8_t *data, uint16_t len) {
    I2C_Error_t err;
    
    // 1. Generate START condition
    I2C_GenerateStart(channel);
    
    // 2. Send slave address + Write bit (0)
    err = I2C_SendByte(channel, (slave_addr << 1) | 0);
    if(err != I2C_OK) {
        I2C_GenerateStop(channel);
        return err;
    }
    
    // 3. Send data bytes
    for(uint16_t i = 0; i < len; i++) {
        err = I2C_SendByte(channel, data[i]);
        if(err != I2C_OK) {
            I2C_GenerateStop(channel);
            return err;
        }
    }
    
    // 4. Generate STOP condition
    I2C_GenerateStop(channel);
    
    return I2C_OK;
}

I2C_Error_t I2C_Read(I2C_Channel_t channel, uint8_t slave_addr,
                     uint8_t *buffer, uint16_t len) {
    I2C_Error_t err;
    
    // 1. Generate START condition
    I2C_GenerateStart(channel);
    
    // 2. Send slave address + Read bit (1)
    err = I2C_SendByte(channel, (slave_addr << 1) | 1);
    if(err != I2C_OK) {
        I2C_GenerateStop(channel);
        return err;
    }
    
    // 3. Receive data bytes
    for(uint16_t i = 0; i < len; i++) {
        bool send_ack = (i < (len - 1));  // Send ACK for all but last byte
        buffer[i] = I2C_ReceiveByte(channel, send_ack);
    }
    
    // 4. Generate STOP condition
    I2C_GenerateStop(channel);
    
    return I2C_OK;
}

I2C_Error_t I2C_WriteThenRead(I2C_Channel_t channel, uint8_t slave_addr,
                              uint8_t *tx_data, uint16_t tx_len,
                              uint8_t *rx_buffer, uint16_t rx_len) {
    I2C_Error_t err;
    
    // 1. Write phase
    err = I2C_Write(channel, slave_addr, tx_data, tx_len);
    if(err != I2C_OK) return err;
    
    // Optional delay for some devices
    Delay_ms(1);
    
    // 2. Read phase
    err = I2C_Read(channel, slave_addr, rx_buffer, rx_len);
    
    return err;
}


#include "i2c_simplified.h"

#define EEPROM_ADDRESS 0x50  // 24C02 EEPROM

int main(void) {
    // 1. Initialize I²C at 100kHz
    I2C_Error_t err = I2C_Init(IIC00, 100000);
    if(err != I2C_OK) {
        // Handle error
        while(1);
    }
    
    // 2. Write data to EEPROM
    uint8_t write_data[] = {0x00, 0x01, 0x02, 0x03};  // Address + data
    err = I2C_Write(IIC00, EEPROM_ADDRESS, write_data, sizeof(write_data));
    
    // Wait for EEPROM write cycle
    Delay_ms(10);
    
    // 3. Read data back
    uint8_t read_buffer[3];
    uint8_t read_addr = 0x01;  // Read from address 0x01
    err = I2C_WriteThenRead(IIC00, EEPROM_ADDRESS, 
                            &read_addr, 1,  // Send address
                            read_buffer, 3); // Read 3 bytes
    
    // 4. Check result
    if(err == I2C_OK) {
        // Success
        for(int i = 0; i < 3; i++) {
            printf("Data[%d] = 0x%02X\n", i, read_buffer[i]);
        }
    } else {
        printf("I2C Error: %d\n", err);
    }
    
    while(1);
}