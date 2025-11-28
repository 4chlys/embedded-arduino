#include "display.h"
#include "commands.h"
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

extern uint8_t isPlaying;

const uint8_t DIGIT_PATTERNS[] = {
    0xC0, // 0
    0xF9, // 1
    0xA4, // 2
    0xB0, // 3
    0x99, // 4
    0x92, // 5
    0x82, // 6
    0xF8, // 7
    0x80, // 8
    0x90  // 9
};

const uint8_t LETTER_PATTERNS[] = {
    0x88, // A
    0x83, // B
    0xC6, // C
    0xA1, // D
    0x86, // E
    0x8E, // F
    0xC2, // G
    0x89, // H
    0xCF, // I
    0xE1, // J
    0x8A, // K
    0xC7, // L
    0xEA, // M
    0xC8, // N
    0xC0, // O
    0x8C, // P
    0x98, // Q
    0xAF, // R
    0x92, // S
    0x87, // T
    0xC1, // U
    0xC1, // V
    0xD5, // W
    0x89, // X
    0x91, // Y
    0xA4  // Z
};

const uint8_t DIGIT_SELECT[] = {0xF1, 0xF2, 0xF4, 0xF8};

uint8_t displayBuffer[4] = {0xFF, 0xFF, 0xFF, 0xFF};

void display_init(void) {
    DDRD |= (1 << DISPLAY_LATCH_PIN) | (1 << DISPLAY_CLOCK_PIN);
    DDRB |= (1 << DISPLAY_DATA_PIN);
}

static void shift_out(uint8_t data) {
    for (int8_t i = 7; i >= 0; i--) {
        if (data & (1 << i)) {
            PORTB |= (1 << DISPLAY_DATA_PIN);
        } else {
            PORTB &= ~(1 << DISPLAY_DATA_PIN);
        }
        
        PORTD |= (1 << DISPLAY_CLOCK_PIN);
        _delay_us(1);
        PORTD &= ~(1 << DISPLAY_CLOCK_PIN);
        _delay_us(1);
    }
}

void display_update(uint8_t check_timeout) {
    for (uint8_t digit = 0; digit < 4; digit++) {
        PORTD &= ~(1 << DISPLAY_LATCH_PIN);
        shift_out(displayBuffer[digit]);
        shift_out(DIGIT_SELECT[digit]);
        PORTD |= (1 << DISPLAY_LATCH_PIN);
        _delay_ms(1);
    }
    
    // The check_timeout parameter is kept for compatibility but not used
}

void display_string(const char* str) {
    size_t len = strlen(str);

    for (uint8_t i = 0; i < 4; i++) {
        displayBuffer[i] = 0xFF;
    }
    
    for (uint8_t i = 0; i < len && i < 4; i++) {
        char c = str[i];
        
        if (c >= '0' && c <= '9') {
            displayBuffer[i] = DIGIT_PATTERNS[c - '0'];
        } else if (c >= 'A' && c <= 'Z') {
            displayBuffer[i] = LETTER_PATTERNS[c - 'A'];
        } else if (c >= 'a' && c <= 'z') {
            displayBuffer[i] = LETTER_PATTERNS[c - 'a'];
        } else if (c == ' ') {
            displayBuffer[i] = 0xFF; 
        }
    }
}

void display_message(const char* str, uint16_t display_time) {
    display_string(str);
    
    for (uint16_t i = 0; i < display_time; i++) {
        display_update(1);
    }

    if (isPlaying) {
        display_string("PLAY");
    } else {
        display_string("PAUS");
    }
}