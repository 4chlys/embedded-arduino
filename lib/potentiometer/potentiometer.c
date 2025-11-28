/**
 * Potentiometer Input Implementation for DJ Controller
 */

#include "potentiometer.h"
#include "leds.h"
#include "display.h"
#include "commands.h"
#include <stdlib.h>
#include <util/delay.h>

static uint16_t baselineValue = 0;
static uint8_t potInitialized = 0;
static uint16_t seekCooldown = 0; 

void potentiometer_init(void) {
    // Set potentiometer pin as input (no pull-up)
    DDRC &= ~(1 << POT_PIN);
    PORTC &= ~(1 << POT_PIN);
    
    // Select AVcc as reference
    ADMUX = (1 << REFS0);
    
    // Enable ADC, set prescaler to 128 (16MHz/128 = 125kHz)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    
    baselineValue = read_adc(POT_PIN);
    potInitialized = 0;
    seekCooldown = 0;
}

uint16_t read_adc(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC));

    return ADC;
}

int32_t map_value(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void potentiometer_check(void) {
    if (seekCooldown > 0) {
        seekCooldown--;
        return;
    }
    
    uint16_t currentValue = read_adc(POT_PIN);
    
    if (!potInitialized) {
        baselineValue = currentValue;
        potInitialized = 1;
        
        char initStr[5];
        sprintf(initStr, "P%03d", currentValue % 1000);
        display_message(initStr, 300);
        return;
    }
    
    int16_t netChange = (int16_t)currentValue - (int16_t)baselineValue;
    
    if (abs(netChange) >= 10) {
        if (netChange > 0) {
            send_command(CMD_REQUEST_SEEK_FWD);
            display_message("SFWD", 100);
        } else {
            send_command(CMD_REQUEST_SEEK_BWD);
            display_message("SBWD", 100);
        }
     
        baselineValue = currentValue;
        seekCooldown = 100;
        
        led_on(LED_SEEK_PIN);
        for (uint8_t i = 0; i < 30; i++) {
            display_update(1);
        }
        led_off(LED_SEEK_PIN);
    }
}