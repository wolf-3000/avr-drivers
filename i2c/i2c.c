#include <avr/io.h>
#include "i2c.h"

#ifndef F_CPU
#error "F_CPU must be defined on the command line: -DF_CPU=16000000UL"
#endif

#ifndef I2C_SCL_HZ
#define I2C_SCL_HZ 100000UL
#endif

/* Spin budget for TWINT. At 100 kHz one byte takes ~90 us, so ~1440 cycles at
 * 16 MHz. 20000 is generous enough for 100 kHz yet still bounded. */
#define I2C_SPIN_LIMIT 20000u

/* Returns 0 if TWINT went high, 1 on timeout. */
static uint8_t i2cWaitFlag(void)
{
    uint16_t spin = I2C_SPIN_LIMIT;

    while (!(TWCR & (1 << TWINT)))
    {
        if (--spin == 0u)
        {
            return 1u;
        }
    }
    return 0u;
}

void i2cInit(void)
{
    /* SCL = F_CPU / (16 + 2 * TWBR * 4^TWPS), with TWPS = 0.
     * 16 MHz / 100 kHz -> TWBR = 72. */
    TWSR = 0; /* prescaler = 1 */
    TWBR = (uint8_t)(((F_CPU / I2C_SCL_HZ) - 16UL) / 2UL);
    TWCR = (1 << TWEN);
}

uint8_t i2cStart(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    if (i2cWaitFlag())
    {
        return I2C_TIMEOUT;
    }
    return (uint8_t)(TWSR & 0xF8u);
}

uint8_t i2cSendAddress(uint8_t address)
{
    TWDR = address;                    /* already shifted, R/W in bit 0 */
    TWCR = (1 << TWINT) | (1 << TWEN); /* clear flag, transmit; no TWSTA */
    if (i2cWaitFlag())
    {
        return I2C_TIMEOUT;
    }
    return (uint8_t)(TWSR & 0xF8u);
}

uint8_t i2cWriteByte(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (i2cWaitFlag())
    {
        return I2C_TIMEOUT;
    }
    return (uint8_t)(TWSR & 0xF8u);
}

void i2cStop(void)
{
    uint16_t spin = I2C_SPIN_LIMIT;

    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);

    /* TWINT is NOT set after a STOP. The hardware clears TWSTO once the bus
     * is released, so that is what gets polled here -- bounded, so a slave
     * clamping SDA low cannot wedge the CPU permanently. */
    while (TWCR & (1 << TWSTO))
    {
        if (--spin == 0u)
        {
            return;
        }
    }
}

uint8_t i2cTransmit(uint8_t address, uint8_t data)
{
    uint8_t status;

    status = i2cStart();
    if (status != I2C_START)
    {
        i2cStop();
        return status;
    }

    status = i2cSendAddress(address);
    if (status != I2C_SLA_W_ACK)
    {
        i2cStop();
        return status;
    }

    status = i2cWriteByte(data);
    if (status != I2C_DATA_ACK)
    {
        i2cStop();
        return status;
    }

    i2cStop();
    return 0u;
}

uint8_t i2cReadNack(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN); // no TWEA = NACK after this byte
    while (!(TWCR & (1 << TWINT)))
        ;
    return TWDR; // returns DATA, not status
}

uint8_t i2cReadAck(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)))
        ;
    return TWDR;
    // same as above, but set TWEA so the master ACKs
}

uint8_t i2cRepeatedStart(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)))
        ;
    return (TWSR & 0xF8); // expect 0x10
}

uint8_t mpuReadRegister(uint8_t reg)
{
    uint8_t data;

    if (i2cStart() != I2C_START)
    {
        i2cStop();
        return 0;
    }
    if (i2cSendAddress(0xD0) != I2C_SLA_W_ACK)
    {
        i2cStop();
        return 0;
    }
    if (i2cWriteByte(reg) != I2C_DATA_ACK)
    {
        i2cStop();
        return 0;
    }

    if (i2cRepeatedStart() != 0x10)
    {
        i2cStop();
        return 0;
    }
    if (i2cSendAddress(0xD1) != 0x40)
    {
        i2cStop();
        return 0;
    }

    data = i2cReadNack();
    i2cStop();
    return data;
}
