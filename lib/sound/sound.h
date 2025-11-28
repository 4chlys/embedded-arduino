/**
 * Sound Control Library for DJ Controller
 * 
 * Provides functions for controlling the piezo-electric speaker
 */

#ifndef SOUND_H
#define SOUND_H

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DURATION 250

#define  C5      523.250
#define  D5      587.330
#define  E5      659.250
#define  F5      698.460
#define  G5      783.990
#define  A5      880.000
#define  B5      987.770
#define  C6      1046.500

/**
 * Initialize the buzzer pin
 */
void buzzer_init(void);

/**
 * Play a tone at the specified frequency for the specified duration
 * @param frequency Frequency in Hz
 * @param duration Duration in milliseconds
 */
void playTone(float frequency, uint32_t duration);

/**
 * Play a startup sound sequence
 */
void play_startup_sequence(void);

#endif 