#include <stdint.h>
#include <avr/io.h>

// Display utility functions
void displayTrackNumber(uint8_t trackNumber, uint16_t duration);
void scrollText(const char* message, uint8_t speed);
void trackChangeEffect(uint8_t trackNumber);
void startupAnimation(void);
void beatAnimation(void);

// Communication utility functions
uint8_t togglePlayPause(volatile uint8_t* isPlaying);
uint8_t verifySerialConnection(void);
void sendDebugMessage(const char* message);
void showError(uint8_t errorCode);

void debugMessage(const char* message);
void displayTime(uint8_t minutes, uint8_t seconds, int delay);
uint8_t parseTimeString(char* timeStr, uint8_t* minutes, uint8_t* seconds);
void testTimeDisplay(void);