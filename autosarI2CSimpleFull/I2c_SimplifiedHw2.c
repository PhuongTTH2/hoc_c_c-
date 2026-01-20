// Định nghĩa địa chỉ thanh ghi GPIO
#define PM0   (*(volatile uint8_t *)0x40000320)  // Port 0 Mode
#define PM1   (*(volatile uint8_t *)0x40000321)  // Port 1 Mode
#define PM2   (*(volatile uint8_t *)0x40000322)
#define PM3   (*(volatile uint8_t *)0x40000323)
#define PM4   (*(volatile uint8_t *)0x40000324)
#define PM5   (*(volatile uint8_t *)0x40000325)

#define POM0  (*(volatile uint8_t *)0x40000040)  // Port 0 Output Mode
#define POM1  (*(volatile uint8_t *)0x40000041)  // Port 1 Output Mode
#define POM2  (*(volatile uint8_t *)0x40000042)
#define POM3  (*(volatile uint8_t *)0x40000043)
#define POM4  (*(volatile uint8_t *)0x40000044)
#define POM5  (*(volatile uint8_t *)0x40000045)

#define P0    (*(volatile uint8_t *)0x40000300)  // Port 0 Data
#define P1    (*(volatile uint8_t *)0x40000301)
#define P2    (*(volatile uint8_t *)0x40000302)
#define P3    (*(volatile uint8_t *)0x40000303)
#define P4    (*(volatile uint8_t *)0x40000304)
#define P5    (*(volatile uint8_t *)0x40000305)

#define PU0   (*(volatile uint8_t *)0x40000030)  // Port 0 Pull-up
#define PU1   (*(volatile uint8_t *)0x40000031)
#define PU3   (*(volatile uint8_t *)0x40000033)
#define PU4   (*(volatile uint8_t *)0x40000034)
#define PU5   (*(volatile uint8_t *)0x40000035)

// Cấu trúc lưu thông tin chân I²C
typedef struct {
    uint8_t scl_port;  // Port number (0-5)
    uint8_t scl_pin;   // Pin bit (0-7)
    uint8_t sda_port;
    uint8_t sda_pin;
} I2C_PinConfig_t;

// Pin mapping mặc định (cần verify với datasheet)
const I2C_PinConfig_t I2C_PinMap[] = {
    // IIC00
    {3, 3, 3, 4},  // SCL=P03, SDA=P04
    // IIC01  
    {1, 4, 1, 3},  // SCL=P14, SDA=P13
    // IIC02
    {1, 7, 1, 6},  // SCL=P17, SDA=P16
    // IIC03
    {5, 7, 5, 6},  // SCL=P57, SDA=P56
    // IIC10
    {1, 1, 1, 0},  // SCL=P11, SDA=P10
    // IIC11
    {1, 5, 1, 4},  // SCL=P15, SDA=P14
    // IIC20
    {5, 5, 5, 4},  // SCL=P55, SDA=P54
    // IIC21
    {5, 7, 5, 6},  // SCL=P57, SDA=P56
};

// Hàm config GPIO cho I²C
void GPIO_Config_For_I2C(uint8_t channel) {
    if(channel >= 8) return;
    
    I2C_PinConfig_t pins = I2C_PinMap[channel];
    
    // 1. CONFIG CHO SCL (Serial Clock)
    volatile uint8_t *pm_scl = (volatile uint8_t *)(0x40000320 + pins.scl_port);
    volatile uint8_t *pom_scl = (volatile uint8_t *)(0x40000040 + pins.scl_port);
    volatile uint8_t *p_scl = (volatile uint8_t *)(0x40000300 + pins.scl_port);
    volatile uint8_t *pu_scl = (volatile uint8_t *)(0x40000030 + pins.scl_port);
    
    // 2. CONFIG CHO SDA (Serial Data)
    volatile uint8_t *pm_sda = (volatile uint8_t *)(0x40000320 + pins.sda_port);
    volatile uint8_t *pom_sda = (volatile uint8_t *)(0x40000040 + pins.sda_port);
    volatile uint8_t *p_sda = (volatile uint8_t *)(0x40000300 + pins.sda_port);
    volatile uint8_t *pu_sda = (volatile uint8_t *)(0x40000030 + pins.sda_port);
    
    // 🔥 CẤU HÌNH CHI TIẾT TỪNG CHÂN:
    
    // a) SCL Configuration
    *pm_scl |= (1 << pins.scl_pin);      // PMx.y = 1 (Input mode)
    *pom_scl |= (1 << pins.scl_pin);     // POMx.y = 1 (Open-drain output)
    *p_scl |= (1 << pins.scl_pin);       // Px.y = 1 (Initial high)
    if(pu_scl) *pu_scl |= (1 << pins.scl_pin); // PUx.y = 1 (Enable pull-up)
    
    // b) SDA Configuration  
    *pm_sda |= (1 << pins.sda_pin);      // PMx.y = 1 (Input mode)
    *pom_sda |= (1 << pins.sda_pin);     // POMx.y = 1 (Open-drain output)
    *p_sda |= (1 << pins.sda_pin);       // Px.y = 1 (Initial high)
    if(pu_sda) *pu_sda |= (1 << pins.sda_pin); // PUx.y = 1 (Enable pull-up)
    
    // c) Đảm bảo bus ở trạng thái idle
    Delay_us(10);  // Wait for stabilization
}


// Định nghĩa thanh ghi Serial Communication
#define PER0   (*(volatile uint8_t *)0x40020420)   // Peripheral Enable 0
#define PER2   (*(volatile uint8_t *)0x40020422)   // Peripheral Enable 2

// SCI0 Registers (cho IIC00-03)
#define SCI0_BASE 0x40041100

// Channel offsets trong SCI0
#define SMR00  (*(volatile uint16_t *)(SCI0_BASE + 0x10))
#define SMR01  (*(volatile uint16_t *)(SCI0_BASE + 0x12))
#define SMR02  (*(volatile uint16_t *)(SCI0_BASE + 0x14))
#define SMR03  (*(volatile uint16_t *)(SCI0_BASE + 0x16))

#define SCR00  (*(volatile uint16_t *)(SCI0_BASE + 0x18))
#define SCR01  (*(volatile uint16_t *)(SCI0_BASE + 0x1A))
#define SCR02  (*(volatile uint16_t *)(SCI0_BASE + 0x1C))
#define SCR03  (*(volatile uint16_t *)(SCI0_BASE + 0x1E))

#define SSR00  (*(volatile uint16_t *)(SCI0_BASE + 0x00))
#define SSR01  (*(volatile uint16_t *)(SCI0_BASE + 0x02))
#define SSR02  (*(volatile uint16_t *)(SCI0_BASE + 0x04))
#define SSR03  (*(volatile uint16_t *)(SCI0_BASE + 0x06))

#define SIR00  (*(volatile uint16_t *)(SCI0_BASE + 0x08))
#define SIR01  (*(volatile uint16_t *)(SCI0_BASE + 0x0A))
#define SIR02  (*(volatile uint16_t *)(SCI0_BASE + 0x0C))
#define SIR03  (*(volatile uint16_t *)(SCI0_BASE + 0x0E))

#define SDR00  (*(volatile uint16_t *)(SCI0_BASE + 0x210))
#define SDR01  (*(volatile uint16_t *)(SCI0_BASE + 0x212))
#define SDR02  (*(volatile uint16_t *)(SCI0_BASE + 0x214))
#define SDR03  (*(volatile uint16_t *)(SCI0_BASE + 0x216))

#define SPS0   (*(volatile uint16_t *)(SCI0_BASE + 0x26))
#define SOE0   (*(volatile uint16_t *)(SCI0_BASE + 0x2A))
#define SS0    (*(volatile uint16_t *)(SCI0_BASE + 0x22))
#define ST0    (*(volatile uint16_t *)(SCI0_BASE + 0x24))
#define SE0    (*(volatile uint16_t *)(SCI0_BASE + 0x20))

// Hàm khởi tạo đầy đủ
I2C_Error_t I2C_Init_Full(I2C_Channel_t channel, uint32_t scl_freq_hz) {
    I2C_Error_t err = I2C_OK;
    
    // 1. CONFIG GPIO (OPEN-DRAIN)
    GPIO_Config_For_I2C(channel);
    
    // 2. ENABLE PERIPHERAL CLOCK
    // SCI0: bit 0 của PER0, SCI1: bit 2, SCI2: bit 4 của PER2
    uint8_t unit = channel / 4;  // 0:SCI0, 1:SCI1, 2:SCI2
    uint8_t ch_num = channel % 4; // 0-3
    
    if(unit == 0) {
        PER0 |= (1 << 0);  // Enable SCI0
    } else if(unit == 1) {
        PER2 |= (1 << 2);  // Enable SCI1
    } else {
        PER2 |= (1 << 4);  // Enable SCI2
    }
    
    // 3. CALCULATE BAUD RATE DIVIDER
    // FMCK thường = FCLK = 32MHz (check datasheet)
    uint32_t fmck = 32000000;  // System clock
    uint16_t divider = (fmck / (2 * scl_freq_hz)) - 1;
    if(divider > 127) divider = 127;
    
    // 4. CONFIGURE SMRmn REGISTER
    volatile uint16_t *smr_reg = NULL;
    volatile uint16_t *scr_reg = NULL;
    volatile uint16_t *sdr_reg = NULL;
    volatile uint16_t *ssr_reg = NULL;
    volatile uint16_t *sir_reg = NULL;
    volatile uint16_t *soe_reg = NULL;
    volatile uint16_t *ss_reg = NULL;
    
    // Xác định thanh ghi dựa trên channel
    if(channel == IIC00) {
        smr_reg = &SMR00; scr_reg = &SCR00; sdr_reg = &SDR00;
        ssr_reg = &SSR00; sir_reg = &SIR00;
        soe_reg = &SOE0; ss_reg = &SS0;
    } else if(channel == IIC01) {
        smr_reg = &SMR01; scr_reg = &SCR01; sdr_reg = &SDR01;
        ssr_reg = &SSR01; sir_reg = &SIR01;
        soe_reg = &SOE0; ss_reg = &SS0;
    }
    // ... thêm các channel khác
    
    // 5. CONFIGURE SMRmn: Simplified I²C Mode
    // 0x0024 = 0000 0000 0010 0100b
    // Bit 2-1 = 10 (Simplified I²C), Bit 5 = 1 (fixed), Bit 3 = 0 (transfer end interrupt)
    *smr_reg = 0x0024;
    
    // 6. CONFIGURE SCRmn: Serial Communication Run Setting
    // Reset value: 0x0087
    *scr_reg = 0x0087;  // Giữ giá trị mặc định
    
    // 7. CONFIGURE BAUD RATE IN SDRmn[15:9]
    // Clear lower 9 bits, set divider in bits 15:9
    *sdr_reg = ((uint16_t)divider << 9);
    
    // 8. ENABLE OUTPUT FOR CHANNEL
    // SOE0: bit0 cho channel 0, bit1 cho channel 1, ...
    *soe_reg |= (1 << ch_num);
    
    // 9. CLEAR ANY PENDING FLAGS
    *sir_reg = 0xFFFF;  // Clear all flags
    
    // 10. START CHANNEL
    *ss_reg |= (1 << ch_num);
    
    // 11. WAIT FOR CHANNEL TO BE READY
    uint32_t timeout = 10000;
    while(!(*ssr_reg & 0x0020)) {  // Wait for ready flag
        if(--timeout == 0) {
            err = I2C_ERR_TIMEOUT;
            break;
        }
    }
    
    return err;
}


// Hàm tạo Start Condition
void I2C_Generate_Start_Condition(uint8_t channel) {
    I2C_PinConfig_t pins = I2C_PinMap[channel];
    
    // Get port data registers
    volatile uint8_t *p_scl = (volatile uint8_t *)(0x40000300 + pins.scl_port);
    volatile uint8_t *p_sda = (volatile uint8_t *)(0x40000300 + pins.sda_port);
    
    // 📌 START CONDITION SEQUENCE:
    // 1. Đảm bảo bus free (SCL=1, SDA=1)
    *p_sda = 1;
    *p_scl = 1;
    Delay_us(5);  // tHIGH > 4.0µs
    
    // 2. SDA chuyển từ 1 xuống 0 trong khi SCL vẫn = 1
    *p_sda = 0;   // SDA = 0 (start)
    Delay_us(5);  // tHD;STA > 4.0µs
    
    // 3. Kéo SCL xuống 0 để chuẩn bị truyền data
    *p_scl = 0;   // SCL = 0
    Delay_us(5);  // tLOW > 4.7µs
}

// Hàm tạo Stop Condition
void I2C_Generate_Stop_Condition(uint8_t channel) {
    I2C_PinConfig_t pins = I2C_PinMap[channel];
    
    volatile uint8_t *p_scl = (volatile uint8_t *)(0x40000300 + pins.scl_port);
    volatile uint8_t *p_sda = (volatile uint8_t *)(0x40000300 + pins.sda_port);
    
    // 📌 STOP CONDITION SEQUENCE:
    // 1. Đảm bảo SCL=0, SDA=0
    *p_scl = 0;
    *p_sda = 0;
    Delay_us(5);  // tLOW > 4.7µs
    
    // 2. Kéo SCL lên 1
    *p_scl = 1;
    Delay_us(5);  // tSU;STO > 4.0µs
    
    // 3. SDA chuyển từ 0 lên 1 trong khi SCL=1
    *p_sda = 1;   // SDA = 1 (stop)
    Delay_us(5);  // tBUF > 4.7µs (bus free time)
}

// Hàm tạo Repeated Start
void I2C_Generate_Repeated_Start(uint8_t channel) {
    // Repeated Start = Stop + Start
    I2C_Generate_Stop_Condition(channel);
    Delay_us(5);
    I2C_Generate_Start_Condition(channel);
}

//Truyền 1 Byte
I2C_Error_t I2C_Transmit_Byte(uint8_t channel, uint8_t data) {
    volatile uint16_t *sdr_reg = NULL;
    volatile uint16_t *ssr_reg = NULL;
    volatile uint16_t *sir_reg = NULL;
    
    // Xác định thanh ghi theo channel
    if(channel == IIC00) {
        sdr_reg = &SDR00; ssr_reg = &SSR00; sir_reg = &SIR00;
    } else if(channel == IIC01) {
        sdr_reg = &SDR01; ssr_reg = &SSR01; sir_reg = &SIR01;
    }
    // ... các channel khác
    
    // 1. WRITE DATA TO SDRmn REGISTER
    *sdr_reg = (uint16_t)data;
    
    // 2. WAIT FOR TRANSMISSION COMPLETE
    uint32_t timeout = 10000;  // Timeout counter
    while(1) {
        uint16_t status = *ssr_reg;
        
        // Check if transfer complete
        if(status & 0x0002) {  // TRC bit = 1 (Transfer Complete)
            break;
        }
        
        // Check for errors
        if(status & 0x0010) {  // ACK error
            *sir_reg = 0x0010;  // Clear ACK error flag
            return I2C_ERR_ACK;
        }
        
        if(status & 0x0004) {  // OVF error (Overflow)
            *sir_reg = 0x0004;  // Clear OVF error flag
            return I2C_ERR_OVF;
        }
        
        // Timeout check
        if(--timeout == 0) {
            return I2C_ERR_TIMEOUT;
        }
    }
    
    // 3. CLEAR TRANSFER COMPLETE FLAG
    *sir_reg = 0x0002;
    
    return I2C_OK;
}

//Nhận 1 Byte
uint8_t I2C_Receive_Byte(uint8_t channel, bool send_ack) {
    volatile uint16_t *sdr_reg = NULL;
    volatile uint16_t *ssr_reg = NULL;
    volatile uint16_t *soe_reg = NULL;
    uint8_t ch_num = channel % 4;
    
    // Xác định thanh ghi
    if(channel == IIC00) {
        sdr_reg = &SDR00; ssr_reg = &SSR00; soe_reg = &SOE0;
    } else if(channel == IIC01) {
        sdr_reg = &SDR01; ssr_reg = &SSR01; soe_reg = &SOE0;
    }
    // ... các channel khác
    
    // 🔴 QUAN TRỌNG: Simplified I²C không tự động generate ACK
    // Phải control ACK bằng SOEm register
    
    // 1. CONFIGURE ACK/NACK BEFORE RECEIVING
    if(send_ack) {
        // Send ACK (master sẽ kéo SDA xuống 0 sau byte này)
        // Enable output để có thể kéo SDA xuống
        *soe_reg |= (1 << ch_num);
    } else {
        // Send NACK (master để SDA = 1 sau byte này)
        // Đây là byte cuối → disable ACK output
        *soe_reg &= ~(1 << ch_num);
    }
    
    // 2. WAIT FOR RECEPTION COMPLETE
    // Trong receive mode, việc đọc SDRmn sẽ trigger reception
    // Chờ cờ transfer complete
    uint32_t timeout = 10000;
    while(!(*ssr_reg & 0x0002)) {  // Wait TRC bit
        if(--timeout == 0) {
            return 0xFF;  // Timeout error
        }
    }
    
    // 3. READ RECEIVED DATA
    uint8_t received_data = (uint8_t)(*sdr_reg & 0x00FF);
    
    // 4. CLEAR TRANSFER COMPLETE FLAG
    volatile uint16_t *sir_reg = (volatile uint16_t *)((uint32_t)ssr_reg + 0x08);
    *sir_reg = 0x0002;
    
    return received_data;
}

//Truyền nhiều Bytes
I2C_Error_t I2C_Master_Transmit(uint8_t channel, uint8_t slave_addr,
                               uint8_t *data, uint16_t len,
                               bool send_stop) {
    I2C_Error_t err;
    
    // 1. GENERATE START CONDITION
    I2C_Generate_Start_Condition(channel);
    
    // 2. SEND SLAVE ADDRESS + WRITE BIT (0)
    uint8_t addr_byte = (slave_addr << 1) | 0x00;  // Write = 0
    err = I2C_Transmit_Byte(channel, addr_byte);
    if(err != I2C_OK) {
        I2C_Generate_Stop_Condition(channel);
        return err;
    }
    
    // 3. SEND DATA BYTES
    for(uint16_t i = 0; i < len; i++) {
        err = I2C_Transmit_Byte(channel, data[i]);
        if(err != I2C_OK) {
            if(send_stop) I2C_Generate_Stop_Condition(channel);
            return err;
        }
    }
    
    // 4. GENERATE STOP CONDITION (nếu cần)
    if(send_stop) {
        I2C_Generate_Stop_Condition(channel);
    }
    
    return I2C_OK;
}
///Nhận nhiều Bytes
I2C_Error_t I2C_Master_Receive(uint8_t channel, uint8_t slave_addr,
                              uint8_t *buffer, uint16_t len) {
    I2C_Error_t err;
    
    // 1. GENERATE START CONDITION
    I2C_Generate_Start_Condition(channel);
    
    // 2. SEND SLAVE ADDRESS + READ BIT (1)
    uint8_t addr_byte = (slave_addr << 1) | 0x01;  // Read = 1
    err = I2C_Transmit_Byte(channel, addr_byte);
    if(err != I2C_OK) {
        I2C_Generate_Stop_Condition(channel);
        return err;
    }
    
    // 3. RECEIVE DATA BYTES
    for(uint16_t i = 0; i < len; i++) {
        // Send ACK cho tất cả byte trừ byte cuối
        bool send_ack = (i < (len - 1));
        
        // Nhận byte
        buffer[i] = I2C_Receive_Byte(channel, send_ack);
        
        // Kiểm tra timeout
        if(buffer[i] == 0xFF) {
            I2C_Generate_Stop_Condition(channel);
            return I2C_ERR_TIMEOUT;
        }
    }
    
    // 4. GENERATE STOP CONDITION
    I2C_Generate_Stop_Condition(channel);
    
    return I2C_OK;
}

//Ghi rồi đọc (Write then Read) - Cho EEPROM, cảm biến...
I2C_Error_t I2C_Write_Then_Read(uint8_t channel, uint8_t slave_addr,
                               uint8_t *tx_data, uint16_t tx_len,
                               uint8_t *rx_buffer, uint16_t rx_len) {
    I2C_Error_t err;
    
    // 1. WRITE PHASE (gửi địa chỉ/command)
    I2C_Generate_Start_Condition(channel);
    
    // Gửi slave address + write
    err = I2C_Transmit_Byte(channel, (slave_addr << 1) | 0x00);
    if(err != I2C_OK) {
        I2C_Generate_Stop_Condition(channel);
        return err;
    }
    
    // Gửi data
    for(uint16_t i = 0; i < tx_len; i++) {
        err = I2C_Transmit_Byte(channel, tx_data[i]);
        if(err != I2C_OK) {
            I2C_Generate_Stop_Condition(channel);
            return err;
        }
    }
    
    // 2. REPEATED START (không dùng Stop/Start)
    I2C_Generate_Repeated_Start(channel);
    
    // 3. READ PHASE
    // Gửi slave address + read
    err = I2C_Transmit_Byte(channel, (slave_addr << 1) | 0x01);
    if(err != I2C_OK) {
        I2C_Generate_Stop_Condition(channel);
        return err;
    }
    
    // Nhận data
    for(uint16_t i = 0; i < rx_len; i++) {
        bool send_ack = (i < (rx_len - 1));
        rx_buffer[i] = I2C_Receive_Byte(channel, send_ack);
        
        if(rx_buffer[i] == 0xFF) {  // Timeout
            I2C_Generate_Stop_Condition(channel);
            return I2C_ERR_TIMEOUT;
        }
    }
    
    // 4. STOP CONDITION
    I2C_Generate_Stop_Condition(channel);
    
    return I2C_OK;
}