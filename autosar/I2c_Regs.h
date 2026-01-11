#ifndef I2C_REGS_H
#define I2C_REGS_H

/* IICA Register Addresses */
#define IICA0_BASE       0x40041800U
#define IICA1_BASE       0x40046000U

/* Register Offsets */
#define IICCTL0_OFFSET   0x00U
#define IICCTL1_OFFSET   0x01U
#define IICS_OFFSET      0x02U
#define IICF_OFFSET      0x03U
#define IICWL_OFFSET     0x04U
#define IICWH_OFFSET     0x05U
#define SVA0_OFFSET      0x06U
#define SVA1_OFFSET      0x07U
#define IICA_OFFSET      0x08U

/* Clock Control */
#define PER0_ADDR        0x40020410U
#define PER0_IICA0_EN    (1 << 7)
#define PER0_IICA1_EN    (1 << 6)

/* Pin Control */
#define PIOR0_ADDR       0x40040877U
#define PIOR0_SCLA0_SEL  (1 << 2)
#define PIOR0_SDAA0_SEL  (1 << 3)

#endif /* I2C_REGS_H */