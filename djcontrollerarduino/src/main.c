#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "leds.h"
#include "display.h"
#include "buttons.h"
#include "potentiometer.h"
#include "usart.h"
#include "commands.h"
#include "sound.h"
#include "playlist.h"

volatile uint16_t beat_counter = 0;
volatile uint8_t beat_detected = 0;

void timer1_init(void);
void init_all_peripherals(void);
void perform_startup_sequence(void);

ISR(TIMER1_COMPA_vect) {
    beat_counter++;
    
    if ((beat_counter % 500) == 0) {
        if ((beat_counter % 2000) == 0) {
            beat_detected = 1;
        }
    }
}

int main(void) {
    buzzer_init();
    leds_init();
    display_init();
    buttons_init();
    potentiometer_init();
    usart_init();
    commands_init();
    timer1_init();
    
    leds_test();
    
    led_off(LED_PLAY_PIN);
    led_off(LED_TRACK_PIN);
    led_off(LED_SEEK_PIN);
    led_off(LED_STATUS_PIN);
    
    display_message("INIT", 300);

    play_startup_sequence();

    uint8_t max_tracks = 20;
    Playlist* playlist = playlist_create(max_tracks);
    
    commands_set_playlist(&playlist);
    
    display_string("PAUS");
    
    send_command(CMD_REQUEST_STATUS);
    
    sei();
    
    while (1) {
        display_update(1);
        buttons_check();
        potentiometer_check();
        process_serial();

        if (beat_detected) {
            beat_detected = 0;
            
            led_on(LED_STATUS_PIN);
            for (uint8_t i = 0; i < 25; i++) {
                _delay_ms(1);
            }
            led_off(LED_STATUS_PIN);
        }
        playlist_check_update(playlist);
    }
    playlist_destroy(playlist);
    
    return 0;
}

void timer1_init(void) {
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12) | (1 << CS10);
    OCR1A = 15;  // 16MHz / 1024 / 16 ≈ 1ms
    TIMSK1 |= (1 << OCIE1A);
}