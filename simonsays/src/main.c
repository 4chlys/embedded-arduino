#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "leds.h"
#include "display.h"
#include "buttons.h"
#include "usart.h"
#include "sound.h"

#define MAX_LEVEL 10
#define BLINK_SPEED 50
#define FEEDBACK_BLINKS 3
#define GAME_LED_1 LED_PLAY_PIN
#define GAME_LED_2 LED_TRACK_PIN
#define GAME_LED_3 LED_SEEK_PIN
#define GAME_LED_4 LED_STATUS_PIN
#define GAME_BUTTON_1 BUTTON_PLAY_PIN
#define GAME_BUTTON_2 BUTTON_NEXT_PIN
#define GAME_BUTTON_3 BUTTON_PREV_PIN

volatile uint8_t button_pushed = 0;
volatile uint32_t random_seed = 0;

void initGame(void);
void generatePuzzle(uint8_t* puzzle, uint8_t length);
void printPuzzle(uint8_t* puzzle, uint8_t length);
void playPuzzle(uint8_t* puzzle, uint8_t level);
uint8_t readInput(uint8_t* puzzle, uint8_t level);
void playWinSequence(void);
void playLoseSequence(void);
void waitForStart(void);

ISR(PCINT1_vect) {
    if (!(PINC & (1 << GAME_BUTTON_1)) ||
        !(PINC & (1 << GAME_BUTTON_2)) ||
        !(PINC & (1 << GAME_BUTTON_3))) {
        
        button_pushed = 1;
    }
}

int main(void) {
    leds_init();
    display_init();
    buttons_init();
    usart_init();
    buzzer_init();
    
    initGame();
    
    display_string("SIMN");
    
    for (uint16_t i = 0; i < 500; i++) {
        display_update(1);
        _delay_ms(1);
    }
    
    uint8_t puzzle[MAX_LEVEL];

    waitForStart();
    
    generatePuzzle(puzzle, MAX_LEVEL);
    
    transmit_string("Generated puzzle: ");
    printPuzzle(puzzle, MAX_LEVEL);
    transmit_string("\r\n");
 
    uint8_t currentLevel = 1;
    uint8_t gameOver = 0;
    
    while (!gameOver && currentLevel <= MAX_LEVEL) {
        char level_display[5];
        sprintf(level_display, "LV%02d", currentLevel);
        display_string(level_display);
        
        for (uint16_t i = 0; i < 300; i++) {
            display_update(1);
            _delay_ms(1);
        }
        
        playPuzzle(puzzle, currentLevel);
        
        uint8_t result = readInput(puzzle, currentLevel);
        
        if (result == 1) {
            for (uint8_t i = 0; i < FEEDBACK_BLINKS; i++) {
                led_on(GAME_LED_4);
                
                for (uint16_t j = 0; j < 50; j++) {
                    display_update(1);
                    _delay_ms(1);
                }
                
                led_off(GAME_LED_4);
                
                for (uint16_t j = 0; j < 50; j++) {
                    display_update(1);
                    _delay_ms(1);
                }
            }
            
            currentLevel++;
        
            if (currentLevel > MAX_LEVEL) {
                display_string("WIN ");
                
                transmit_string("Congratulations, you are the Simon Master!\r\n");
                
                for (uint16_t i = 0; i < 500; i++) {
                    display_update(1);
                    _delay_ms(1);
                }
        
                playWinSequence();
                
                gameOver = 1;
            } else {
                char levelMsg[50];
                sprintf(levelMsg, "Correct, we are going to level %d\r\n", currentLevel);
                transmit_string(levelMsg);
            }
        } else {
            display_string("FAIL");
           
            transmit_string("Wrong, the correct pattern was: [");
            for (uint8_t i = 0; i < currentLevel; i++) {
                char numStr[4];
                sprintf(numStr, "%d ", puzzle[i]);
                transmit_string(numStr);
            }
            transmit_string("]\r\n");
            
            for (uint16_t i = 0; i < 500; i++) {
                display_update(1);
                _delay_ms(1);
            }

            playLoseSequence();
            
            gameOver = 1;
        }
    }
  
    display_string("OVER");

    for (uint16_t i = 0; i < 1000; i++) {
        display_update(1);
        _delay_ms(1);
    }

    display_string("    ");
    
    while (1) {
        display_update(1);
    }
    
    return 0;
}

/**
 * Initialize the game components
 */
void initGame(void) {
    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << PCINT9) | (1 << PCINT10) | (1 << PCINT11);
    
    sei();
    
    button_pushed = 0;
   
    random_seed = 0;
}

/**
 * Wait for player to press a button to start the game
 */
void waitForStart(void) {
    display_string("STRT");
    
    transmit_string("Press button 1 to start the game\r\n");
 
    button_pushed = 0;
    
    while (!button_pushed) {
        led_on(GAME_LED_4);
        
        for (uint8_t i = 0; i < BLINK_SPEED / 10; i++) {
            display_update(1);
            _delay_ms(5); 
            random_seed++;
        }
        
        led_off(GAME_LED_4);

        for (uint8_t i = 0; i < BLINK_SPEED / 10; i++) {
            display_update(1);
            _delay_ms(5);  
            random_seed++; 
        }
    }
    srand(random_seed);
}

/**
 * Generate a random puzzle sequence
 * @param puzzle Array to store the puzzle
 * @param length Length of the puzzle
 */
void generatePuzzle(uint8_t* puzzle, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        puzzle[i] = rand() % 3;
    }
}

/**
 * Print puzzle array to Serial Monitor
 * @param puzzle Array containing the puzzle
 * @param length Length of the puzzle to print
 */
void printPuzzle(uint8_t* puzzle, uint8_t length) {
    transmit_string("[");
    for (uint8_t i = 0; i < length; i++) {
        char numStr[4];
        sprintf(numStr, "%d ", puzzle[i]);
        transmit_string(numStr);
    }
    transmit_string("]");
}

/**
 * Play the puzzle sequence on the LEDs
 * @param puzzle Array containing the puzzle
 * @param level Current level (number of elements to play)
 */
void playPuzzle(uint8_t* puzzle, uint8_t level) {
    display_string("PLAY");
 
    for (uint16_t i = 0; i < 100; i++) {
        display_update(1);
        _delay_ms(1);
    }
    
    for (uint8_t i = 0; i < level; i++) {
        led_off(GAME_LED_1);
        led_off(GAME_LED_2);
        led_off(GAME_LED_3);
        
        for (uint16_t j = 0; j < 50; j++) {
            display_update(1);
            _delay_ms(1);
        }
        
        switch (puzzle[i]) {
            case 0:
                led_on(GAME_LED_1);
                break;
            case 1:
                led_on(GAME_LED_2);
                break;
            case 2:
                led_on(GAME_LED_3);
                break;
        }
        
        for (uint16_t j = 0; j < 200; j++) {
            display_update(1);
            _delay_ms(1);
        }
   
        led_off(GAME_LED_1);
        led_off(GAME_LED_2);
        led_off(GAME_LED_3);
    }

    for (uint16_t i = 0; i < 200; i++) {
        display_update(1);
        _delay_ms(1);
    }
}

/**
 * Read the player's input and check if it matches the puzzle
 * @param puzzle Array containing the puzzle
 * @param level Current level (number of elements to check)
 * @return 1 if input matches the puzzle, 0 otherwise
 */
uint8_t readInput(uint8_t* puzzle, uint8_t level) {
    display_string("INPT");
    
    for (uint8_t i = 0; i < level; i++) {     
        uint8_t button_pressed = 0;
        uint8_t input_value = 0;
        uint8_t button_number = 0;
        
        while (!button_pressed) {
            if (read_button(GAME_BUTTON_1) == 0) {
                button_pressed = 1;
                input_value = 0;
                button_number = 1;
                led_on(GAME_LED_1);
            }
            else if (read_button(GAME_BUTTON_2) == 0) {
                button_pressed = 1;
                input_value = 1;
                button_number = 2;
                led_on(GAME_LED_2);
            }
            else if (read_button(GAME_BUTTON_3) == 0) {
                button_pressed = 1;
                input_value = 2;
                button_number = 3;
                led_on(GAME_LED_3);
            }
            
            display_update(1);
        }
    
        char buttonMsg[50];
        if (input_value == puzzle[i]) {
            sprintf(buttonMsg, "You have pressed button %d, correct!\r\n", button_number);
        } else {
            sprintf(buttonMsg, "You have pressed button %d, wrong!\r\n", button_number);
        }
        transmit_string(buttonMsg);
        
        for (uint16_t j = 0; j < 100; j++) {
            display_update(1);
            _delay_ms(1);
        }
        
        led_off(GAME_LED_1);
        led_off(GAME_LED_2);
        led_off(GAME_LED_3);
        
        if (input_value != puzzle[i]) { 
            return 0; 
        }
    }
    return 1;
}

/**
 * Play a win sequence animation with lights and sound
 */
void playWinSequence(void) {
    playTone(C5, 200);
    playTone(E5, 200);
    playTone(G5, 200);
    playTone(C6, 400);
    
    led_on(GAME_LED_1);
    led_on(GAME_LED_2);
    led_on(GAME_LED_3);
    led_on(GAME_LED_4);
    
    display_string("MSTR");
    for (uint16_t i = 0; i < 500; i++) {
        display_update(1);
        _delay_ms(1);
    }
    
    for (uint8_t i = 0; i < 3; i++) {
        led_off(GAME_LED_1);
        led_off(GAME_LED_2);
        led_off(GAME_LED_3);
        led_off(GAME_LED_4);
        
        for (uint16_t j = 0; j < 100; j++) {
            display_update(1);
            _delay_ms(1);
        }
        
        led_on(GAME_LED_1);
        led_on(GAME_LED_2);
        led_on(GAME_LED_3);
        led_on(GAME_LED_4);
        
        for (uint16_t j = 0; j < 100; j++) {
            display_update(1);
            _delay_ms(1);
        }
    }
    
    led_off(GAME_LED_1);
    led_off(GAME_LED_2);
    led_off(GAME_LED_3);
    led_off(GAME_LED_4);
}

/**
 * Play a lose sequence animation with lights and sound
 */
void playLoseSequence(void) {
    playTone(C5, 300);
    playTone(G5, 500);
    
    led_on(GAME_LED_1);
    led_on(GAME_LED_2);
    led_on(GAME_LED_3);
    led_on(GAME_LED_4);
    
    for (uint16_t i = 0; i < 500; i++) {
        display_update(1);
        _delay_ms(1);
    }
    
    led_off(GAME_LED_1);
    led_off(GAME_LED_2);
    led_off(GAME_LED_3);
    led_off(GAME_LED_4);
}