// ============================================
// IICA REGISTER ADDRESSES (Chapter 20)
// ============================================

// IICA Base Addresses
#define IICA0_BASE 0x40042000  // IICA Channel 0
#define IICA1_BASE 0x40042800  // IICA Channel 1

// IICA Control Registers (n = 0,1)
#define IICCTLn0(base) (*(volatile uint16_t *)(base + 0x00))  // Control Register n0
#define IICCTLn1(base) (*(volatile uint16_t *)(base + 0x02))  // Control Register n1
#define IICSn(base)    (*(volatile uint16_t *)(base + 0x04))  // Status Register n
#define IICFn(base)    (*(volatile uint16_t *)(base + 0x06))  // Flag Register n

// IICA Data & Configuration Registers
#define IICWLn(base)   (*(volatile uint8_t *)(base + 0x08))   // Low Width Setting
#define IICWHn(base)   (*(volatile uint8_t *)(base + 0x09))   // High Width Setting
#define IICDn(base)    (*(volatile uint8_t *)(base + 0x0A))   // Data Register n
#define SVAn(base)     (*(volatile uint8_t *)(base + 0x0B))   // Slave Address Register n

// IICA Mode Registers
#define IICMn0(base)   (*(volatile uint8_t *)(base + 0x0C))   // Mode Register n0
#define IICMn1(base)   (*(volatile uint8_t *)(base + 0x0D))   // Mode Register n1
#define IICMn2(base)   (*(volatile uint8_t *)(base + 0x0E))   // Mode Register n2

// Peripheral Enable Register
#define PER1           (*(volatile uint8_t *)0x40020421)

// GPIO Registers for IICA Pins
#define PM4            (*(volatile uint8_t *)0x40000324)  // Port 4 Mode
#define POM4           (*(volatile uint8_t *)0x40000044)  // Port 4 Output Mode
#define P4             (*(volatile uint8_t *)0x40000304)  // Port 4 Data
#define PU4            (*(volatile uint8_t *)0x40000034)  // Port 4 Pull-up

#define PM5            (*(volatile uint8_t *)0x40000325)
#define POM5           (*(volatile uint8_t *)0x40000045)
#define P5             (*(volatile uint8_t *)0x40000305)
#define PU5            (*(volatile uint8_t *)0x40000035)

// ============================================
// IICA PIN MAPPING
// ============================================

// IICA Channel 0: SCLA0(P40), SDAA0(P41)
// IICA Channel 1: SCLA1(P50), SDAA1(P51)
// (Có thể remap qua PIOR registers)

typedef enum {
    IICA_CHANNEL_0 = 0,
    IICA_CHANNEL_1 = 1
} IICA_Channel_t;

typedef enum {
    IICA_MODE_MASTER = 0,
    IICA_MODE_SLAVE = 1
} IICA_Mode_t;

// ============================================
// ERROR DEFINITIONS
// ============================================

typedef enum {
    IICA_OK = 0,
    IICA_ERR_BUS_BUSY,
    IICA_ERR_ARBITRATION_LOST,
    IICA_ERR_NACK,
    IICA_ERR_TIMEOUT,
    IICA_ERR_INVALID_STATE,
    IICA_ERR_START_FAILED,
    IICA_ERR_STOP_FAILED
} IICA_Error_t;

// ============================================
// GPIO CONFIGURATION FOR IICA
// ============================================

void IICA_GPIO_Config(IICA_Channel_t channel) {
    if(channel == IICA_CHANNEL_0) {
        // IICA0: P40(SCLA0), P41(SDAA0)
        
        // 1. Set as Open-drain (bắt buộc cho I²C)
        POM4 |= (1 << 0) | (1 << 1);  // POM40=1, POM41=1 (Open-drain)
        
        // 2. Set as Input mode khi idle
        PM4 |= (1 << 0) | (1 << 1);   // PM40=1, PM41=1 (Input mode)
        
        // 3. Enable internal pull-up (optional, vẫn cần pull-up ngoài)
        PU4 |= (1 << 0) | (1 << 1);   // PU40=1, PU41=1
        
        // 4. Set bus idle state (high)
        P4 |= (1 << 0) | (1 << 1);    // SCL=1, SDA=1
        
    } else if(channel == IICA_CHANNEL_1) {
        // IICA1: P50(SCLA1), P51(SDAA1)
        
        POM5 |= (1 << 0) | (1 << 1);  // Open-drain
        PM5 |= (1 << 0) | (1 << 1);   // Input mode
        PU5 |= (1 << 0) | (1 << 1);   // Pull-up
        P5 |= (1 << 0) | (1 << 1);    // Bus idle
    }
    
    // Wait for bus stabilization
    Delay_us(10);
}

// ============================================
// IICA PERIPHERAL ENABLE
// ============================================

void IICA_Peripheral_Enable(IICA_Channel_t channel) {
    // Enable clock for IICA peripheral
    // Bit 4: IICA0 enable, Bit 5: IICA1 enable trong PER1
    
    if(channel == IICA_CHANNEL_0) {
        PER1 |= (1 << 4);  // Enable IICA0 clock
    } else {
        PER1 |= (1 << 5);  // Enable IICA1 clock
    }
    
    // Wait for peripheral to be ready
    Delay_us(100);
}

// ============================================
// GPIO CONFIGURATION FOR IICA
// ============================================

void IICA_GPIO_Config(IICA_Channel_t channel) {
    if(channel == IICA_CHANNEL_0) {
        // IICA0: P40(SCLA0), P41(SDAA0)
        
        // 1. Set as Open-drain (bắt buộc cho I²C)
        POM4 |= (1 << 0) | (1 << 1);  // POM40=1, POM41=1 (Open-drain)
        
        // 2. Set as Input mode khi idle
        PM4 |= (1 << 0) | (1 << 1);   // PM40=1, PM41=1 (Input mode)
        
        // 3. Enable internal pull-up (optional, vẫn cần pull-up ngoài)
        PU4 |= (1 << 0) | (1 << 1);   // PU40=1, PU41=1
        
        // 4. Set bus idle state (high)
        P4 |= (1 << 0) | (1 << 1);    // SCL=1, SDA=1
        
    } else if(channel == IICA_CHANNEL_1) {
        // IICA1: P50(SCLA1), P51(SDAA1)
        
        POM5 |= (1 << 0) | (1 << 1);  // Open-drain
        PM5 |= (1 << 0) | (1 << 1);   // Input mode
        PU5 |= (1 << 0) | (1 << 1);   // Pull-up
        P5 |= (1 << 0) | (1 << 1);    // Bus idle
    }
    
    // Wait for bus stabilization
    Delay_us(10);
}

// ============================================
// IICA PERIPHERAL ENABLE
// ============================================

void IICA_Peripheral_Enable(IICA_Channel_t channel) {
    // Enable clock for IICA peripheral
    // Bit 4: IICA0 enable, Bit 5: IICA1 enable trong PER1
    
    if(channel == IICA_CHANNEL_0) {
        PER1 |= (1 << 4);  // Enable IICA0 clock
    } else {
        PER1 |= (1 << 5);  // Enable IICA1 clock
    }
    
    // Wait for peripheral to be ready
    Delay_us(100);
}

// ============================================
// MASTER MODE INITIALIZATION
// ============================================

IICA_Error_t IICA_Master_Init(IICA_Channel_t channel, uint32_t scl_freq_hz) {
    uint32_t base_addr = (channel == IICA_CHANNEL_0) ? IICA0_BASE : IICA1_BASE;
    
    // 1. Enable Peripheral Clock
    IICA_Peripheral_Enable(channel);
    
    // 2. Configure GPIO Pins
    IICA_GPIO_Config(channel);
    
    // 3. Disable IICA (IICE=0) trước khi config
    IICCTLn0(base_addr) &= ~(1 << 7);  // Clear IICE bit
    
    // 4. Set Master Mode
    IICCTLn0(base_addr) &= ~(1 << 0);   // MST = 1 (Master mode)
    IICCTLn0(base_addr) |= (1 << 0);    // Set Master mode
    
    // 5. Configure Clock Frequency
    // Tính toán giá trị cho IICWLn và IICWHn
    // Tần số SCL = fCLK / (IICWH + IICWL + 6)
    
    uint32_t fclk = 32000000;  // System clock (cần check datasheet)
    uint32_t total_cycles = fclk / scl_freq_hz;
    
    if(total_cycles < 6) {
        return IICA_ERR_INVALID_STATE;  // Frequency too high
    }
    
    uint32_t wh_wl = total_cycles - 6;
    uint8_t iicwh = wh_wl / 2;
    uint8_t iicwl = wh_wl - iicwh;
    
    // Limit values (register is 8-bit)
    if(iicwh > 255) iicwh = 255;
    if(iicwl > 255) iicwl = 255;
    
    IICWHn(base_addr) = iicwh;
    IICWLn(base_addr) = iicwl;
    
    // 6. Configure Interrupts (optional)
    // Enable transfer complete interrupt
    IICCTLn0(base_addr) |= (1 << 3);  // WTIM = 0 (8-bit data)
    IICCTLn0(base_addr) |= (1 << 6);  // SPIE = 1 (Enable stop condition interrupt)
    
    // 7. Clear all flags
    IICFn(base_addr) = 0xFF;
    
    // 8. Enable IICA
    IICCTLn0(base_addr) |= (1 << 7);  // IICE = 1
    
    // 9. Wait for bus free
    uint32_t timeout = 10000;
    while(IICSn(base_addr) & (1 << 2)) {  // Check BB bit (Bus Busy)
        if(--timeout == 0) {
            return IICA_ERR_BUS_BUSY;
        }
        Delay_us(1);
    }
    
    return IICA_OK;
}

// ============================================
// SLAVE MODE INITIALIZATION
// ============================================

IICA_Error_t IICA_Slave_Init(IICA_Channel_t channel, uint8_t slave_address) {
    uint32_t base_addr = (channel == IICA_CHANNEL_0) ? IICA0_BASE : IICA1_BASE;
    
    // 1. Enable Peripheral Clock
    IICA_Peripheral_Enable(channel);
    
    // 2. Configure GPIO Pins
    IICA_GPIO_Config(channel);
    
    // 3. Disable IICA (IICE=0) trước khi config
    IICCTLn0(base_addr) &= ~(1 << 7);
    
    // 4. Set Slave Mode
    IICCTLn0(base_addr) &= ~(1 << 0);  // MST = 0 (Slave mode)
    
    // 5. Set Slave Address (7-bit)
    if(slave_address > 0x7F) {
        return IICA_ERR_INVALID_STATE;  // 7-bit address only
    }
    SVAn(base_addr) = slave_address << 1;  // Shift left 1 bit
    
    // 6. Configure Interrupts
    // Enable address match interrupt
    IICCTLn0(base_addr) |= (1 << 3);  // WTIM = 0
    IICCTLn0(base_addr) |= (1 << 5);  // ACKE = 1 (Enable ACK)
    IICCTLn0(base_addr) |= (1 << 4);  // WREL = 1 (Enable wait release)
    
    // 7. Configure Mode Registers
    // Enable extension code (nếu cần 10-bit addressing)
    IICMn0(base_addr) = 0x00;  // Thường để 0 cho 7-bit
    
    // 8. Clear all flags
    IICFn(base_addr) = 0xFF;
    
    // 9. Enable IICA
    IICCTLn0(base_addr) |= (1 << 7);  // IICE = 1
    
    // 10. Enable Wake-up function (nếu cần)
    IICMn1(base_addr) |= (1 << 0);  // WUP = 1
    
    return IICA_OK;
}

// ============================================
// MASTER START CONDITION
// ============================================

IICA_Error_t IICA_Master_Start(uint32_t base_addr, uint8_t slave_addr, uint8_t rw_bit) {
    // 1. Check if bus is busy
    if(IICSn(base_addr) & (1 << 2)) {  // BB bit = 1
        return IICA_ERR_BUS_BUSY;
    }
    
    // 2. Clear all flags
    IICFn(base_addr) = 0xFF;
    
    // 3. Set slave address + R/W bit
    uint8_t addr_byte = (slave_addr << 1) | (rw_bit & 0x01);
    IICDn(base_addr) = addr_byte;
    
    // 4. Generate Start Condition
    // Set STT = 1 (Start condition) và SPT = 0
    IICCTLn0(base_addr) |= (1 << 1);   // STT = 1
    IICCTLn0(base_addr) &= ~(1 << 2);  // SPT = 0
    
    // 5. Wait for transmission to start
    uint32_t timeout = 10000;
    while(!(IICFn(base_addr) & (1 << 1))) {  // Wait TDRE flag
        if(--timeout == 0) {
            return IICA_ERR_START_FAILED;
        }
        Delay_us(1);
    }
    
    // 6. Check for NACK
    if(IICFn(base_addr) & (1 << 6)) {  // NACKF flag
        IICFn(base_addr) = (1 << 6);   // Clear NACKF
        return IICA_ERR_NACK;
    }
    
    return IICA_OK;
}

// ============================================
// MASTER SEND DATA
// ============================================

IICA_Error_t IICA_Master_Send_Byte(uint32_t base_addr, uint8_t data) {
    // 1. Wait for transmit buffer empty
    uint32_t timeout = 10000;
    while(!(IICFn(base_addr) & (1 << 1))) {  // Wait TDRE
        if(--timeout == 0) {
            return IICA_ERR_TIMEOUT;
        }
        Delay_us(1);
    }
    
    // 2. Write data to IICD register
    IICDn(base_addr) = data;
    
    // 3. Clear TDRE flag (write 1 to clear)
    IICFn(base_addr) = (1 << 1);
    
    // 4. Wait for transmission complete
    timeout = 10000;
    while(!(IICFn(base_addr) & (1 << 0))) {  // Wait TRC
        // Check for errors
        if(IICFn(base_addr) & (1 << 6)) {  // NACK
            IICFn(base_addr) = (1 << 6);   // Clear NACKF
            return IICA_ERR_NACK;
        }
        
        if(IICFn(base_addr) & (1 << 3)) {  // Arbitration lost
            IICFn(base_addr) = (1 << 3);   // Clear AL
            return IICA_ERR_ARBITRATION_LOST;
        }
        
        if(--timeout == 0) {
            return IICA_ERR_TIMEOUT;
        }
        Delay_us(1);
    }
    
    // 5. Clear TRC flag
    IICFn(base_addr) = (1 << 0);
    
    return IICA_OK;
}

// Send multiple bytes
IICA_Error_t IICA_Master_Send_Bytes(uint32_t base_addr, uint8_t *data, uint16_t len) {
    IICA_Error_t err;
    
    for(uint16_t i = 0; i < len; i++) {
        err = IICA_Master_Send_Byte(base_addr, data[i]);
        if(err != IICA_OK) {
            return err;
        }
    }
    
    return IICA_OK;
}

// ============================================
// MASTER RECEIVE DATA
// ============================================

IICA_Error_t IICA_Master_Receive_Byte(uint32_t base_addr, uint8_t *data, bool send_ack) {
    // 1. Configure ACK/NACK cho byte này
    if(send_ack) {
        IICCTLn0(base_addr) |= (1 << 5);   // ACKE = 1 (Send ACK)
    } else {
        IICCTLn0(base_addr) &= ~(1 << 5);  // ACKE = 0 (Send NACK)
        
        // Nếu là byte cuối, generate Stop condition sau khi nhận
        IICCTLn0(base_addr) |= (1 << 2);   // SPT = 1 (Auto stop)
    }
    
    // 2. Wait for receive buffer full
    uint32_t timeout = 10000;
    while(!(IICFn(base_addr) & (1 << 2))) {  // Wait RDRF
        if(--timeout == 0) {
            return IICA_ERR_TIMEOUT;
        }
        Delay_us(1);
    }
    
    // 3. Read data
    *data = IICDn(base_addr);
    
    // 4. Clear RDRF flag
    IICFn(base_addr) = (1 << 2);
    
    return IICA_OK;
}

// Receive multiple bytes
IICA_Error_t IICA_Master_Receive_Bytes(uint32_t base_addr, uint8_t *buffer, uint16_t len) {
    IICA_Error_t err;
    
    for(uint16_t i = 0; i < len; i++) {
        bool send_ack = (i < (len - 1));  // ACK cho tất cả trừ byte cuối
        
        err = IICA_Master_Receive_Byte(base_addr, &buffer[i], send_ack);
        if(err != IICA_OK) {
            return err;
        }
    }
    
    return IICA_OK;
}

// ============================================
// MASTER STOP CONDITION
// ============================================

IICA_Error_t IICA_Master_Stop(uint32_t base_addr) {
    // 1. Check if bus is busy
    if(!(IICSn(base_addr) & (1 << 2))) {  // BB = 0 (bus not busy)
        return IICA_OK;  // Already stopped
    }
    
    // 2. Generate Stop Condition
    IICCTLn0(base_addr) |= (1 << 2);  // SPT = 1
    
    // 3. Wait for stop condition to complete
    uint32_t timeout = 10000;
    while(IICSn(base_addr) & (1 << 2)) {  // Wait BB = 0
        if(--timeout == 0) {
            return IICA_ERR_STOP_FAILED;
        }
        Delay_us(1);
    }
    
    // 4. Clear stop flag
    IICCTLn0(base_addr) &= ~(1 << 2);  // SPT = 0
    
    return IICA_OK;
}

// ============================================
// COMPLETE MASTER FUNCTIONS
// ============================================

IICA_Error_t IICA_Master_Write(IICA_Channel_t channel, uint8_t slave_addr,
                               uint8_t *data, uint16_t len, bool send_stop) {
    uint32_t base_addr = (channel == IICA_CHANNEL_0) ? IICA0_BASE : IICA1_BASE;
    IICA_Error_t err;
    
    // 1. Start Condition + Address (Write)
    err = IICA_Master_Start(base_addr, slave_addr, 0);  // R/W = 0 (Write)
    if(err != IICA_OK) {
        return err;
    }
    
    // 2. Send Data
    err = IICA_Master_Send_Bytes(base_addr, data, len);
    if(err != IICA_OK) {
        IICA_Master_Stop(base_addr);  // Cleanup
        return err;
    }
    
    // 3. Stop Condition (nếu cần)
    if(send_stop) {
        err = IICA_Master_Stop(base_addr);
    }
    
    return err;
}

IICA_Error_t IICA_Master_Read(IICA_Channel_t channel, uint8_t slave_addr,
                              uint8_t *buffer, uint16_t len) {
    uint32_t base_addr = (channel == IICA_CHANNEL_0) ? IICA0_BASE : IICA1_BASE;
    IICA_Error_t err;
    
    // 1. Start Condition + Address (Read)
    err = IICA_Master_Start(base_addr, slave_addr, 1);  // R/W = 1 (Read)
    if(err != IICA_OK) {
        return err;
    }
    
    // 2. Receive Data
    err = IICA_Master_Receive_Bytes(base_addr, buffer, len);
    
    // 3. Stop Condition
    IICA_Master_Stop(base_addr);
    
    return err;
}

IICA_Error_t IICA_Master_Write_Then_Read(IICA_Channel_t channel, uint8_t slave_addr,
                                         uint8_t *tx_data, uint16_t tx_len,
                                         uint8_t *rx_buffer, uint16_t rx_len) {
    uint32_t base_addr = (channel == IICA_CHANNEL_0) ? IICA0_BASE : IICA1_BASE;
    IICA_Error_t err;
    
    // 1. Write Phase (gửi địa chỉ/command)
    err = IICA_Master_Write(channel, slave_addr, tx_data, tx_len, false);  // No stop
    if(err != IICA_OK) {
        IICA_Master_Stop(base_addr);  // Cleanup
        return err;
    }
    
    // 2. Repeated Start
    // IICA tự động handle repeated start khi không gửi stop
    
    // 3. Read Phase
    err = IICA_Master_Read(channel, slave_addr, rx_buffer, rx_len);
    
    return err;
}

// ============================================
// SLAVE MODE - INTERRUPT HANDLER
// ============================================

typedef struct {
    uint8_t rx_buffer[256];
    uint8_t tx_buffer[256];
    uint16_t rx_index;
    uint16_t tx_index;
    uint16_t tx_length;
    bool address_matched;
    bool read_requested;
    bool write_requested;
} IICA_Slave_Context_t;

static IICA_Slave_Context_t slave_ctx[2];  // Cho IICA0 và IICA1

// Slave Interrupt Handler
void __attribute__((interrupt)) IICA0_IRQHandler(void) {
    uint32_t base_addr = IICA0_BASE;
    uint16_t status = IICSn(base_addr);
    uint16_t flags = IICFn(base_addr);
    
    // 1. Start Condition Detected
    if(flags & (1 << 4)) {  // STARTF flag
        slave_ctx[0].address_matched = false;
        slave_ctx[0].read_requested = false;
        slave_ctx[0].write_requested = false;
        
        IICFn(base_addr) = (1 << 4);  // Clear STARTF
    }
    
    // 2. Address Match
    if(flags & (1 << 5)) {  // ADR0F flag (address 0 match)
        slave_ctx[0].address_matched = true;
        
        // Check R/W bit
        uint8_t received_addr = IICDn(base_addr);
        if(received_addr & 0x01) {
            // Master wants to READ from us
            slave_ctx[0].read_requested = true;
            slave_ctx[0].tx_index = 0;
            
            // Prepare first byte to send
            if(slave_ctx[0].tx_index < slave_ctx[0].tx_length) {
                IICDn(base_addr) = slave_ctx[0].tx_buffer[slave_ctx[0].tx_index++];
            }
            
        } else {
            // Master wants to WRITE to us
            slave_ctx[0].write_requested = true;
            slave_ctx[0].rx_index = 0;
        }
        
        IICFn(base_addr) = (1 << 5);  // Clear ADR0F
    }
    
    // 3. Receive Data Ready (Master writing to us)
    if(flags & (1 << 2)) {  // RDRF flag
        if(slave_ctx[0].write_requested && slave_ctx[0].rx_index < 256) {
            slave_ctx[0].rx_buffer[slave_ctx[0].rx_index++] = IICDn(base_addr);
        }
        IICFn(base_addr) = (1 << 2);  // Clear RDRF
    }
    
    // 4. Transmit Data Ready (Master reading from us)
    if(flags & (1 << 1)) {  // TDRE flag
        if(slave_ctx[0].read_requested && slave_ctx[0].tx_index < slave_ctx[0].tx_length) {
            // Send next byte
            IICDn(base_addr) = slave_ctx[0].tx_buffer[slave_ctx[0].tx_index++];
        } else {
            // No more data to send, send 0xFF
            IICDn(base_addr) = 0xFF;
        }
        IICFn(base_addr) = (1 << 1);  // Clear TDRE
    }
    
    // 5. Stop Condition Detected
    if(flags & (1 << 3)) {  // STOPF flag
        // Transfer complete
        if(slave_ctx[0].write_requested) {
            // Process received data
            Process_Received_Data(slave_ctx[0].rx_buffer, slave_ctx[0].rx_index);
        }
        
        // Reset context
        slave_ctx[0].address_matched = false;
        slave_ctx[0].read_requested = false;
        slave_ctx[0].write_requested = false;
        slave_ctx[0].rx_index = 0;
        slave_ctx[0].tx_index = 0;
        
        IICFn(base_addr) = (1 << 3);  // Clear STOPF
    }
    
    // 6. NACK Received (Master doesn't want more data)
    if(flags & (1 << 6)) {  // NACKF flag
        slave_ctx[0].read_requested = false;
        IICFn(base_addr) = (1 << 6);  // Clear NACKF
    }
}

// ============================================
// SLAVE DATA PREPARATION
// ============================================

void IICA_Slave_Prepare_Data(IICA_Channel_t channel, uint8_t *data, uint16_t len) {
    if(channel == IICA_CHANNEL_0) {
        if(len > 256) len = 256;
        
        // Copy data to transmit buffer
        for(uint16_t i = 0; i < len; i++) {
            slave_ctx[0].tx_buffer[i] = data[i];
        }
        slave_ctx[0].tx_length = len;
        slave_ctx[0].tx_index = 0;
        
    } else {
        // For IICA1
        if(len > 256) len = 256;
        
        for(uint16_t i = 0; i < len; i++) {
            slave_ctx[1].tx_buffer[i] = data[i];
        }
        slave_ctx[1].tx_length = len;
        slave_ctx[1].tx_index = 0;
    }
}

uint16_t IICA_Slave_Get_Received_Data(IICA_Channel_t channel, uint8_t *buffer) {
    uint16_t count = 0;
    
    if(channel == IICA_CHANNEL_0) {
        count = slave_ctx[0].rx_index;
        for(uint16_t i = 0; i < count; i++) {
            buffer[i] = slave_ctx[0].rx_buffer[i];
        }
        slave_ctx[0].rx_index = 0;  // Clear buffer
    } else {
        count = slave_ctx[1].rx_index;
        for(uint16_t i = 0; i < count; i++) {
            buffer[i] = slave_ctx[1].rx_buffer[i];
        }
        slave_ctx[1].rx_index = 0;
    }
    
    return count;
}

// ============================================
// SLAVE WITH MULTIPLE ADDRESSES
// ============================================

IICA_Error_t IICA_Slave_Multi_Address_Init(IICA_Channel_t channel, 
                                           uint8_t *addresses, uint8_t num_addresses) {
    uint32_t base_addr = (channel == IICA_CHANNEL_0) ? IICA0_BASE : IICA1_BASE;
    
    // 1. Basic Slave Init
    IICA_Error_t err = IICA_Slave_Init(channel, addresses[0]);
    if(err != IICA_OK) {
        return err;
    }
    
    // 2. Configure Additional Address Registers
    // IICA có thể nhận đến 4 slave addresses:
    // - ADR0 (chính): SVAn register
    // - ADR1, ADR2, ADR3: trong IICMn2 register
    
    if(num_addresses > 1) {
        // Enable additional address recognition
        IICMn1(base_addr) |= (1 << 3);  // ADRMD = 1 (enable multi addresses)
        
        // Set additional addresses
        // Format: bit 7:1 = address, bit 0 = enable
        for(uint8_t i = 1; i < num_addresses && i <= 3; i++) {
            uint8_t reg_value = (addresses[i] << 1) | 0x01;
            
            if(i == 1) {
                IICMn2(base_addr) = (IICMn2(base_addr) & 0x0F) | (reg_value << 4);
            } else if(i == 2) {
                IICMn2(base_addr) = (IICMn2(base_addr) & 0xF0) | (reg_value >> 4);
                // Need another register for full implementation
            }
        }
    }
    
    // 3. Enable all interrupts
    IICCTLn0(base_addr) |= (1 << 5);  // ACKE = 1
    IICCTLn0(base_addr) |= (1 << 4);  // WREL = 1
    IICCTLn0(base_addr) |= (1 << 3);  // WTIM = 0
    
    return IICA_OK;
}

// ============================================
// 10-BIT ADDRESSING
// ============================================

IICA_Error_t IICA_Master_10Bit_Addressing(IICA_Channel_t channel, uint16_t slave_addr,
                                          uint8_t *data, uint16_t len, bool is_read) {
    uint32_t base_addr = (channel == IICA_CHANNEL_0) ? IICA0_BASE : IICA1_BASE;
    
    if(slave_addr > 0x3FF) {
        return IICA_ERR_INVALID_STATE;  // 10-bit max 0x3FF
    }
    
    // 1. Enable 10-bit addressing mode
    IICMn0(base_addr) |= (1 << 0);  // EXCEN = 1 (enable extension code)
    
    // 2. Start with header byte
    // Header: 11110 + A9 + A8 + R/W
    uint8_t header = 0xF0 | ((slave_addr >> 8) & 0x03);
    if(is_read) header |= 0x01;
    
    // 3. Send header
    IICA_Error_t err = IICA_Master_Start(base_addr, header >> 1, header & 0x01);
    if(err != IICA_OK) {
        return err;
    }
    
    // 4. Send lower 8 bits of address
    uint8_t addr_low = slave_addr & 0xFF;
    err = IICA_Master_Send_Byte(base_addr, addr_low);
    if(err != IICA_OK) {
        IICA_Master_Stop(base_addr);
        return err;
    }
    
    // 5. If writing, send data
    if(!is_read) {
        err = IICA_Master_Send_Bytes(base_addr, data, len);
    } else {
        // If reading, need repeated start
        IICA_Master_Stop(base_addr);  // Send stop
        Delay_us(10);
        
        // Restart with read bit
        header |= 0x01;  // Set read bit
        err = IICA_Master_Start(base_addr, header >> 1, header & 0x01);
        if(err == IICA_OK) {
            err = IICA_Master_Receive_Bytes(base_addr, data, len);
        }
    }
    
    // 6. Stop
    IICA_Master_Stop(base_addr);
    
    return err;
}