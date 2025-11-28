/**
 * Potentiometer Input Library for DJ Controller
 * 
 * Provides functions for handling potentiometer input
 */

#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include <avr/io.h>

// Potentiometer pin
#define POT_PIN 0   // PC0 (ADC0)

/**
 * Initialize the ADC for potentiometer reading
 */
void potentiometer_init(void);

/**
 * Read a value from the ADC
 * @param channel The ADC channel to read
 * @return The ADC conversion result (0-1023)
 */
uint16_t read_adc(uint8_t channel);

/**
 * Map a value from one range to another
 * @param x The value to map
 * @param in_min The minimum of the input range
 * @param in_max The maximum of the input range
 * @param out_min The minimum of the output range
 * @param out_max The maximum of the output range
 * @return The mapped value
 */
int32_t map_value(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);

/**
 * Check potentiometer position and handle seek
 */
void potentiometer_check(void);

#endif