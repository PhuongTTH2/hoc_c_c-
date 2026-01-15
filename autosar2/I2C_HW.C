#include "I2C_HW.h"
#include "Mcu.h"

/* ========================== PRIVATE FUNCTIONS ========================== */

/* Configure GPIO pins for I2C */
static void ConfigureI2CPins(uint8 channelId)
{
    if (channelId == 0)  /* I2C0: P20=SCL0, P21=SDA0 */
    {
        /* Configure P20 (SCL0) */
        PM20 &= ~0x01;      /* Set as input mode */
        PMC20 |= 0x01;      /* Enable input buffer */
        P20CFG = 0x02;      /* Select I2C SCL0 function */
        POM20 |= 0x01;      /* Enable open-drain for P20 */
        
        /* Configure P21 (SDA0) */
        PM21 &= ~0x01;
        PMC21 |= 0x01;
        P21CFG = 0x02;      /* Select I2C SDA0 function */
        POM21 |= 0x01;      /* Enable open-drain for P21 */
    }
    else if (channelId == 1)  /* I2C1: P22=SCL1, P23=SDA1 */
    {
        /* Configure P22 (SCL1) */
        PM22 &= ~0x01;
        PMC22 |= 0x01;
        P22CFG = 0x02;      /* Select I2C SCL1 function */
        POM22 |= 0x01;
        
        /* Configure P23 (SDA1) */
        PM23 &= ~0x01;
        PMC23 |= 0x01;
        P23CFG = 0x02;      /* Select I2C SDA1 function */
        POM23 |= 0x01;
    }
}

/* Get register pointer for channel */
static void* GetI2CRegisters(uint8 channelId)
{
    return (channelId == 0) ? (void*)I2C0_REGS : (void*)I2C1_REGS;
}

/* Enable peripheral clock */
static void EnableI2CClock(uint8 channelId)
{
    if (channelId == 0)
    {
        PER0 |= PER0_IICA0EN;
    }
    else if (channelId == 1)
    {
        PER0 |= PER0_IICA1EN;
    }
}

/* Disable peripheral clock */
static void DisableI2CClock(uint8 channelId)
{
    if (channelId == 0)
    {
        PER0 &= ~PER0_IICA0EN;
    }
    else if (channelId == 1)
    {
        PER0 &= ~PER0_IICA1EN;
    }
}

/* Reset I2C peripheral */
static void ResetI2C(uint8 channelId)
{
    if (channelId == 0)
    {
        I2C0_REGS->IICCTL01 |= IICCTL1_IICRS;
        for(volatile uint32 i = 0; i < 100; i++);  /* Wait */
        I2C0_REGS->IICCTL01 &= ~IICCTL1_IICRS;
    }
    else if (channelId == 1)
    {
        I2C1_REGS->IICCTL11 |= IICCTL1_IICRS;
        for(volatile uint32 i = 0; i < 100; i++);
        I2C1_REGS->IICCTL11 &= ~IICCTL1_IICRS;
    }
}

/* Calculate baudrate timing */
static void SetI2CBaudrate(uint8 channelId, uint32 baudrate)
{
    uint32 pclk = Mcu_GetPclkFrequency();  /* Get peripheral clock (e.g., 16MHz) */
    uint32 divider;
    uint8 lowWidth, highWidth;
    
    /* Calculate divider */
    divider = pclk / baudrate;
    
    if (baudrate <= 100000)  /* Standard mode (100kHz) */
    {
        lowWidth = (uint8)((divider * 3) / 4);  /* 75% low */
        highWidth = (uint8)(divider / 4);       /* 25% high */
    }
    else  /* Fast mode (400kHz) */
    {
        lowWidth = (uint8)((divider * 3) / 8);  /* 37.5% low */
        highWidth = (uint8)(divider / 8);       /* 12.5% high */
    }
    
    /* Apply limits */
    if (lowWidth < 1) lowWidth = 1;
    if (highWidth < 1) highWidth = 1;
    if (lowWidth > 255) lowWidth = 255;
    if (highWidth > 255) highWidth = 255;
    
    /* Set timing registers */
    if (channelId == 0)
    {
        I2C0_REGS->IICWL0 = lowWidth;
        I2C0_REGS->IICWH0 = highWidth;
    }
    else if (channelId == 1)
    {
        I2C1_REGS->IICWL1 = lowWidth;
        I2C1_REGS->IICWH1 = highWidth;
    }
}

/* ========================== PUBLIC FUNCTIONS ========================== */

void I2C_Hw_InitChannel(uint8 channelId, uint32 baudrate, uint8 mode)
{
    /* Configure GPIO pins */
    ConfigureI2CPins(channelId);
    
    /* Enable peripheral clock */
    EnableI2CClock(channelId);
    
    /* Reset peripheral */
    ResetI2C(channelId);
    
    /* Set baudrate */
    SetI2CBaudrate(channelId, baudrate);
    
    /* Configure mode and enable */
    if (channelId == 0)
    {
        I2C0_Registers_Type* regs = I2C0_REGS;
        
        if (mode == 0)  /* Controller mode */
        {
            regs->IICCTL00 = 0x00;  /* Controller, write mode */
        }
        else  /* Target mode */
        {
            regs->IICCTL00 = IICCTL0_IICM;  /* Target mode */
            regs->SVA0 = 0x50;  /* Set slave address */
        }
        
        /* Enable interrupts */
        regs->IICCTL01 |= (IICCTL1_IICE | IICCTL1_TRXE);
        
        /* Enable I2C */
        regs->IICCTL00 |= IICCTL0_EN;
    }
    else if (channelId == 1)
    {
        I2C1_Registers_Type* regs = I2C1_REGS;
        
        if (mode == 0)
        {
            regs->IICCTL10 = 0x00;
        }
        else
        {
            regs->IICCTL10 = IICCTL0_IICM;
            regs->SVA1 = 0x50;
        }
        
        regs->IICCTL11 |= (IICCTL1_IICE | IICCTL1_TRXE);
        regs->IICCTL10 |= IICCTL0_EN;
    }
}

void I2C_Hw_StartTransmit(uint8 channelId, uint8 slaveAddr, const uint8* data, uint16 length)
{
    if (channelId == 0)
    {
        I2C0_Registers_Type* regs = I2C0_REGS;
        
        /* Set slave address */
        regs->IICA0 = slaveAddr << 1;  /* 7-bit address, LSB=0 for write */
        
        /* Clear direction bit (write mode) */
        regs->IICCTL00 &= ~IICCTL0_TRX;
        
        /* Enable acknowledge */
        regs->IICCTL00 |= IICCTL0_ACK;
        
        /* Write first data byte if available */
        if (length > 0 && data != NULL)
        {
            regs->IICF0 = data[0];
        }
        
        /* Generate START condition */
        regs->IICCTL00 |= IICCTL0_STA;
    }
    else if (channelId == 1)
    {
        I2C1_Registers_Type* regs = I2C1_REGS;
        
        regs->IICA1 = slaveAddr << 1;
        regs->IICCTL10 &= ~IICCTL0_TRX;
        regs->IICCTL10 |= IICCTL0_ACK;
        
        if (length > 0 && data != NULL)
        {
            regs->IICF1 = data[0];
        }
        
        regs->IICCTL10 |= IICCTL0_STA;
    }
}

void I2C_Hw_StartReceive(uint8 channelId, uint8 slaveAddr, uint8* buffer, uint16 length)
{
    if (channelId == 0)
    {
        I2C0_Registers_Type* regs = I2C0_REGS;
        
        /* Set slave address with read bit (LSB=1) */
        regs->IICA0 = (slaveAddr << 1) | 0x01;
        
        /* Set direction to read */
        regs->IICCTL00 |= IICCTL0_TRX;
        
        /* Enable acknowledge */
        regs->IICCTL00 |= IICCTL0_ACK;
        
        /* Generate START condition */
        regs->IICCTL00 |= IICCTL0_STA;
    }
    else if (channelId == 1)
    {
        I2C1_Registers_Type* regs = I2C1_REGS;
        
        regs->IICA1 = (slaveAddr << 1) | 0x01;
        regs->IICCTL10 |= IICCTL0_TRX;
        regs->IICCTL10 |= IICCTL0_ACK;
        regs->IICCTL10 |= IICCTL0_STA;
    }
}

void I2C_Hw_WriteData(uint8 channelId, uint8 data)
{
    if (channelId == 0)
    {
        I2C0_REGS->IICF0 = data;
    }
    else if (channelId == 1)
    {
        I2C1_REGS->IICF1 = data;
    }
}

uint8 I2C_Hw_ReadData(uint8 channelId)
{
    if (channelId == 0)
    {
        return I2C0_REGS->IICF0;
    }
    else if (channelId == 1)
    {
        return I2C1_REGS->IICF1;
    }
    return 0;
}

void I2C_Hw_GenerateStart(uint8 channelId)
{
    if (channelId == 0)
    {
        I2C0_REGS->IICCTL00 |= IICCTL0_STA;
    }
    else if (channelId == 1)
    {
        I2C1_REGS->IICCTL10 |= IICCTL0_STA;
    }
}

void I2C_Hw_GenerateStop(uint8 channelId)
{
    if (channelId == 0)
    {
        I2C0_REGS->IICCTL00 |= IICCTL0_STO;
    }
    else if (channelId == 1)
    {
        I2C1_REGS->IICCTL10 |= IICCTL0_STO;
    }
}

void I2C_Hw_EnableAck(uint8 channelId)
{
    if (channelId == 0)
    {
        I2C0_REGS->IICCTL00 |= IICCTL0_ACK;
    }
    else if (channelId == 1)
    {
        I2C1_REGS->IICCTL10 |= IICCTL0_ACK;
    }
}

void I2C_Hw_DisableAck(uint8 channelId)
{
    if (channelId == 0)
    {
        I2C0_REGS->IICCTL00 &= ~IICCTL0_ACK;
    }
    else if (channelId == 1)
    {
        I2C1_REGS->IICCTL10 &= ~IICCTL0_ACK;
    }
}

boolean I2C_Hw_IsBusBusy(uint8 channelId)
{
    uint8 status;
    
    if (channelId == 0)
    {
        status = I2C0_REGS->IICS0;
    }
    else
    {
        status = I2C1_REGS->IICS1;
    }
    
    return (status & IICS_BUSY) ? TRUE : FALSE;
}

boolean I2C_Hw_IsTransferComplete(uint8 channelId)
{
    uint8 status;
    
    if (channelId == 0)
    {
        status = I2C0_REGS->IICS0;
    }
    else
    {
        status = I2C1_REGS->IICS1;
    }
    
    return (status & IICS_TRC) ? TRUE : FALSE;
}

boolean I2C_Hw_IsNackReceived(uint8 channelId)
{
    uint8 status;
    
    if (channelId == 0)
    {
        status = I2C0_REGS->IICS0;
    }
    else
    {
        status = I2C1_REGS->IICS1;
    }
    
    return (status & IICS_NACK) ? TRUE : FALSE;
}

uint8 I2C_Hw_GetStatus(uint8 channelId)
{
    if (channelId == 0)
    {
        return I2C0_REGS->IICS0;
    }
    else if (channelId == 1)
    {
        return I2C1_REGS->IICS1;
    }
    return 0;
}

void I2C_Hw_ClearStatus(uint8 channelId)
{
    if (channelId == 0)
    {
        I2C0_REGS->IICS0 = 0x00;
    }
    else if (channelId == 1)
    {
        I2C1_REGS->IICS1 = 0x00;
    }
}