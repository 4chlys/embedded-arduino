/**
 * Button Input Implementation for DJ Controller
 */

#include "buttons.h"
#include "leds.h"
#include "display.h"
#include "commands.h"

static uint8_t lastButtonPlay = 1;
static uint8_t lastButtonNext = 1;
static uint8_t lastButtonPrev = 1;

static uint8_t buttonCooldown = 0;

// External variables from commands module
extern uint8_t isPlaying;

void buttons_init(void) {
    DDRC &= ~((1 << BUTTON_PLAY_PIN) | (1 << BUTTON_NEXT_PIN) | (1 << BUTTON_PREV_PIN));
    PORTC |= (1 << BUTTON_PLAY_PIN) | (1 << BUTTON_NEXT_PIN) | (1 << BUTTON_PREV_PIN);
    
    buttonCooldown = 0;
}

uint8_t read_button(uint8_t pin) {
    return (PINC & (1 << pin)) ? 1 : 0;
}

void buttons_check(void) {
    if (buttonCooldown > 0) {
        buttonCooldown--;
        return;
    }
    
    uint8_t buttonPlay = read_button(BUTTON_PLAY_PIN);
    
    if (buttonPlay == 0 && lastButtonPlay == 1) {
        if (isPlaying) {
            send_command(CMD_REQUEST_PAUSE);
            display_message("RPAU", 100);
        } else {
            send_command(CMD_REQUEST_PLAY);
            display_message("RPLY", 100);
        }
        
        led_toggle(LED_PLAY_PIN);
        buttonCooldown = 50;
    }
    lastButtonPlay = buttonPlay;

    uint8_t buttonNext = read_button(BUTTON_NEXT_PIN);
    
    if (buttonNext == 0 && lastButtonNext == 1) {
        send_command(CMD_REQUEST_NEXT);
        display_message("RNXT", 100);
        
        flash_led_briefly(LED_TRACK_PIN, 100);
        
        buttonCooldown = 50;
    }
    lastButtonNext = buttonNext;
    
    uint8_t buttonPrev = read_button(BUTTON_PREV_PIN);
    
    if (buttonPrev == 0 && lastButtonPrev == 1) {
        send_command(CMD_REQUEST_PREV);
        display_message("RPRV", 100);
  
        flash_led_briefly(LED_TRACK_PIN, 100);
        
        buttonCooldown = 50;
    }
    lastButtonPrev = buttonPrev;
}