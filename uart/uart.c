#include <avr/io.h>
#include "uart.h"

#ifndef F_CPU
#error "F_CPU must be defined on the command line: -DF_CPU=16000000UL"
#endif

/* Normal (non-2x) mode: UBRR = F_CPU / (16 * BAUD) - 1.
 * 16 MHz, 9600 baud -> 103, which is 0.2% error. */
#define UBRR_VALUE ((uint16_t)((F_CPU / (16UL * UART_BAUD)) - 1UL))

void uartInit(void)
{
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)(UBRR_VALUE);
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); /* 8N1 */
}

void uartSendChar(char c)
{
    while (!(UCSR0A & (1 << UDRE0)))
    {
        ; /* wait for the transmit buffer to empty */
    }
    UDR0 = (uint8_t)c;
}

char uartReceiveChar(void)
{
    while (!(UCSR0A & (1 << RXC0)))
    {
        ;
    }
    return (char)UDR0;
}

void uartSendString(const char *str)
{
    while (*str != '\0')
    {
        uartSendChar(*str);
        str++;
    }
}

void uartSendHex(uint8_t value)
{
    /* Lookup table instead of printf: two characters cost ~16 bytes here
     * versus ~1.5 kB of flash for the formatting machinery. */
    static const char hex[] = "0123456789ABCDEF";

    uartSendChar(hex[(value >> 4) & 0x0Fu]);
    uartSendChar(hex[value & 0x0Fu]);
}
