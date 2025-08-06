#ifndef RS485_H_INCLUDED
#define RS485_H_INCLUDED


#include <stdint.h>
#include <stdlib.h>


void rs485_init(void);
void rs485_write(uint8_t *buffer, size_t len);
int  rs485_read(uint8_t *buffer, size_t len, unsigned long timeout_ms);
void rs485_flush(void);
void rs485_flush_input(void);
void rs485_wait_tx_done(void);
void rs485_set_baudrate(uint32_t baudrate);


#endif
