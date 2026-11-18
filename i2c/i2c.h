#ifndef I2C_H
#define I2C_H

#include <stdint.h>

/* TWSR status codes, already masked with 0xF8.
 * See ATmega328P datasheet Table 22-2 (Master Transmitter mode). */
#define I2C_START 0x08u      /* START transmitted                    */
#define I2C_REP_START 0x10u  /* Repeated START transmitted           */
#define I2C_SLA_W_ACK 0x18u  /* SLA+W transmitted, ACK received      */
#define I2C_SLA_W_NACK 0x20u /* SLA+W transmitted, NACK received     */
#define I2C_DATA_ACK 0x28u   /* Data byte transmitted, ACK received  */
#define I2C_DATA_NACK 0x30u  /* Data byte transmitted, NACK received */
#define I2C_ARB_LOST 0x38u   /* Arbitration lost                     */

/* Not a real TWSR value. TWSR's low 3 bits are prescaler/reserved, so every
 * genuine masked status is a multiple of 8 and 0xFF can never collide. */
#define I2C_TIMEOUT 0xFFu

void i2cInit(void);
uint8_t i2cStart(void);
uint8_t i2cRepeatedStart(void);
uint8_t i2cSendAddress(uint8_t address);
uint8_t i2cWriteByte(uint8_t data);
uint8_t i2cReadAck(void);
uint8_t i2cReadNack(void);
uint8_t mpuReadRegister(uint8_t reg);
void i2cStop(void);

/* Convenience: full single-byte write transaction.
 * Returns 0 on success, otherwise the TWSR code of the failing step. */
uint8_t i2cTransmit(uint8_t address, uint8_t data);

#endif /* I2C_H */
