#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

/* 0x3C shifted left one place with the R/W bit cleared. address might not always be 0x3C */
#define SSD1306_ADDR_W 0x78u

#define SSD1306_WIDTH 128u
#define SSD1306_PAGES 8u
#define SSD1306_BUF_LEN ((uint16_t)SSD1306_WIDTH * SSD1306_PAGES) /* 1024 */

/* Control bytes: Co = 0 means the stream continues without repeating this. */
#define SSD1306_CTRL_CMD 0x00u  /* D/C# = 0 -> command stream */
#define SSD1306_CTRL_DATA 0x40u /* D/C# = 1 -> display data   */

/* Named failures. The old code returned bare integers 1..28 which told you
 * nothing at the terminal; these pair with ssd1306_status_string(). */
typedef enum
{
    SSD1306_OK = 0,
    SSD1306_ERR_START,        /* START condition not acknowledged by the bus */
    SSD1306_ERR_ADDRESS,      /* panel did not ACK its own address           */
    SSD1306_ERR_CONTROL,      /* control byte not ACKed                      */
    SSD1306_ERR_COMMAND,      /* a command byte not ACKed                    */
    SSD1306_ERR_DATA,         /* a GDDRAM data byte not ACKed                */
    SSD1306_ERR_COLUMN_RANGE, /* set-column-address transaction failed       */
    SSD1306_ERR_PAGE_RANGE    /* set-page-address transaction failed         */
} ssd1306_status_t;

const char *ssd1306_status_string(ssd1306_status_t status);

ssd1306_status_t ssd1306_init(uint8_t address);
ssd1306_status_t ssd1306_set_column_range(uint8_t start, uint8_t end, uint8_t address);
ssd1306_status_t ssd1306_set_page_range(uint8_t start, uint8_t end, uint8_t address);
ssd1306_status_t ssd1306_fill(uint8_t pattern, uint8_t address);

#endif /* SSD1306_H */
       /* dont forget the ssd1306 datasheet*/