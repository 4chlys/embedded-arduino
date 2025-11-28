/**
 * USART Communication Library for DJ Controller
 * 
 * Provides functions for serial communication
 */

#ifndef USART_H
#define USART_H

#include <avr/io.h>

/**
 * Initialize the UART for serial communication
 */
void usart_init(void);

/**
 * Send a byte over UART
 * @param data The byte to send
 */
void transmit_byte(uint8_t data);

/**
 * Receive a byte from UART (blocking)
 * @return The received byte
 */
uint8_t receive_byte(void);

/**
 * Check if data is available to read
 * @return 1 if data is available, 0 otherwise
 */
uint8_t is_data_available(void);

/**
 * Transmit string over USART
 * @param str Pointer to null-terminated string
 */
void transmit_string(const char* str);

#endif