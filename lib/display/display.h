#ifndef DISPLAY_H
#define DISPLAY_H

#include <avr/io.h>
#include <string.h>

#define DISPLAY_LATCH_PIN 4  // PD4
#define DISPLAY_CLOCK_PIN 7  // PD7
#define DISPLAY_DATA_PIN 0   // PB0

/**
 * Initialize the display pins
 */
void display_init(void);

/**
 * Shift out a byte of data to the display
 * @param data The byte to send
 */
static void shift_out(uint8_t data);

/**
 * Display a string (up to 4 characters)
 * @param str The string to display
 */
void display_string(const char* str);

/**
 * Update the display with current buffer contents
 * @param check_timeout If 1, checks and updates command response timeout
 */
void display_update(uint8_t check_timeout);

/**
 * Show a message on the display for a specified time
 * @param str The string to display
 * @param display_time Number of display refresh cycles to show message
 */
void display_message(const char* str, uint16_t display_time);

#endif