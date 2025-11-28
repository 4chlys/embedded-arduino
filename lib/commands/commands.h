/**
 * Command Processing Library for DJ Controller
 * 
 * Provides functions for handling commands and communication with Java application
 */

#ifndef COMMANDS_H
#define COMMANDS_H

#include <avr/io.h>
#include <stdio.h>

typedef struct Playlist Playlist;

#define CMD_PLAY 'P'
#define CMD_PAUSE 'S' 
#define CMD_NEXT_TRACK 'N'
#define CMD_PREV_TRACK 'B'
#define CMD_SEEK_FORWARD 'F'
#define CMD_SEEK_BACKWARD 'R'
#define CMD_STATUS_REQUEST 'Q'
#define CMD_TRACK_COUNT_INC 'T' 
#define CMD_TRACK_COUNT_DEC 'D' 
#define CMD_CURRENT_TRACK_INC 'C'
#define CMD_CURRENT_TRACK_DEC 'V'
#define CMD_BEAT_DETECTED 'b'

#define CMD_REQUEST_PLAY 'P'
#define CMD_REQUEST_PAUSE 'S'
#define CMD_REQUEST_NEXT 'N'
#define CMD_REQUEST_PREV 'B'
#define CMD_REQUEST_SEEK_FWD 'F'
#define CMD_REQUEST_SEEK_BWD 'R'
#define CMD_REQUEST_STATUS 'Q'

extern uint8_t isPlaying;
extern uint8_t currentTrack;
extern uint8_t totalTracks;

extern uint8_t isPlaying;
extern uint8_t currentTrack;
extern uint8_t totalTracks;

/**
 * Send a single command byte to Java
 * @param cmd The command byte to send
 */
void send_command(uint8_t cmd);

/**
 * Process serial data received from Java
 */
void process_serial(void);
/**
 * Initialize command state
 */
void commands_init(void);

/**
 * Set the active playlist (by-reference parameter example)
 * @param playlist_ptr Pointer to a playlist pointer
 */
void commands_set_playlist(Playlist** playlist_ptr);

#endif