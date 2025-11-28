#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "leds.h"
#include "display.h"
#include "buttons.h"
#include "usart.h"
#include "sound.h"

#define DOT_DURATION 100    
#define DASH_DURATION 400    
#define SYMBOL_SPACE 100  
#define LETTER_SPACE 600     
#define THINKING_TIME 2000  

const char* morse_codes[] = {
    ".-",    // A
    "-...",  // B
    "-.-.",  // C
    "-..",   // D
    ".",     // E
    "..-.",  // F
    "--.",   // G
    "....",  // H
    "..",    // I
    ".---",  // J
    "-.-",   // K
    ".-..",  // L
    "--",    // M
    "-.",    // N
    "---",   // O
    ".--.",  // P
    "--.-",  // Q
    ".-.",   // R
    "...",   // S
    "-",     // T
    "..-",   // U
    "...-",  // V
    ".--",   // W
    "-..-",  // X
    "-.--",  // Y
    "--.."   // Z
};

const char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void countdown_pattern(void);
void show_morse_code(const char* morse);
void show_morse_for_character(char character);
void led_dance(void);
void show_morse_for_string(const char* str);

int main(void) {
    leds_init();
    display_init();
    buttons_init();
    usart_init();
    buzzer_init();
    
    srand(TCNT0);

    _delay_ms(500);
    display_string("MRSE");
    _delay_ms(500);
    
    play_startup_sequence();
    
    countdown_pattern();
    
    for (uint8_t i = 0; i < 10; i++) {
        char round_display[5];
        sprintf(round_display, "RND%d", i + 1);
        display_string(round_display);
        
        for (uint16_t j = 0; j < 500; j++) {
            display_update(1);
            _delay_ms(1);
        }
        
        uint8_t random_index = rand() % 26;
        char selected_char = characters[random_index];
        
        show_morse_for_character(selected_char);
      
        display_string("WAIT");
        
        for (uint16_t j = 0; j < THINKING_TIME; j++) {
            display_update(1);
            _delay_ms(1);
        }
        
        char answer_display[5] = "ANS ";
        answer_display[3] = selected_char;
        display_string(answer_display);
        
        printf("Round %d - Answer: %c\r\n", i + 1, selected_char);
        
        for (uint16_t j = 0; j < 500; j++) {
            display_update(1);
            _delay_ms(1);
        }
    }
    
    display_string("DONE");
    _delay_ms(1000);
    
    led_dance();

    display_string("DEMO");
    _delay_ms(1000);
    show_morse_for_string("SOS");
    
    display_string("END ");
    _delay_ms(2000);
    
    display_string("    ");
    
    while (1) {
        buttons_check();
        display_update(1);
    }
    return 0;
}

/**
 * Display a countdown pattern on the LEDs
 * Shows 4 LEDs, then 3, 2, 1, and 0 before starting
 */
void countdown_pattern(void) {
    display_string("REDY");
    
    led_on(LED_PLAY_PIN);
    led_on(LED_TRACK_PIN);
    led_on(LED_SEEK_PIN);
    led_on(LED_STATUS_PIN);
    
    for (uint16_t i = 0; i < 300; i++) {
        display_update(1);
        _delay_ms(1);
    }
    
    led_off(LED_STATUS_PIN);
    display_string("  3 ");
    
    for (uint16_t i = 0; i < 300; i++) {
        display_update(1);
        _delay_ms(1);
    }
    
    led_off(LED_SEEK_PIN);
    display_string("  2 ");
    
    for (uint16_t i = 0; i < 300; i++) {
        display_update(1);
        _delay_ms(1);
    }
    
    led_off(LED_TRACK_PIN);
    display_string("  1 ");
    
    for (uint16_t i = 0; i < 300; i++) {
        display_update(1);
        _delay_ms(1);
    }
    
    led_off(LED_PLAY_PIN);
    display_string(" GO ");
    
    for (uint16_t i = 0; i < 300; i++) {
        display_update(1);
        _delay_ms(1);
    }
}

/**
 * Display a Morse code sequence on all LEDs
 * @param morse The Morse code string (dots and dashes)
 */
void show_morse_code(const char* morse) {
    for (uint8_t i = 0; morse[i] != '\0'; i++) {
        led_on(LED_PLAY_PIN);
        led_on(LED_TRACK_PIN);
        led_on(LED_SEEK_PIN);
        led_on(LED_STATUS_PIN);
        
        if (morse[i] == '.') {
            playTone(C6, DOT_DURATION);

        } else if (morse[i] == '-') {
            playTone(C5, DASH_DURATION);
        }
        
        led_off(LED_PLAY_PIN);
        led_off(LED_TRACK_PIN);
        led_off(LED_SEEK_PIN);
        led_off(LED_STATUS_PIN);
        
        for (uint16_t j = 0; j < SYMBOL_SPACE; j++) {
            display_update(1);
            _delay_ms(1);
        }
    }

    for (uint16_t j = 0; j < (LETTER_SPACE - SYMBOL_SPACE); j++) {
        display_update(1);
        _delay_ms(1);
    }
}

/**
 * Show Morse code for a specific character
 * @param character The character to show in Morse code (A-Z)
 */
void show_morse_for_character(char character) {
    if (character >= 'A' && character <= 'Z') {
        uint8_t index = character - 'A';
        show_morse_code(morse_codes[index]);
    } else if (character >= 'a' && character <= 'z') {
        uint8_t index = character - 'a';
        show_morse_code(morse_codes[index]);
    }
}

/**
 * Show a fun LED animation at the end of the program
 * Knight Rider style animation
 */
void led_dance(void) {
    uint8_t leds[] = {
        LED_PLAY_PIN,
        LED_TRACK_PIN,
        LED_SEEK_PIN,
        LED_STATUS_PIN
    };
    
    uint8_t num_leds = 4;
    
    display_string("DANC");
    
    for (uint8_t cycle = 0; cycle < 3; cycle++) {
        for (uint8_t i = 0; i < num_leds; i++) {
            led_on(leds[i]);
            playTone(C5 + (i * 100), 100);
            
            for (uint16_t j = 0; j < 150; j++) {
                display_update(1);
                _delay_ms(1);
            }
            
            led_off(leds[i]);
        }
        
        for (int8_t i = num_leds - 2; i >= 0; i--) {
            led_on(leds[i]);
            playTone(C5 + (i * 100), 100);
            
            for (uint16_t j = 0; j < 150; j++) {
                display_update(1);
                _delay_ms(1);
            }
            
            led_off(leds[i]);
        }
    }
    
    display_string("TADA");
    for (uint8_t i = 0; i < 3; i++) {
        for (uint8_t j = 0; j < num_leds; j++) {
            led_on(leds[j]);
        }
        
        playTone(C6, 200);
        
        for (uint16_t j = 0; j < 200; j++) {
            display_update(1);
            _delay_ms(1);
        }
        
        for (uint8_t j = 0; j < num_leds; j++) {
            led_off(leds[j]);
        }
        
        for (uint16_t j = 0; j < 200; j++) {
            display_update(1);
            _delay_ms(1);
        }
    }
}

/**
 * Show Morse code for an entire string (Extension requirement)
 * @param str The string to show in Morse code
 */
void show_morse_for_string(const char* str) {
    printf("Showing Morse for string: %s\r\n", str);
    
    for (uint8_t i = 0; str[i] != '\0'; i++) {
        char char_display[5] = "    ";
        char_display[0] = str[i];
        display_string(char_display);
        
        for (uint16_t j = 0; j < 500; j++) {
            display_update(1);
            _delay_ms(1);
        }
   
        show_morse_for_character(str[i]);
        
        for (uint16_t j = 0; j < LETTER_SPACE; j++) {
            display_update(1);
            _delay_ms(1);
        }
    }
}