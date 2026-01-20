/**
 * Final correct SCRmn configuration for Simplified I2C
 */
#define SCR_I2C_CONFIG_FINAL  ( \
    (1 << 15) | /* TXEmn = 1 */ \
    (1 << 14) | /* RXEmn = 1 */ \
    (0 << 13) | /* DAPmn = 0 */ \
    (0 << 12) | /* CKPmn = 0 */ \
    (0 << 11) | /* Bit11 = 0 */ \
    (0 << 10) | /* EOCmn = 0 */ \
    (0 << 9)  | /* PTCmn1 = 0 */ \
    (0 << 8)  | /* PTCmn0 = 0 */ \
    (0 << 7)  | /* DIRmn = 0 */ \
    (0 << 6)  | /* Bit6 = 0 */ \
    (0 << 5)  | /* SLCmn1 = 0 */ \
    (1 << 4)  | /* SLCmn0 = 1 (1 stop bit) */ \
    (0 << 3)  | /* DLSmn3 = 0 */ \
    (1 << 2)  | /* DLSmn2 = 1 AND Fixed bit2 = 1 */ \
    (1 << 1)  | /* DLSmn1 = 1 */ \
    (1 << 0)    /* DLSmn0 = 1 */ \
)  /* Result: 0xC017 = 49175 */

/* Hoặc tính toán: */
#define SCR_I2C_VALUE  0xC017  /* 49175 decimal */

/* Giải thích hex: */
/* 0xC017 = 1100 0000 0001 0111 binary */
/*          │││  ││││ ││││ └┴┴┴─ DLSmn3-0 = 0111 (8-bit) */
/*          │││  ││││ │││└────── SLCmn0 = 1 (1 stop bit) */
/*          │││  ││││ ││└─────── SLCmn1 = 0 */
/*          │││  ││││ │└──────── Bit6 = 0 */
/*          │││  ││││ └───────── DIRmn = 0 */
/*          │││  │││└─────────── PTCmn0 = 0 */
/*          │││  ││└──────────── PTCmn1 = 0 */
/*          │││  │└───────────── EOCmn = 0 */
/*          │││  └────────────── Bit11 = 0 */
/*          ││└───────────────── CKPmn = 0 */
/*          │└────────────────── DAPmn = 0 */
/*          └─────────────────── RXEmn = 1, TXEmn = 1 */



// Clear all error flags:
SIRmn = 0x0007;  // = 0000 0000 0000 0111
                 //         │││ └└└─ OVCT=1, PECT=1, FECT=1
                 // Write 1 để clear flag tương ứng

// Clear chỉ ACK error:
SIRmn = 0x0002;  // = 0000 0000 0000 0010
                 // Chỉ set PECT=1 → clear PEF flag trong SSRmn

// Clear overflow error:
SIRmn = 0x0001;  // = 0000 0000 0000 0001
                 // Chỉ set OVCT=1 → clear OVF flag

SMRmn = 0x4000;  // = 0100 0000 0000 0000 binary
                /* 
                Bit 15 (MD2) = 1 ─────────────┐
                Bit 14 (MD1) = 0 ────────┐    │ → Mode = 10 (Simplified I2C)
                Bit 13-10 = 0 ───────┐   │    │
                Bit 9 (SIS) = 0 ─┐   │   │    │
                Bit 8 (STS) = 0 ─┤   │   │    │
                Bit 7-6 = 0 ────┼┐  │   │    │
                Bit 5 = 1 ──────┼│┐ │   │    │
                Bit 4-3 = 0 ────┼││┐│   │    │
                Bit 2 (MD0) = 0 ┼││││┐  │    │ → Interrupt: Transfer end
                Bit 1 (CCS) = 0 ┼│││││┐ │    │ → Clock: Divided FMCK
                Bit 0 (CKS) = 0 ┼││││││┐│    │ → Clock source: CKm0
                └───────────────┘││││││││    │
                                 ││││││││    │
                Reset value: 0x0020 = 0000 0000 0010 0000
                             (bit 5 luôn = 1 theo reset)
                */
SCRmn = 0xC017;  // = 1100 0000 0001 0111 binary
                /*
                Bit 15 (TXE) = 1 ───────────────┐ → Transmit enabled
                Bit 14 (RXE) = 1 ────────────┐  │ → Receive enabled  
                Bit 13 (DAP) = 0 ────────┐   │  │ → Not used in I2C
                Bit 12 (CKP) = 0 ──────┐ │   │  │ → Not used in I2C
                Bit 11 = 0 ──────────┐ │ │   │  │ → Fixed 0
                Bit 10 (EOC) = 0 ──┐ │ │ │   │  │ → Error interrupt enabled
                Bit 9 = 0 ───────┐ │ │ │ │   │  │ → Fixed 0
                Bit 8 (PTC1) = 0 ┼┐│ │ │ │   │  │ → No parity
                Bit 7 (PTC0) = 0 ┼││ │ │ │   │  │ → No parity
                Bit 6 (DIR) = 0 ─┼│││ │ │ │   │  │ → MSB-first (required)
                Bit 5 = 0 ──────┼││││ │ │ │   │  │ → Fixed 0
                Bit 4 (SLC1) = 0┼│││││ │ │ │   │  │ → Stop bits = 01
                Bit 3 (SLC0) = 1┼││││││ │ │ │   │  │ → (1 stop bit)
                Bit 2 = 1 ──────┼│││││││ │ │ │   │  │ → Fixed 1
                Bit 1 (DLS3) = 0┼││││││││ │ │ │   │  │ → Data length = 0111
                Bit 0 (DLS2) = 1┼│││││││││ │ │ │   │  │ → (8-bit data)
                DLS1 = 1 ───────┘││││││││││││││   │  │ → (trong thực tế)
                DLS0 = 1 ────────┘│││││││││││││   │  │ → bits này ở vị trí khác
                */