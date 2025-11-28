/**
 * DJ Controller - Utility functions for display and animations
 * Updated to work with new non-blocking display library
 */

 #define __AVR_ATmega328P__
#define __DELAY_BACKWARD_COMPATIBLE__

#include "utils.h"
#include <time.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "leds.h"
#include "buttons.h"
#include "display.h"
#include "potentiometer.h"
#include "usart.h"

#define LED_PLAYING 0  // PB2
#define LED_TRACK   1  // PB3
#define LED_SEEK    2  // PB4
#define LED_STATUS  3  // PB5

/**
 * Display the track number in the format "tr01" (Track 1)
 * Updated to use non-blocking display functions
 * 
 * @param trackNumber The track number to display (1-99)
 * @param duration How long to display in milliseconds (now handled without blocking)
 */
void displayTrackNumber(uint8_t trackNumber, uint16_t duration) {
    char trackStr[5] = "tr00";
    
    // Convert track number to ASCII digits
    if (trackNumber > 99) trackNumber = 99;
    trackStr[2] = '0' + (trackNumber / 10);
    trackStr[3] = '0' + (trackNumber % 10);
    
    // Display on the 4-digit display - non-blocking now
    writeString(trackStr);
}

/**
 * Display a scrolling text message on the 4-digit display
 * This function still blocks but uses the display buffer correctly
 * 
 * @param message The message to scroll
 * @param speed Scrolling speed (lower is faster)
 */
void scrollText(const char* message, uint8_t speed) {
    uint8_t length = strlen(message);
    
    // Need at least 4 characters to fill the display
    if (length < 4) return;
    
    // Create a temporary buffer to hold the 4 characters currently displayed
    char displayStr[5];
    
    for (uint8_t start = 0; start <= length - 4; start++) {
        // Copy the next 4 characters to display
        strncpy(displayStr, message + start, 4);
        displayStr[4] = '\0';
        
        // Update display and wait (still blocking for scrolling effect)
        writeString(displayStr);
        _delay_ms(speed * 50);  // Reduced delay for better responsiveness
    }
}

/**
 * Show a visual effect when changing tracks
 * 
 * @param trackNumber The new track number
 */
void trackChangeEffect(uint8_t trackNumber) {
    // Flash LEDs in a pattern
    allOff();
    
    // Sweep pattern
    for (uint8_t i = 0; i < 4; i++) {
        ledOn(i);
        _delay_ms(25);  // Reduced delay for better responsiveness
    }
    
    for (uint8_t i = 0; i < 4; i++) {
        ledOff(i);
        _delay_ms(25);  // Reduced delay for better responsiveness
    }
    
    // Display track number
    displayTrackNumber(trackNumber, 0);
}

/**
 * Show a startup animation
 */
void startupAnimation(void) {
    // Initialize random seed for better randomness
    srand(readADC(BPM_POT));
    
    // Display welcome message - now non-blocking for scrolling
    scrollText("DJ CONTROLLER READY", 2);
    
    // Light pattern
    allOff();
    for (int i = 0; i < 2; i++) {
        // Forward
        for (uint8_t led = 0; led < 4; led++) {
            allOff();
            ledOn(led);
            _delay_ms(50);  // Reduced delay for better responsiveness
        }
        
        // Backward
        for (uint8_t led = 3; led != 255; led--) {
            allOff();
            ledOn(led);
            _delay_ms(50);  // Reduced delay for better responsiveness
        }
    }
    
    // All LEDs on then off
    allOn();
    _delay_ms(150);  // Reduced delay for better responsiveness
    allOff();
}

/**
 * Show a beat indicator animation
 * This is called when a beat is detected in the music
 */
void beatAnimation(void) {
    ledPulseBeat(LED_STATUS);
}

/**
 * Toggle play/pause status and send appropriate command
 * This function handles the play/pause logic
 * 
 * @param isPlaying Pointer to the playing status variable
 * @return New playing status (1 = playing, 0 = paused)
 */
uint8_t togglePlayPause(volatile uint8_t* isPlaying) {
    // Toggle state
    *isPlaying = !(*isPlaying);
    
    // Send appropriate command
    if (*isPlaying) {
        transmitByte('P');  // Play command
        ledOn(LED_PLAYING);
    } else {
        transmitByte('S');  // Stop/Pause command
        ledOff(LED_PLAYING);
    }
    
    // Show PLAY or PAUS on display briefly - now non-blocking
    if (*isPlaying) {
        writeString("PLAY");
    } else {
        writeString("PAUS");
    }
    
    // Return new state
    return *isPlaying;
}

/**
 * Verify serial communication is working by sending a test signal
 * and checking for response
 * 
 * @return 1 if communication is working, 0 otherwise
 */
uint8_t verifySerialConnection(void) {
    uint8_t attempts = 3;
    
    while (attempts--) {
        // Send status request
        transmitByte('Q');
        
        // Wait for a response (with timeout)
        uint8_t timeout = 200;  // ~1 second timeout
        while (timeout-- && !USART_HAS_DATA) {
            _delay_ms(5);
        }
        
        // If we got a response, serial is working
        if (USART_HAS_DATA) {
            return 1;
        }
        
        // Small delay before trying again
        _delay_ms(200);
    }
    
    // No response after multiple attempts
    return 0;
}

/**
 * Send a debug message over serial
 * 
 * @param message The message to send
 */
void sendDebugMessage(const char* message) {
    transmitByte('D');  // Debug message indicator
    printString(message);
    transmitByte('\n');  // End with newline
}

/**
 * Show an error on the display
 * 
 * @param errorCode Error code to display
 */
void showError(uint8_t errorCode) {
    char errorStr[5] = "Er00";
    
    // Convert error code to ASCII digits
    errorStr[2] = '0' + (errorCode / 10) % 10;
    errorStr[3] = '0' + errorCode % 10;
    
    // Display error message (now non-blocking)
    writeString(errorStr);
    _delay_ms(500);
    writeString("    ");  // Clear display briefly
    _delay_ms(200);
    writeString(errorStr);
    _delay_ms(500);
    writeString("    ");
    _delay_ms(200);
    writeString(errorStr);
}

#define DEBUG_MODE 1  // Set to 1 to enable debug messages

/**
 * Send a debug message over serial
 * Only sends if DEBUG_MODE is enabled
 * 
 * @param message The debug message to send
 */
void debugMessage(const char* message) {
    #if DEBUG_MODE
    transmitByte('D');  // Debug message indicator
    printString(message);
    transmitByte('\n');
    #endif
}

/**
 * Format and display the time on the LED display
 * Updated to use non-blocking display functions
 * 
 * @param minutes Minutes to display (0-99)
 * @param seconds Seconds to display (0-59)
 * @param delay How long to display in milliseconds (no longer blocks)
 */
void displayTime(uint8_t minutes, uint8_t seconds, int delay) {
    // Make sure values are in valid ranges
    if (minutes > 99) minutes = 99;
    if (seconds > 59) seconds = 59;
    
    // Use writeTime with the second digit (index 1) as the decimal position
    writeTime(minutes, seconds, 0);  // No longer blocks
    
    // Debug the time we're displaying
    char debugBuffer[20];
    sprintf(debugBuffer, "Time: %02d.%02d", minutes, seconds);
    debugMessage(debugBuffer);
}

/**
 * Verify that a time format string from Java is valid
 * 
 * @param timeStr The time string (expected format: "MM:SS")
 * @param minutes Pointer to store parsed minutes
 * @param seconds Pointer to store parsed seconds
 * @return 1 if valid format, 0 otherwise
 */
uint8_t parseTimeString(char* timeStr, uint8_t* minutes, uint8_t* seconds) {
    // Check if string is at least 5 chars long (MM:SS)
    if (strlen(timeStr) < 5) return 0;
    
    // Check for colon separator
    if (timeStr[2] != ':') return 0;
    
    // Check that all other characters are digits
    if (!isdigit(timeStr[0]) || !isdigit(timeStr[1]) || 
        !isdigit(timeStr[3]) || !isdigit(timeStr[4])) return 0;
    
    // Parse minutes and seconds
    *minutes = (timeStr[0] - '0') * 10 + (timeStr[1] - '0');
    *seconds = (timeStr[3] - '0') * 10 + (timeStr[4] - '0');
    
    // Validate ranges
    if (*minutes > 99 || *seconds > 59) return 0;
    
    return 1;
}



/**
 * Manually test the display time functionality
 * This is useful for verifying that the display works correctly
 */
void testTimeDisplay() {
    // Test various times - now non-blocking
    displayTime(0, 0, 0);   // 00:00
    _delay_ms(1000);
    displayTime(1, 30, 0);  // 01:30
    _delay_ms(1000);
    displayTime(59, 59, 0); // 59:59
    _delay_ms(1000);
    displayTime(99, 59, 0); // 99:59 (max)
    _delay_ms(1000);
    
    // Test decimal point placement
    writeNumberWithDecimal(0130, 1); // Should show 01.30
    _delay_ms(1000);
}