#include "ssd1306.h"
#include "i2c.h"

/* Open a transaction and send the control byte that selects command mode. */
static ssd1306_status_t cmd_begin(uint8_t address)
{
    if (i2cStart() != I2C_START) {
        i2cStop();
        return SSD1306_ERR_START;
    }
    if (i2cSendAddress(address) != I2C_SLA_W_ACK) {
        i2cStop();
        return SSD1306_ERR_ADDRESS;
    }
    if (i2cWriteByte(SSD1306_CTRL_CMD) != I2C_DATA_ACK) {
        i2cStop();
        return SSD1306_ERR_CONTROL;
    }
    return SSD1306_OK;
}

/* Send one command byte inside an already-open command stream. */
static ssd1306_status_t cmd_byte(uint8_t byte)
{
    if (i2cWriteByte(byte) != I2C_DATA_ACK) {
        i2cStop();
        return SSD1306_ERR_COMMAND;
    }
    return SSD1306_OK;
}

const char *ssd1306_status_string(ssd1306_status_t status)
{
    switch (status) {
    case SSD1306_OK:               return "OK";
    case SSD1306_ERR_START:        return "START failed (bus busy or SDA held low)";
    case SSD1306_ERR_ADDRESS:      return "no ACK from panel address";
    case SSD1306_ERR_CONTROL:      return "control byte not ACKed";
    case SSD1306_ERR_COMMAND:      return "command byte not ACKed";
    case SSD1306_ERR_DATA:         return "GDDRAM data byte not ACKed";
    case SSD1306_ERR_COLUMN_RANGE: return "set column range failed";
    case SSD1306_ERR_PAGE_RANGE:   return "set page range failed";
    default:                       return "unknown status";
    }
}

ssd1306_status_t ssd1306_init(uint8_t address)
{
    ssd1306_status_t s;

    s = cmd_begin(address);
    if (s != SSD1306_OK) {
        return s;
    }

    /* 0x20 0x00 -- horizontal addressing: the column pointer auto-increments
     * and rolls into the next page, so a full frame is one flat 1024-byte
     * stream with no per-page cursor reset. */
    s = cmd_byte(0x20u); if (s != SSD1306_OK) { return s; }
    s = cmd_byte(0x00u); if (s != SSD1306_OK) { return s; }

    /* 0x8D 0x14 -- charge pump ON. Skip this and every byte still ACKs while
     * the panel stays dark: it has no high-voltage rail to drive the OLED. */
    s = cmd_byte(0x8Du); if (s != SSD1306_OK) { return s; }
    s = cmd_byte(0x14u); if (s != SSD1306_OK) { return s; }

    /* 0xAF -- display ON. Everything else runs on power-on reset defaults,
     * which are correct for a 128x64 panel. A 128x32 panel would also need
     * 0xA8 (multiplex ratio) and 0xDA (COM pin config). */
    s = cmd_byte(0xAFu); if (s != SSD1306_OK) { return s; }

    i2cStop();
    return SSD1306_OK;
}

ssd1306_status_t ssd1306_set_column_range(uint8_t start, uint8_t end, uint8_t address)
{
    ssd1306_status_t s;

    s = cmd_begin(address);
    if (s != SSD1306_OK) {
        return s;
    }

    s = cmd_byte(0x21u); if (s != SSD1306_OK) { return s; }   /* set column address */
    s = cmd_byte(start); if (s != SSD1306_OK) { return s; }
    s = cmd_byte(end);   if (s != SSD1306_OK) { return s; }

    i2cStop();
    return SSD1306_OK;
}

ssd1306_status_t ssd1306_set_page_range(uint8_t start, uint8_t end, uint8_t address)
{
    ssd1306_status_t s;

    s = cmd_begin(address);
    if (s != SSD1306_OK) {
        return s;
    }

    s = cmd_byte(0x22u); if (s != SSD1306_OK) { return s; }   /* set page address */
    s = cmd_byte(start); if (s != SSD1306_OK) { return s; }
    s = cmd_byte(end);   if (s != SSD1306_OK) { return s; }

    i2cStop();
    return SSD1306_OK;
}

ssd1306_status_t ssd1306_fill(uint8_t pattern, uint8_t address)
{
    uint16_t i;

    if (ssd1306_set_column_range(0x00u, SSD1306_WIDTH - 1u, address) != SSD1306_OK) {
        return SSD1306_ERR_COLUMN_RANGE;
    }
    if (ssd1306_set_page_range(0x00u, SSD1306_PAGES - 1u, address) != SSD1306_OK) {
        return SSD1306_ERR_PAGE_RANGE;
    }

    /* One transaction for the whole frame. Co = 0 in the 0x40 control byte
     * means the panel keeps interpreting everything after it as data, so the
     * 1024 bytes stream without re-addressing. */
    if (i2cStart() != I2C_START) {
        i2cStop();
        return SSD1306_ERR_START;
    }
    if (i2cSendAddress(address) != I2C_SLA_W_ACK) {
        i2cStop();
        return SSD1306_ERR_ADDRESS;
    }
    if (i2cWriteByte(SSD1306_CTRL_DATA) != I2C_DATA_ACK) {
        i2cStop();
        return SSD1306_ERR_CONTROL;
    }

    for (i = 0; i < SSD1306_BUF_LEN; i++) {
        if (i2cWriteByte(pattern) != I2C_DATA_ACK) {
            i2cStop();
            return SSD1306_ERR_DATA;
        }
    }

    i2cStop();
    return SSD1306_OK;
}
