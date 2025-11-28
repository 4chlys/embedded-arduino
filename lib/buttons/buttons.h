/**
 * Button Input Library for DJ Controller
 * 
 * Provides functions for handling button inputs
 */

#ifndef BUTTONS_H
#define BUTTONS_H

#include <avr/io.h>

// Button pin definitions
#define BUTTON_PLAY_PIN 1  // PC1
#define BUTTON_NEXT_PIN 2  // PC2
#define BUTTON_PREV_PIN 3  // PC3

/**
 * Initialize the button pins as inputs with pull-ups
 */
void buttons_init(void);

/**
 * Read button state (returns 0 when pressed, 1 when released)
 * @param pin The pin number of the button to read
 * @return 0 if pressed, 1 if released
 */
uint8_t read_button(uint8_t pin);

/**
 * Process all button inputs and trigger necessary actions
 */
void buttons_check(void);

#endif