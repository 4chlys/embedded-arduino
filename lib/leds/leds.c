/**
 * LED Control Implementation for DJ Controller
 */

#include "leds.h"

void leds_init(void) {
    // Set LED pins as outputs
    DDRB |= (1 << LED_PLAY_PIN) | (1 << LED_TRACK_PIN) | 
            (1 << LED_SEEK_PIN) | (1 << LED_STATUS_PIN);
    
    // LEDs off initially (common anode - HIGH = off)
    PORTB |= (1 << LED_PLAY_PIN) | (1 << LED_TRACK_PIN) | 
             (1 << LED_SEEK_PIN) | (1 << LED_STATUS_PIN);
}

void led_on(uint8_t pin) {
    PORTB &= ~(1 << pin);  // Common anode - LOW = on
}

void led_off(uint8_t pin) {
    PORTB |= (1 << pin);  // Common anode - HIGH = off
}

void led_toggle(uint8_t pin) {
    PORTB ^= (1 << pin);
}

void leds_test(void) {
    led_on(LED_PLAY_PIN);
    led_on(LED_TRACK_PIN);
    led_on(LED_SEEK_PIN);
    led_on(LED_STATUS_PIN);
}

void flash_led_briefly(uint8_t pin, uint16_t duration_ms) {
    led_on(pin);
    _delay_ms(duration_ms);
    led_off(pin);
}