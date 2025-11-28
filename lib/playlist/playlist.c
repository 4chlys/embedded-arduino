/**
 * Playlist Management Implementation for DJ Controller
 */

#include "playlist.h"
#include <stdio.h>

Playlist* playlist_create(uint8_t capacity) {
    Playlist* playlist = (Playlist*)malloc(sizeof(Playlist));
    
    if (playlist == NULL) {
        return NULL;
    }
   
    playlist->tracks = (Track*)malloc(sizeof(Track) * capacity);
    
    if (playlist->tracks == NULL) {
        free(playlist);
        return NULL;
    }

    playlist->capacity = capacity;
    playlist->count = 0;
    playlist->current_index = 0;
    playlist->display_needs_update = 0;
    
    return playlist;
}

void playlist_destroy(Playlist* playlist) {
    if (playlist != NULL) {
        if (playlist->tracks != NULL) {
            free(playlist->tracks);
        }
        free(playlist);
    }
}

uint8_t playlist_add_track(Playlist* playlist, Track* track) {
    if (playlist == NULL || track == NULL) {
        return 0; 
    }
    
    if (playlist->count >= playlist->capacity) {
        return 0;
    }
    
    memcpy(&(playlist->tracks[playlist->count]), track, sizeof(Track));
   
    playlist->count++;
    
    return 1; 
}

Track* playlist_get_track(Playlist* playlist, uint8_t index) {
    if (playlist == NULL || index >= playlist->count) {
        return NULL;
    }
    
    return &(playlist->tracks[index]);
}

Track* playlist_get_current_track(Playlist* playlist) {
    if (playlist == NULL || playlist->count == 0) {
        return NULL; 
    }
    
    return &(playlist->tracks[playlist->current_index]);
}

uint8_t playlist_set_current_track(Playlist* playlist, uint8_t index) {
    if (playlist == NULL || index >= playlist->count) {
        return 0; 
    }
    
    playlist->current_index = index;
    playlist->display_needs_update = 1;
    
    return 1;
}

uint8_t playlist_next_track(Playlist* playlist) {
    if (playlist == NULL || playlist->count <= 1) {
        return 0; 
    }

    playlist->current_index = (playlist->current_index + 1) % playlist->count;
    playlist->display_needs_update = 1;
    
    return 1;
}

uint8_t playlist_prev_track(Playlist* playlist) {
    if (playlist == NULL || playlist->count <= 1) {
        return 0;
    }
    
    if (playlist->current_index == 0) {
        playlist->current_index = playlist->count - 1;
    } else {
        playlist->current_index--;
    }
    
    playlist->display_needs_update = 1;
    
    return 1;
}

void playlist_set_playing(Playlist* playlist, uint8_t isPlaying) {
    if (playlist == NULL || playlist->count == 0) {
        return; 
    }
    
    Track* current = &(playlist->tracks[playlist->current_index]);
    current->isPlaying = isPlaying;
    playlist->display_needs_update = 1;
}

void playlist_check_update(Playlist* playlist) {
    if (playlist == NULL || !playlist->display_needs_update) {
        return;
    }
    
    playlist->display_needs_update = 0;
    
    if (playlist->count == 0) {
        return;
    }
   
    Track* current = &(playlist->tracks[playlist->current_index]);
   
    char trackDisplay[5];
    sprintf(trackDisplay, "TR%02d", current->number);
 
    display_message(trackDisplay, 200);
}