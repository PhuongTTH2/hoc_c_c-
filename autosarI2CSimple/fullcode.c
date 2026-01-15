#include <stdint.h>

// ============================================
// 1. ĐỊA CHỈ THANH GHI CHO SIMPLIFIED I2C (IIC00)
// ============================================

// Base address cho Universal Serial Communication Unit
#define USCI_BASE       0x4004C000

// Thanh ghi cho IIC00 (Simplified I2C Channel 00)
#define SMR00           (*(volatile uint8_t *)(USCI_BASE + 0x00))  // Serial Mode Register 00
#define SCR00           (*(volatile uint8_t *)(USCI_BASE + 0x04))  // Serial Control Register 00
#define SDR00           (*(volatile uint8_t *)(USCI_BASE + 0x08))  // Serial Data Register 00
#define SSR00           (*(volatile uint8_t *)(USCI_BASE + 0x0C))  // Serial Status Register 00
#define SPS0            (*(volatile uint8_t *)(USCI_BASE + 0x10))  // Serial Clock Select Register 0

// PER0 - Peripheral Enable Register 0 (Enable IIC00)
#define PER0            (*(volatile uint8_t *)0x40040800)
#define PER0_IIC00_EN   0x00000001  // Bit enable cho IIC00 (check datasheet)

// ============================================
// 2. BIT ĐỊNH NGHĨA
// ============================================

// SSR00 Status bits
#define SSR00_TDRE      0x20  // Transmit Data Register Empty
#define SSR00_RDRF      0x10  // Receive Data Register Full
#define SSR00_ORER      0x08  // Overrun Error
#define SSR00_FER       0x04  // Framing Error
#define SSR00_PER       0x02  // Parity Error
#define SSR00_TEND      0x01  // Transmission End
#define SSR00_AL        0x80  // Arbitration Lost (for I2C)
#define SSR00_ACK       0x40  // Acknowledge bit status

// SMR00 Mode bits cho Simplified I2C
#define SMR00_CKE1      0x80  // Clock Enable 1
#define SMR00_CKE0      0x40  // Clock Enable 0
#define SMR00_MODE_I2C  0x38  // Mode select for Simplified I2C (b5=1, b4=1, b3=1)
#define SMR00_BC2       0x04  // Bit length
#define SMR00_BC1       0x02
#define SMR00_BC0       0x01

// SCR00 Control bits
#define SCR00_TE        0x20  // Transmit Enable
#define SCR00_RE        0x10  // Receive Enable
#define SCR00_MPB       0x08  // Multiprocessor bit
#define SCR00_CKE1      0x04  // Clock Enable 1
#define SCR00_CKE0      0x02  // Clock Enable 0
#define SCR00_SOE       0x01  // Serial Output Enable

// ============================================
// 3. KHỞI TẠO SIMPLIFIED I2C (IIC00)
// ============================================

void Simplified_I2C_Init_Master(uint32_t speed_khz) {
    // 1. Kích hoạt clock cho IIC00 trong PER0
    PER0 |= PER0_IIC00_EN;
    
    // 2. Cấu hình chân GPIO cho SDA00 và SCL00
    // Tùy theo PIORx setting, ví dụ SDA00=P03, SCL00=P04
    // Giả sử đã cấu hình PIOR cho IIC00
    
    // 3. Cấu hình tốc độ I2C trong SPS0
    // Fclk = 16MHz, target speed = 100kHz
    // BRR = Fclk / (speed * 2) - 1
    uint32_t brr = (16000000 / (speed_khz * 2000)) - 1;
    if (brr > 255) brr = 255;
    if (brr < 2) brr = 2;
    SPS0 = (uint8_t)brr;
    
    // 4. Cấu hình chế độ Simplified I2C Master
    SMR00 = 0;
    SMR00 |= SMR00_MODE_I2C;      // Chọn chế độ Simplified I2C
    SMR00 |= SMR00_BC2;           // 8-bit data format
    
    // 5. Kích hoạt I2C
    SCR00 = 0;
    SCR00 |= SCR00_TE;            // Transmit Enable
    SCR00 |= SCR00_RE;            // Receive Enable
    SCR00 |= SCR00_SOE;           // Serial Output Enable
}

// ============================================
// 4. HÀM GỬI START CONDITION
// ============================================

void I2C_Send_Start(void) {
    // Trong Simplified I2C, start condition được tạo tự động
    // khi ghi địa chỉ slave vào SDR00
    // Có thể cần set bit START trong thanh ghi khác (check datasheet)
    
    // Clear status flags
    SSR00 &= ~(SSR00_TEND | SSR00_AL | SSR00_ACK);
}

// ============================================
// 5. HÀM GỬI ĐỊA CHỈ SLAVE
// ============================================

uint8_t I2C_Send_Slave_Address(uint8_t slave_addr, uint8_t read_write) {
    // 0 = Write, 1 = Read
    
    // 1. Tạo Start condition (implied)
    I2C_Send_Start();
    
    // 2. Gửi địa chỉ slave + R/W bit
    uint8_t addr_byte = (slave_addr << 1) | (read_write & 0x01);
    SDR00 = addr_byte;
    
    // 3. Chờ truyền xong và check ACK
    while (!(SSR00 & SSR00_TEND));
    
    // 4. Kiểm tra ACK từ slave
    if (SSR00 & SSR00_ACK) {
        return 1;  // ACK nhận được
    } else {
        return 0;  // NACK - slave không phản hồi
    }
}

// ============================================
// 6. HÀM GHI DỮ LIỆU (MASTER TRANSMIT)
// ============================================

uint8_t I2C_Write_Data(uint8_t slave_addr, uint8_t *data, uint8_t len) {
    uint8_t i;
    
    // 1. Gửi địa chỉ slave với bit Write (0)
    if (!I2C_Send_Slave_Address(slave_addr, 0)) {
        return 0;  // Slave không ACK
    }
    
    // 2. Gửi từng byte dữ liệu
    for (i = 0; i < len; i++) {
        SDR00 = data[i];
        while (!(SSR00 & SSR00_TDRE));  // Chờ buffer trống
        while (!(SSR00 & SSR00_TEND));  // Chờ truyền xong
        
        // Check ACK sau mỗi byte
        if (!(SSR00 & SSR00_ACK)) {
            return 0;  // Slave NACK
        }
    }
    
    return 1;  // Thành công
}

// ============================================
// 7. HÀM ĐỌC DỮ LIỆU (MASTER RECEIVE)
// ============================================

uint8_t I2C_Read_Data(uint8_t slave_addr, uint8_t *buffer, uint8_t len) {
    uint8_t i;
    
    // 1. Gửi địa chỉ slave với bit Read (1)
    if (!I2C_Send_Slave_Address(slave_addr, 1)) {
        return 0;  // Slave không ACK
    }
    
    // 2. Nhận từng byte dữ liệu
    for (i = 0; i < len; i++) {
        // Đối với byte cuối cùng, gửi NACK
        if (i == len - 1) {
            // Set bit để gửi NACK (check datasheet)
            // Có thể cần cấu hình thanh ghi khác
        }
        
        // Chờ dữ liệu đến
        while (!(SSR00 & SSR00_RDRF));
        
        // Đọc dữ liệu
        buffer[i] = SDR00;
        
        // Clear flag
        SSR00 &= ~SSR00_RDRF;
    }
    
    return 1;  // Thành công
}

// ============================================
// 8. HÀM GỬI STOP CONDITION
// ============================================

void I2C_Send_Stop(void) {
    // Trong Simplified I2C, stop condition có thể được tạo
    // bằng cách set bit STOP trong thanh ghi điều khiển
    
    // Ví dụ (cần check datasheet):
    // SCR00 |= 0x80;  // Giả sử bit 7 là STOP
    
    // Hoặc đơn giản là disable I2C
    SCR00 &= ~(SCR00_TE | SCR00_RE);
}

// ============================================
// 9. VÍ DỤ SỬ DỤNG VỚI EEPROM 24C02 (0x50)
// ============================================

// Ghi dữ liệu vào EEPROM
uint8_t EEPROM_Write(uint16_t addr, uint8_t *data, uint8_t len) {
    uint8_t buffer[16];
    
    if (len > 16) return 0;  // Quá giới hạn
    
    // Tạo buffer với địa chỉ EEPROM trước
    buffer[0] = (uint8_t)(addr >> 8);   // High byte address
    buffer[1] = (uint8_t)(addr & 0xFF); // Low byte address
    
    // Copy dữ liệu
    for (uint8_t i = 0; i < len; i++) {
        buffer[2 + i] = data[i];
    }
    
    // Gửi qua I2C
    return I2C_Write_Data(0x50, buffer, len + 2);
}

// Đọc dữ liệu từ EEPROM
uint8_t EEPROM_Read(uint16_t addr, uint8_t *data, uint8_t len) {
    uint8_t addr_buffer[2];
    
    // 1. Gửi địa chỉ muốn đọc (Write mode)
    addr_buffer[0] = (uint8_t)(addr >> 8);
    addr_buffer[1] = (uint8_t)(addr & 0xFF);
    
    if (!I2C_Write_Data(0x50, addr_buffer, 2)) {
        return 0;
    }
    
    // 2. Đọc dữ liệu từ địa chỉ đó (Read mode)
    return I2C_Read_Data(0x50, data, len);
}

// ============================================
// 10. HÀM MAIN VÍ DỤ
// ============================================

int main(void) {
    uint8_t write_data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t read_data[4];
    uint8_t status;
    
    // Khởi tạo hệ thống clock trước
    SystemClock_Config();
    
    // Khởi tạo Simplified I2C với tốc độ 100kHz
    Simplified_I2C_Init_Master(100);
    
    // Ghi dữ liệu vào EEPROM tại địa chỉ 0x0100
    status = EEPROM_Write(0x0100, write_data, 4);
    
    if (status) {
        // Delay cho EEPROM write cycle (5ms)
        Delay_ms(5);
        
        // Đọc lại dữ liệu
        status = EEPROM_Read(0x0100, read_data, 4);
        
        if (status) {
            // So sánh dữ liệu
            for (int i = 0; i < 4; i++) {
                if (write_data[i] != read_data[i]) {
                    // Lỗi verify
                    while(1);
                }
            }
            // Thành công
        }
    }
    
    while(1) {
        // Ứng dụng chính
    }
}

// ============================================
// HÀM TRỢ GIÚP
// ============================================

void SystemClock_Config(void) {
    // Cấu hình system clock (cần implement theo datasheet)
}

void Delay_ms(uint32_t ms) {
    // Simple delay function
    for (volatile uint32_t i = 0; i < ms * 1000; i++);
}