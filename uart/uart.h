#ifndef UART_H
#define UART_H

#include <stdint.h>

/* Override on the command line with -DUART_BAUD=115200UL if wanted. */
#ifndef UART_BAUD
#define UART_BAUD 9600UL
#endif

void uartInit(void);
void uartSendChar(char c);
char uartReceiveChar(void);
void uartSendString(const char *str);
void uartSendHex(uint8_t value); /* two uppercase nibbles, no prefix */
void uartSendHex16(uint16_t value);

#endif /* UART_H */
