/**
 * LED Control Library for DJ Controller
 * 
 * Provides functions for controlling the LED indicators
 */

#ifndef LEDS_H
#define LEDS_H

#include <avr/io.h>
#include <util/delay.h>

#define LED_PLAY_PIN 2   // PB2
#define LED_TRACK_PIN 3  // PB3
#define LED_SEEK_PIN 4   // PB4
#define LED_STATUS_PIN 5 // PB5

/**
 * Initialize the LED pins as outputs
 */
void leds_init(void);

/**
 * Turn on an LED
 * @param pin The pin number of the LED to turn on
 */
void led_on(uint8_t pin);

/**
 * Turn off an LED
 * @param pin The pin number of the LED to turn off
 */
void led_off(uint8_t pin);

/**
 * Toggle an LED's state
 * @param pin The pin number of the LED to toggle
 */
void led_toggle(uint8_t pin);

/**
 * Flash test pattern on all LEDs (for startup)
 */
void leds_test(void);

/**
 * Flash an LED briefly
 * @param pin The pin number of the LED to flash
 * @param duration_ms Duration to keep the LED on in milliseconds
 */
void flash_led_briefly(uint8_t pin, uint16_t duration_ms);

#endif