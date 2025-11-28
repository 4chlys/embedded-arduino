/**
 * Playlist Management Library for DJ Controller
 * 
 * Provides structs and functions for managing a playlist of tracks
 * Demonstrates struct usage and dynamic memory allocation
 */

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <avr/io.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration to avoid circular dependency errors
void display_message(const char* str, uint16_t display_time);

typedef struct {
    char name[5];          
    uint8_t number;        
    uint16_t duration_sec; 
    uint8_t isPlaying;     
} Track;


typedef struct Playlist {
    Track* tracks;         
    uint8_t capacity;    
    uint8_t count;       
    uint8_t current_index;
    uint8_t display_needs_update; 
} Playlist;

/**
 * Create a new playlist with specified capacity
 * @param capacity Maximum number of tracks
 * @return Pointer to the created playlist or NULL on failure
 */
Playlist* playlist_create(uint8_t capacity);

/**
 * Free memory used by a playlist
 * @param playlist Pointer to the playlist
 */
void playlist_destroy(Playlist* playlist);

/**
 * Add a track to the playlist
 * @param playlist Pointer to the playlist
 * @param track Pointer to the track to add
 * @return 1 if successful, 0 if playlist is full
 */
uint8_t playlist_add_track(Playlist* playlist, Track* track);

/**
 * Get a track by index
 * @param playlist Pointer to the playlist
 * @param index Index of the track to get
 * @return Pointer to the track or NULL if index is invalid
 */
Track* playlist_get_track(Playlist* playlist, uint8_t index);

/**
 * Get the current track
 * @param playlist Pointer to the playlist
 * @return Pointer to the current track or NULL if no tracks
 */
Track* playlist_get_current_track(Playlist* playlist);

/**
 * Set the current track by index
 * @param playlist Pointer to the playlist
 * @param index Index of the track to set as current
 * @return 1 if successful, 0 if index is invalid
 */
uint8_t playlist_set_current_track(Playlist* playlist, uint8_t index);

/**
 * Move to next track
 * @param playlist Pointer to the playlist
 * @return 1 if successful, 0 if no next track
 */
uint8_t playlist_next_track(Playlist* playlist);

/**
 * Move to previous track
 * @param playlist Pointer to the playlist
 * @return 1 if successful, 0 if no previous track
 */
uint8_t playlist_prev_track(Playlist* playlist);

/**
 * Set play status of current track
 * @param playlist Pointer to the playlist
 * @param isPlaying Play status to set
 */
void playlist_set_playing(Playlist* playlist, uint8_t isPlaying);

/**
 * Check if display needs updating and handle it
 * @param playlist Pointer to the playlist
 */
void playlist_check_update(Playlist* playlist);

#endif