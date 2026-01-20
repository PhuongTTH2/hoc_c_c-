// ============================================
// I2C/IICA INPUT CONFIGURATION STRUCTURE
// ============================================

/**
 * @brief Main configuration structure for I2C/IICA peripheral
 * @details Combines all configuration parameters from both IICA (Full I2C)
 *          and Simplified I2C implementations
 */
typedef struct {
    // =================== BASIC CONFIGURATION ===================
    IIC_Peripheral_t peripheral;   // Type of I2C peripheral (Full/Simplified)
    IIC_Channel_t    channel;      // Channel index (0-7 for Simplified I2C, 0-1 for IICA)
    IIC_Mode_t       mode;         // Master/Slave mode
    
    // =================== CLOCK CONFIGURATION ===================
    uint32_t         system_clock_hz;     // System clock frequency (Hz)
    uint32_t         i2c_freq_hz;         // Desired I2C frequency (Hz)
    uint32_t         timeout_ms;          // Communication timeout (ms)
    
    // =================== ADDRESS CONFIGURATION ===================
    union {
        struct {
            uint8_t  slave_address_7bit;  // 7-bit slave address for standard mode
            uint16_t slave_address_10bit; // 10-bit slave address for extended mode
        };
        uint8_t      own_address;         // Own address (for slave mode)
    };
    
    bool             use_10bit_addressing;  // Enable 10-bit addressing
    
    // =================== GPIO PIN CONFIGURATION ===================
    struct {
        uint8_t      scl_port;            // GPIO port for SCL (0-5)
        uint8_t      scl_pin;             // GPIO pin for SCL (0-7)
        uint8_t      sda_port;            // GPIO port for SDA (0-5)
        uint8_t      sda_pin;             // GPIO pin for SDA (0-7)
        
        // Pin mode configuration
        bool         open_drain_enabled;  // Enable open-drain mode
        bool         pullup_enabled;      // Enable internal pull-up
        bool         fast_mode_enabled;   // Enable fast mode (400kHz)
    } gpio;
    
    // =================== INTERRUPT CONFIGURATION ===================
    struct {
        bool         tx_complete_irq;     // Enable TX complete interrupt
        bool         rx_complete_irq;     // Enable RX complete interrupt
        bool         error_irq;           // Enable error interrupt
        bool         stop_irq;            // Enable stop condition interrupt
        uint8_t      irq_priority;        // Interrupt priority (0-15)
    } interrupts;
    
    // =================== ADVANCED CONFIGURATION ===================
    struct {
        bool         clock_stretching;    // Enable clock stretching
        bool         multi_master;        // Enable multi-master support
        bool         arbitration_check;   // Enable arbitration loss detection
        bool         general_call;        // Respond to general call address
    } features;
    
    // =================== TIMING CONFIGURATION ===================
    struct {
        uint8_t      scl_high_period;     // SCL high period (in cycles)
        uint8_t      scl_low_period;      // SCL low period (in cycles)
        uint8_t      data_hold_time;      // Data hold time (ns)
        uint8_t      data_setup_time;     // Data setup time (ns)
    } timing;
    
    // =================== CALLBACK FUNCTIONS ===================
    struct {
        void (*tx_complete_cb)(void);     // TX complete callback
        void (*rx_complete_cb)(void);     // RX complete callback
        void (*error_cb)(IIC_Error_t);    // Error callback
        void (*stop_cb)(void);            // Stop condition callback
        void (*address_match_cb)(void);   // Address match callback (slave)
    } callbacks;
    
    // =================== DEBUG CONFIGURATION ===================
    struct {
        bool         enable_debug;        // Enable debug output
        uint32_t     debug_baudrate;      // Debug UART baudrate
    } debug;
    
} IIC_Config_t;

// ============================================
// ENUM DEFINITIONS
// ============================================

/**
 * @brief I2C Peripheral Type
 */
typedef enum {
    IIC_PERIPHERAL_FULL_I2C = 0,      // Full I2C (IICA peripheral)
    IIC_PERIPHERAL_SIMPLIFIED_I2C = 1 // Simplified I2C (SCI channels)
} IIC_Peripheral_t;

/**
 * @brief I2C Channel Selection
 * @note For Full I2C: 0-1 (IICA0, IICA1)
 *       For Simplified I2C: 0-7 (IIC00 to IIC31)
 */
typedef enum {
    // Full I2C Channels
    IICA_CHANNEL_0 = 0,
    IICA_CHANNEL_1 = 1,
    
    // Simplified I2C Channels (SCI0)
    IIC00 = 0,  // SCI0 Channel 0
    IIC01 = 1,  // SCI0 Channel 1
    IIC10 = 2,  // SCI0 Channel 2
    IIC11 = 3,  // SCI0 Channel 3
    
    // Simplified I2C Channels (SCI1)
    IIC20 = 4,  // SCI1 Channel 0
    IIC21 = 5,  // SCI1 Channel 1
    
    // Simplified I2C Channels (SCI2)
    IIC30 = 6,  // SCI2 Channel 0
    IIC31 = 7,  // SCI2 Channel 1
} IIC_Channel_t;

/**
 * @brief I2C Operating Mode
 */
typedef enum {
    IIC_MODE_MASTER = 0,
    IIC_MODE_SLAVE = 1
} IIC_Mode_t;

/**
 * @brief I2C Error Codes
 */
typedef enum {
    IIC_OK = 0,
    IIC_ERR_BUS_BUSY,
    IIC_ERR_ARBITRATION_LOST,
    IIC_ERR_NACK,
    IIC_ERR_TIMEOUT,
    IIC_ERR_INVALID_STATE,
    IIC_ERR_START_FAILED,
    IIC_ERR_STOP_FAILED,
    IIC_ERR_OVF,
    IIC_ERR_FRAMING,
    IIC_ERR_PARITY,
    IIC_ERR_INVALID_PARAM,
    IIC_ERR_NOT_INITIALIZED
} IIC_Error_t;

// ============================================
// HELPER FUNCTIONS FOR CONFIGURATION
// ============================================

/**
 * @brief Default configuration for Full I2C (IICA) in Master mode
 */
IIC_Config_t IIC_DefaultConfig_FullI2C_Master = {
    .peripheral = IIC_PERIPHERAL_FULL_I2C,
    .channel = IICA_CHANNEL_0,
    .mode = IIC_MODE_MASTER,
    .system_clock_hz = 32000000,      // 32 MHz
    .i2c_freq_hz = 100000,            // 100 kHz standard
    .timeout_ms = 100,
    .slave_address_7bit = 0x50,       // Default EEPROM address
    .use_10bit_addressing = false,
    .gpio = {
        .scl_port = 3,
        .scl_pin = 3,                 // P03 for IIC00 SCL
        .sda_port = 3,
        .sda_pin = 4,                 // P04 for IIC00 SDA
        .open_drain_enabled = true,
        .pullup_enabled = true,
        .fast_mode_enabled = false
    },
    .interrupts = {
        .tx_complete_irq = false,
        .rx_complete_irq = false,
        .error_irq = false,
        .stop_irq = false,
        .irq_priority = 0
    },
    .features = {
        .clock_stretching = true,
        .multi_master = false,
        .arbitration_check = true,
        .general_call = false
    },
    .timing = {
        .scl_high_period = 4,         // Standard mode
        .scl_low_period = 4,
        .data_hold_time = 0,
        .data_setup_time = 250        // 250ns
    },
    .callbacks = {
        .tx_complete_cb = NULL,
        .rx_complete_cb = NULL,
        .error_cb = NULL,
        .stop_cb = NULL,
        .address_match_cb = NULL
    },
    .debug = {
        .enable_debug = false,
        .debug_baudrate = 115200
    }
};

/**
 * @brief Default configuration for Simplified I2C in Master mode
 */
IIC_Config_t IIC_DefaultConfig_SimplifiedI2C_Master = {
    .peripheral = IIC_PERIPHERAL_SIMPLIFIED_I2C,
    .channel = IIC00,                  // Default to IIC00
    .mode = IIC_MODE_MASTER,
    .system_clock_hz = 32000000,      // 32 MHz
    .i2c_freq_hz = 100000,            // 100 kHz
    .timeout_ms = 100,
    .slave_address_7bit = 0x50,
    .use_10bit_addressing = false,
    .gpio = {
        .scl_port = 3,
        .scl_pin = 3,                 // P03 for IIC00 SCL
        .sda_port = 3,
        .sda_pin = 4,                 // P04 for IIC00 SDA
        .open_drain_enabled = true,
        .pullup_enabled = true,
        .fast_mode_enabled = false
    },
    .interrupts = {
        .tx_complete_irq = false,
        .rx_complete_irq = false,
        .error_irq = true,            // Enable error IRQ for Simplified I2C
        .stop_irq = false,
        .irq_priority = 1
    },
    .features = {
        .clock_stretching = false,    // Simplified I2C doesn't support clock stretching
        .multi_master = false,
        .arbitration_check = true,
        .general_call = false
    },
    .timing = {
        .scl_high_period = 4,
        .scl_low_period = 4,
        .data_hold_time = 0,
        .data_setup_time = 250
    },
    .callbacks = {
        .tx_complete_cb = NULL,
        .rx_complete_cb = NULL,
        .error_cb = NULL,
        .stop_cb = NULL,
        .address_match_cb = NULL
    },
    .debug = {
        .enable_debug = false,
        .debug_baudrate = 115200
    }
};

/**
 * @brief Default configuration for Slave mode
 */
IIC_Config_t IIC_DefaultConfig_Slave = {
    .peripheral = IIC_PERIPHERAL_FULL_I2C,
    .channel = IICA_CHANNEL_1,        // Use IICA1 as slave
    .mode = IIC_MODE_SLAVE,
    .system_clock_hz = 32000000,
    .i2c_freq_hz = 100000,
    .timeout_ms = 100,
    .own_address = 0x30,              // Slave address
    .use_10bit_addressing = false,
    .gpio = {
        .scl_port = 5,
        .scl_pin = 0,                 // P50 for IICA1 SCL
        .sda_port = 5,
        .sda_pin = 1,                 // P51 for IICA1 SDA
        .open_drain_enabled = true,
        .pullup_enabled = true,
        .fast_mode_enabled = false
    },
    .interrupts = {
        .tx_complete_irq = true,
        .rx_complete_irq = true,
        .error_irq = true,
        .stop_irq = true,             // Important for slave
        .irq_priority = 2
    },
    .features = {
        .clock_stretching = true,
        .multi_master = false,
        .arbitration_check = false,
        .general_call = true          // Respond to general call
    },
    .timing = {
        .scl_high_period = 4,
        .scl_low_period = 4,
        .data_hold_time = 0,
        .data_setup_time = 250
    },
    .callbacks = {
        .tx_complete_cb = NULL,
        .rx_complete_cb = NULL,
        .error_cb = NULL,
        .stop_cb = NULL,
        .address_match_cb = NULL
    },
    .debug = {
        .enable_debug = false,
        .debug_baudrate = 115200
    }
};

// ============================================
// CONFIGURATION VALIDATION FUNCTIONS
// ============================================

/**
 * @brief Validate I2C configuration parameters
 * @param config Pointer to configuration structure
 * @return IIC_Error_t Validation result
 */
IIC_Error_t IIC_ValidateConfig(const IIC_Config_t *config) {
    if (config == NULL) {
        return IIC_ERR_INVALID_PARAM;
    }
    
    // Validate peripheral type
    if (config->peripheral > IIC_PERIPHERAL_SIMPLIFIED_I2C) {
        return IIC_ERR_INVALID_PARAM;
    }
    
    // Validate channel based on peripheral type
    if (config->peripheral == IIC_PERIPHERAL_FULL_I2C) {
        if (config->channel > IICA_CHANNEL_1) {
            return IIC_ERR_INVALID_PARAM;
        }
    } else { // Simplified I2C
        if (config->channel > IIC31) {
            return IIC_ERR_INVALID_PARAM;
        }
    }
    
    // Validate clock frequencies
    if (config->system_clock_hz == 0) {
        return IIC_ERR_INVALID_PARAM;
    }
    
    if (config->i2c_freq_hz == 0 || config->i2c_freq_hz > 400000) {
        return IIC_ERR_INVALID_PARAM; // Max 400kHz for I2C
    }
    
    // Validate GPIO pins
    if (config->gpio.scl_port > 5 || config->gpio.sda_port > 5) {
        return IIC_ERR_INVALID_PARAM; // Only ports 0-5
    }
    
    if (config->gpio.scl_pin > 7 || config->gpio.sda_pin > 7) {
        return IIC_ERR_INVALID_PARAM; // Only pins 0-7
    }
    
    // Validate slave address
    if (config->use_10bit_addressing) {
        if (config->slave_address_10bit > 0x3FF) {
            return IIC_ERR_INVALID_PARAM; // 10-bit max 0x3FF
        }
    } else {
        if (config->slave_address_7bit > 0x7F) {
            return IIC_ERR_INVALID_PARAM; // 7-bit max 0x7F
        }
    }
    
    return IIC_OK;
}

/**
 * @brief Initialize I2C peripheral with configuration
 * @param config Pointer to configuration structure
 * @return IIC_Error_t Initialization result
 */
IIC_Error_t IIC_Init(const IIC_Config_t *config) {
    IIC_Error_t err;
    
    // Validate configuration
    err = IIC_ValidateConfig(config);
    if (err != IIC_OK) {
        return err;
    }
    
    // Initialize based on peripheral type
    if (config->peripheral == IIC_PERIPHERAL_FULL_I2C) {
        return IICA_Init_FromConfig(config);
    } else {
        return IIC_Simplified_Init_FromConfig(config);
    }
}

// ============================================
// EXAMPLE USAGE
// ============================================

/**
 * @brief Example: Configure IICA0 as Master for EEPROM communication
 */
void Example_IICA_Master_EEPROM(void) {
    IIC_Config_t config = IIC_DefaultConfig_FullI2C_Master;
    
    // Customize for specific application
    config.channel = IICA_CHANNEL_0;
    config.i2c_freq_hz = 400000;        // Fast mode 400kHz
    config.slave_address_7bit = 0x50;   // EEPROM address
    config.gpio.fast_mode_enabled = true;
    config.timeout_ms = 50;
    
    // Initialize
    IIC_Error_t err = IIC_Init(&config);
    if (err != IIC_OK) {
        // Handle error
        while(1);
    }
    
    // Now ready to communicate with EEPROM
}

/**
 * @brief Example: Configure Simplified I2C for sensor reading
 */
void Example_SimplifiedI2C_Sensor(void) {
    IIC_Config_t config = IIC_DefaultConfig_SimplifiedI2C_Master;
    
    // Customize for sensor
    config.channel = IIC20;              // Use IIC20 channel
    config.slave_address_7bit = 0x76;    // BMP280 sensor address
    config.gpio.scl_port = 5;           // P55 for SCL
    config.gpio.scl_pin = 5;
    config.gpio.sda_port = 5;           // P54 for SDA
    config.gpio.sda_pin = 4;
    config.interrupts.error_irq = true;  // Enable error interrupt
    
    // Initialize
    IIC_Init(&config);
}

/**
 * @brief Example: Configure IICA as Slave device
 */
void Example_IICA_Slave(void) {
    IIC_Config_t config = IIC_DefaultConfig_Slave;
    
    // Customize for slave
    config.own_address = 0x30;           // Our slave address
    config.interrupts.tx_complete_irq = true;
    config.interrupts.rx_complete_irq = true;
    config.interrupts.stop_irq = true;
    config.callbacks.address_match_cb = Slave_AddressMatch_Handler;
    config.callbacks.rx_complete_cb = Slave_RxComplete_Handler;
    config.callbacks.tx_complete_cb = Slave_TxComplete_Handler;
    
    // Initialize
    IIC_Init(&config);
}