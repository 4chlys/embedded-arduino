/**
 * Command Processing Implementation for DJ Controller
 */

#include "commands.h"
#include "usart.h"
#include "display.h"
#include "leds.h"
#include "playlist.h"
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

uint8_t isPlaying = 0;
uint8_t currentTrack = 1;
uint8_t totalTracks = 1;

static Playlist *active_playlist = NULL;

void commands_init(void)
{
    isPlaying = 0;
    currentTrack = 1;
    totalTracks = 1;
}

void commands_set_playlist(Playlist **playlist_ptr)
{
    if (playlist_ptr != NULL)
    {
        active_playlist = *playlist_ptr;
    }
}

void reset_track_counters(void)
{
    currentTrack = 1;
    totalTracks = 1;

    if (active_playlist != NULL)
    {
        active_playlist->count = 0;
        active_playlist->current_index = 0;
    }
}

void update_playlist_tracks(void)
{
    if (active_playlist == NULL)
    {
        return;
    }

    active_playlist->count = 0;

    for (uint8_t i = 1; i <= totalTracks; i++)
    {
        Track new_track;
        sprintf(new_track.name, "TR%02d", i);
        new_track.number = i;
        new_track.duration_sec = 180;
        new_track.isPlaying = (isPlaying && (i == currentTrack));

        if (active_playlist->count < active_playlist->capacity)
        {
            memcpy(&(active_playlist->tracks[active_playlist->count]), &new_track, sizeof(Track));
            active_playlist->count++;
        }
    }

    if (currentTrack <= totalTracks && currentTrack > 0)
    {
        active_playlist->current_index = currentTrack - 1;
        playlist_set_current_track(active_playlist, currentTrack - 1);
    }
}

void send_command(uint8_t cmd)
{
    transmit_byte(cmd);

    char cmdStr[5] = "S ";
    cmdStr[1] = cmd;
    cmdStr[2] = '\0';
    display_message(cmdStr, 50);
}

void process_serial(void)
{
    if (is_data_available())
    {
        char cmd = receive_byte();

        switch (cmd)
        {
        case CMD_PLAY:
            isPlaying = 1;
            
            if (active_playlist != NULL) {
                playlist_set_playing(active_playlist, 1);
            }
            
            led_on(LED_PLAY_PIN);
            display_string("PLAY");
            break;

        case CMD_PAUSE:
            isPlaying = 0;
            
            if (active_playlist != NULL) {
                playlist_set_playing(active_playlist, 0);
            }
            
            led_off(LED_PLAY_PIN);
            display_string("PAUS");
            break;

        case CMD_TRACK_COUNT_INC:
            {
                totalTracks++;
                if (totalTracks > 99) totalTracks = 99;
                update_playlist_tracks();
                
                char total_msg[5];
                sprintf(total_msg, "T%02d", totalTracks);
                display_message(total_msg, 200);
            }
            break;

        case CMD_TRACK_COUNT_DEC:
        {
            if (totalTracks > 1)
            {
                totalTracks--;
                if (currentTrack > totalTracks)
                {
                    currentTrack = totalTracks;
                }
                update_playlist_tracks();
            }

            char total_dec_msg[5];
            sprintf(total_dec_msg, "T%02d", totalTracks);
            display_message(total_dec_msg, 200);
        }
        break;

        case CMD_CURRENT_TRACK_INC:
        {
            if (currentTrack < totalTracks)
            {
                currentTrack++;
            }
            else
            {
                currentTrack = 1;
            }
            update_playlist_tracks();

            char curr_inc_msg[5];
            sprintf(curr_inc_msg, "TR%02d", currentTrack);
            display_message(curr_inc_msg, 300);
        }
        break;

        case CMD_CURRENT_TRACK_DEC:
        {
            if (currentTrack > 1)
            {
                currentTrack--;
            }
            else
            {
                currentTrack = totalTracks;
            }
            update_playlist_tracks();

            char curr_dec_msg[5];
            sprintf(curr_dec_msg, "TR%02d", currentTrack);
            display_message(curr_dec_msg, 300);
        }
        break;

        case CMD_NEXT_TRACK:
        {
            if (currentTrack < totalTracks)
            {
                currentTrack++;
            }
            else
            {
                currentTrack = 1;
            }
            update_playlist_tracks();

            char next_msg[5];
            sprintf(next_msg, "TR%02d", currentTrack);
            display_message(next_msg, 300);

            _delay_ms(100);
            send_command(CMD_REQUEST_STATUS);
        }
        break;

        case CMD_PREV_TRACK:
        {
            if (currentTrack > 1)
            {
                currentTrack--;
            }
            else
            {
                currentTrack = totalTracks;
            }
            update_playlist_tracks();

            char prev_msg[5];
            sprintf(prev_msg, "TR%02d", currentTrack);
            display_message(prev_msg, 300);

            _delay_ms(100);
            send_command(CMD_REQUEST_STATUS);
        }
        break;

        case CMD_BEAT_DETECTED:
            led_on(LED_STATUS_PIN);
            for (uint8_t i = 0; i < 50; i++)
            {
                display_update(1);
            }
            led_off(LED_STATUS_PIN);
            break;

        case CMD_SEEK_FORWARD:
            display_message("FF", 100);
            break;

        case CMD_SEEK_BACKWARD:
            display_message("RW", 100);
            break;

        case CMD_STATUS_REQUEST:
        {
            char status_msg[5];
            sprintf(status_msg, "TR%02d", currentTrack);
            display_message(status_msg, 200);
        }
        break;

        default:
        {
            char unknownStr[5] = "? ";
            unknownStr[1] = cmd;
            display_message(unknownStr, 100);
        }
        break;
        }
    }
}