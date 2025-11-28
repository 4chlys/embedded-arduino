/**
 * Sound Control Implementation for DJ Controller
 */

#include "sound.h"
#include <avr/interrupt.h>

void buzzer_init(void) { 
    DDRD |= (1 << PD3);
    PORTD |= (1 << PD3);
}

void playTone(float frequency, uint32_t duration) {
    uint8_t sreg = SREG;
    cli();  // Disable interrupts for precise timing
    
    volatile uint32_t periodInMicro = (uint32_t)(1000000/frequency);
    volatile uint32_t durationInMicro = duration * 1000;
    
    for (volatile uint32_t i = 0; i < durationInMicro/periodInMicro; i++) {
        PORTD &= ~(1 << PD3);
        
        for (volatile uint16_t j = 0; j < periodInMicro/2/10; j++) {
            _delay_us(10);
        }
        
        PORTD |= (1 << PD3); 
        
        for (volatile uint16_t j = 0; j < periodInMicro/2/10; j++) {
            _delay_us(10);
        }
    }
    
    // Restore interrupt state
    SREG = sreg;
}

void play_startup_sequence(void) {
    DDRD |= (1 << PD3);
    PORTD |= (1 << PD3);
    
    _delay_ms(200);
    
    playTone(C5, 150);
    _delay_ms(50);
    playTone(E5, 150);
    _delay_ms(50);
    playTone(G5, 150);
    _delay_ms(50);
    playTone(C6, 200);
}