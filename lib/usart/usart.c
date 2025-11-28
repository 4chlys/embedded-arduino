/**
 * USART Communication Implementation for DJ Controller
 */

#include "usart.h"

void usart_init(void) {
    // Set baud rate to 9600 bps for 16MHz clock
    UBRR0H = 0;
    UBRR0L = 103;  // 16MHz / (16 * 9600) - 1 = 103
    
    // Enable transmitter and receiver
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    
    // Set frame format: 8 data bits, 1 stop bit, no parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void transmit_byte(uint8_t data) {
    // Wait for transmit buffer to be empty
    while (!(UCSR0A & (1 << UDRE0)));
    
    // Load data into transmit register
    UDR0 = data;
}

uint8_t receive_byte(void) {
    // Wait for data to be received
    while (!(UCSR0A & (1 << RXC0)));
    
    // Get and return received data from buffer
    return UDR0;
}

uint8_t is_data_available(void) {
    return (UCSR0A & (1 << RXC0));
}

void transmit_string(const char* str) {
    while (*str) {
        transmit_byte(*str++);
    }
}