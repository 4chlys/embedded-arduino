#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "leds.h"
#include "display.h"
#include "buttons.h"
#include "usart.h"
#include "potentiometer.h"
#include "sound.h"

#define DEFAULT_START_AMOUNT 21
#define DEFAULT_MAX_TAKE 3
#define MIN_START_AMOUNT 21
#define MAX_START_AMOUNT 99
#define MIN_MAX_TAKE 3
#define MAX_MAX_TAKE 9
#define MAX_MOVES 50

#define TAKE_DIGIT 0
#define TURN_DIGIT 1
#define TENS_DIGIT 2
#define ONES_DIGIT 3

#define PLAYER 0
#define COMPUTER 1

#define BUTTON_DEBOUNCE_MS 200
#define FLASH_ON_MS 200
#define FLASH_OFF_MS 100
#define CONFIG_DISPLAY_MS 500
#define SEED_DISPLAY_MS 500

typedef struct
{
    uint8_t current_player;
    uint8_t sticks_remaining;
    uint8_t take_amount;
    uint8_t max_take;
    uint8_t game_over;
    uint8_t winner;
    uint8_t start_amount;
} GameState;

typedef struct
{
    uint8_t player;
    uint8_t amount_taken;
    uint8_t remaining_after;
} Move;

void init_game(GameState *game, uint8_t start_amount, uint8_t max_take);
void display_game_state(GameState *game);
void process_player_turn(GameState *game, Move *history, uint8_t *move_count);
void process_computer_turn(GameState *game, Move *history, uint8_t *move_count);
void flash_turn_indicator(GameState *game);
void display_winner(uint8_t winner);
void display_seed(uint16_t seed);
void play_game(GameState *game, Move *history, uint8_t *move_count);
uint8_t calculate_computer_move(GameState *game);
uint8_t read_potentiometer_range(uint8_t min_val, uint8_t max_val, const char *label);
uint8_t configure_game_parameters(uint8_t *start_amount, uint8_t *max_take);
void print_game_history(Move *history, uint8_t move_count, GameState *game);
void record_move(Move *history, uint8_t *move_count, uint8_t player, uint8_t amount, uint8_t remaining);
void transmit_string(const char *str);
void show_config_display(const char *label, uint8_t value);
void play_victory_sound(uint8_t winner);
void play_game_over_sequence(GameState *game);

int main(void)
{
    leds_init();
    display_init();
    buttons_init();
    usart_init();
    potentiometer_init();
    buzzer_init();

    uint16_t seed = 0;
    uint8_t start_amount = DEFAULT_START_AMOUNT;
    uint8_t max_take = DEFAULT_MAX_TAKE;

    display_string("NIM ");
    for (uint16_t i = 0; i < SEED_DISPLAY_MS; i++)
    {
        display_update(1);
        _delay_ms(1);
    }

    uint8_t button_pressed = 0;
    transmit_string("NIM Game Started!\r\n");
    transmit_string("Turn potentiometer to generate seed, then press button 1 to start\r\n");

    while (!button_pressed)
    {
        uint16_t pot_value = read_adc(POT_PIN);
        seed = pot_value % 10000;

        display_seed(seed);

        if (read_button(BUTTON_PLAY_PIN) == 0)
        {
            button_pressed = 1;
        }
        display_update(1);
    }

    _delay_ms(BUTTON_DEBOUNCE_MS);
    srand(seed);

    transmit_string("Configuring game parameters...\r\n");
    if (!configure_game_parameters(&start_amount, &max_take))
    {
        transmit_string("Configuration failed!\r\n");
        while (1)
            ;
    }

    GameState game;

    Move *history = (Move *)calloc(MAX_MOVES, sizeof(Move));
    if (history == NULL)
    {
        transmit_string("Memory allocation failed!\r\n");
        while (1)
            ;
    }

    uint8_t move_count = 0;

    init_game(&game, start_amount, max_take);

    char config_msg[100];
    sprintf(config_msg, "Game configured: %d sticks, max take %d, seed %d\r\n",
            start_amount, max_take, seed);
    transmit_string(config_msg);

    play_game(&game, history, &move_count);

    play_game_over_sequence(&game);
    print_game_history(history, move_count, &game);

    free(history);

    while (1)
    {
        display_update(1);
    }

    return 0;
}

/**
 * Configure game parameters using potentiometer
 */
uint8_t configure_game_parameters(uint8_t *start_amount, uint8_t *max_take)
{
    if (start_amount == NULL || max_take == NULL)
    {
        return 0;
    }

    *start_amount = read_potentiometer_range(MIN_START_AMOUNT, MAX_START_AMOUNT, "START");

    uint8_t max_possible_take = (*start_amount) / 5;
    if (max_possible_take < MIN_MAX_TAKE)
    {
        max_possible_take = MIN_MAX_TAKE;
    }
    if (max_possible_take > MAX_MAX_TAKE)
    {
        max_possible_take = MAX_MAX_TAKE;
    }

    *max_take = read_potentiometer_range(MIN_MAX_TAKE, max_possible_take, "MAX");

    return 1;
}

/**
 * Initialize the game state
 */
void init_game(GameState *game, uint8_t start_amount, uint8_t max_take)
{
    game->sticks_remaining = start_amount;
    game->start_amount = start_amount;
    game->max_take = max_take;
    game->take_amount = 1;
    game->game_over = 0;
    game->winner = 0;

    game->current_player = rand() % 2;

    display_game_state(game);
}

/**
 * Display the current game state on the 4-digit display
 */
void display_game_state(GameState *game)
{
    char display_buffer[5] = "    ";

    if (game->current_player == PLAYER && !game->game_over)
    {
        display_buffer[TAKE_DIGIT] = '0' + game->take_amount;
    }
    else
    {
        display_buffer[TAKE_DIGIT] = ' ';
    }

    if (!game->game_over)
    {
        display_buffer[TURN_DIGIT] = (game->current_player == PLAYER) ? 'P' : 'C';
    }
    else
    {
        display_buffer[TURN_DIGIT] = ' ';
    }

    uint8_t tens = game->sticks_remaining / 10;
    uint8_t ones = game->sticks_remaining % 10;

    display_buffer[TENS_DIGIT] = (tens > 0) ? ('0' + tens) : ' ';
    display_buffer[ONES_DIGIT] = '0' + ones;

    display_string(display_buffer);
}

/**
 * Flash the turn indicator to show whose turn it is - interruptible by button press
 */
void flash_turn_indicator(GameState *game)
{
    uint8_t flash_count = 0;
    uint8_t flash_state = 1;
    uint16_t timer = 0;
    uint8_t interrupted = 0;

    while (flash_count < 5 && !interrupted)
    {
        if (game->current_player == PLAYER)
        {
            if (read_button(BUTTON_PLAY_PIN) == 0 ||
                read_button(BUTTON_NEXT_PIN) == 0 ||
                read_button(BUTTON_PREV_PIN) == 0)
            {
                interrupted = 1;
            }
        }
        else
        {
            if (read_button(BUTTON_NEXT_PIN) == 0)
            {
                interrupted = 1;
            }
        }

        if (flash_state)
        {
            display_game_state(game);
            if (timer >= FLASH_ON_MS)
            {
                flash_state = 0;
                timer = 0;
            }
        }
        else
        {
            char display_buffer[5] = "    ";

            if (game->current_player == PLAYER)
            {
                display_buffer[TAKE_DIGIT] = '0' + game->take_amount;
            }

            uint8_t tens = game->sticks_remaining / 10;
            uint8_t ones = game->sticks_remaining % 10;

            display_buffer[TENS_DIGIT] = (tens > 0) ? ('0' + tens) : ' ';
            display_buffer[ONES_DIGIT] = '0' + ones;

            display_string(display_buffer);

            if (timer >= FLASH_OFF_MS)
            {
                flash_state = 1;
                timer = 0;
                flash_count++;
            }
        }

        display_update(1);
        _delay_ms(1);
        timer++;
    }

    display_game_state(game);
}

/**
 * Process player's turn
 */
void process_player_turn(GameState *game, Move *history, uint8_t *move_count)
{
    uint8_t turn_complete = 0;

    flash_turn_indicator(game);

    while (!turn_complete)
    {
        if (read_button(BUTTON_PREV_PIN) == 0)
        {
            if (game->take_amount < game->max_take && game->take_amount < game->sticks_remaining)
            {
                game->take_amount++;
                display_game_state(game);
                _delay_ms(BUTTON_DEBOUNCE_MS);
            }
        }

        if (read_button(BUTTON_PLAY_PIN) == 0)
        {
            if (game->take_amount > 1)
            {
                game->take_amount--;
                display_game_state(game);
                _delay_ms(BUTTON_DEBOUNCE_MS);
            }
        }

        if (read_button(BUTTON_NEXT_PIN) == 0)
        {
            if (game->take_amount <= game->sticks_remaining)
            {
                game->sticks_remaining -= game->take_amount;

                record_move(history, move_count, PLAYER, game->take_amount, game->sticks_remaining);

                playTone(C5, 100);

                if (game->sticks_remaining == 0)
                {
                    game->game_over = 1;
                    game->winner = COMPUTER;
                }
                else
                {
                    game->current_player = COMPUTER;
                }

                turn_complete = 1;
                _delay_ms(BUTTON_DEBOUNCE_MS);
            }
        }

        display_update(1);
    }

    display_game_state(game);
}

/**
 * Process computer's turn
 */
void process_computer_turn(GameState *game, Move *history, uint8_t *move_count)
{
    uint8_t turn_complete = 0;

    flash_turn_indicator(game);

    while (!turn_complete)
    {
        if (read_button(BUTTON_NEXT_PIN) == 0)
        {
            game->take_amount = calculate_computer_move(game);

            char display_buffer[5] = "    ";
            display_buffer[TAKE_DIGIT] = '0' + game->take_amount;
            display_buffer[TURN_DIGIT] = 'C';

            uint8_t tens = game->sticks_remaining / 10;
            uint8_t ones = game->sticks_remaining % 10;

            display_buffer[TENS_DIGIT] = (tens > 0) ? ('0' + tens) : ' ';
            display_buffer[ONES_DIGIT] = '0' + ones;

            display_string(display_buffer);

            uint8_t confirmed = 0;
            while (!confirmed)
            {
                if (read_button(BUTTON_NEXT_PIN) == 0)
                {
                    confirmed = 1;
                    _delay_ms(BUTTON_DEBOUNCE_MS);
                }
                display_update(1);
            }

            game->sticks_remaining -= game->take_amount;

            record_move(history, move_count, COMPUTER, game->take_amount, game->sticks_remaining);

            playTone(G5, 100);

            if (game->sticks_remaining == 0)
            {
                game->game_over = 1;
                game->winner = PLAYER;
            }
            else
            {
                game->current_player = PLAYER;
                game->take_amount = 1;
            }

            turn_complete = 1;
        }

        display_update(1);
    }

    display_game_state(game);
}

/**
 * Calculate computer's optimal move
 */
uint8_t calculate_computer_move(GameState *game)
{
    uint8_t remainder = game->sticks_remaining % (game->max_take + 1);
    uint8_t optimal_move = (remainder == 0) ? 0 : remainder;

    if (optimal_move == 0)
    {
        uint8_t random_move = (rand() % game->max_take) + 1;

        if (random_move > game->sticks_remaining)
        {
            random_move = game->sticks_remaining;
        }

        return random_move;
    }

    if (optimal_move >= game->sticks_remaining)
    {
        optimal_move = game->sticks_remaining - 1;
        if (optimal_move == 0)
        {
            optimal_move = 1;
        }
    }

    return optimal_move;
}

/**
 * Record a move in the game history
 */
void record_move(Move *history, uint8_t *move_count, uint8_t player, uint8_t amount, uint8_t remaining)
{
    if (history != NULL && move_count != NULL && *move_count < MAX_MOVES)
    {
        history[*move_count].player = player;
        history[*move_count].amount_taken = amount;
        history[*move_count].remaining_after = remaining;
        (*move_count)++;
    }
}

/**
 * Display the seed value
 */
void display_seed(uint16_t seed)
{
    char display_buffer[5];
    sprintf(display_buffer, "%04d", seed);
    display_string(display_buffer);
}

/**
 * Display the winner
 */
void display_winner(uint8_t winner)
{
    if (winner == PLAYER)
    {
        display_string("PWIN");
    }
    else
    {
        display_string("CWIN");
    }
}

/**
 * Play victory sound based on winner
 */
void play_victory_sound(uint8_t winner)
{
    if (winner == PLAYER)
    {
        playTone(C5, 200);
        playTone(E5, 200);
        playTone(G5, 200);
        playTone(C6, 400);
    }
    else
    {
        playTone(C6, 200);
        playTone(G5, 200);
        playTone(E5, 200);
        playTone(C5, 400);
    }
}

/**
 * Play the game over sequence
 */
void play_game_over_sequence(GameState *game)
{
    play_victory_sound(game->winner);

    for (uint8_t i = 0; i < 5; i++)
    {
        display_winner(game->winner);
        for (uint16_t j = 0; j < FLASH_ON_MS; j++)
        {
            display_update(1);
            _delay_ms(1);
        }

        display_string("    ");
        for (uint16_t j = 0; j < FLASH_OFF_MS; j++)
        {
            display_update(1);
            _delay_ms(1);
        }
    }

    display_winner(game->winner);
}

/**
 * Main game loop
 */
void play_game(GameState *game, Move *history, uint8_t *move_count)
{
    while (!game->game_over)
    {
        if (game->current_player == PLAYER)
        {
            process_player_turn(game, history, move_count);
        }
        else
        {
            process_computer_turn(game, history, move_count);
        }
    }
}

/**
 * Print complete game history to serial monitor
 */
void print_game_history(Move *history, uint8_t move_count, GameState *game)
{
    transmit_string("\r\n=== GAME HISTORY ===\r\n");

    char buffer[100];
    sprintf(buffer, "Game setup: %d sticks, max take %d\r\n",
            game->start_amount, game->max_take);
    transmit_string(buffer);

    transmit_string("Move history:\r\n");

    for (uint8_t i = 0; i < move_count; i++)
    {
        sprintf(buffer, "Turn %2d: %s took %d stick%s, %d remaining\r\n",
                i + 1,
                (history[i].player == PLAYER) ? "Player  " : "Computer",
                history[i].amount_taken,
                (history[i].amount_taken == 1) ? " " : "s",
                history[i].remaining_after);
        transmit_string(buffer);
    }

    sprintf(buffer, "\r\nGame Over! Winner: %s\r\n",
            (game->winner == PLAYER) ? "Player" : "Computer");
    transmit_string(buffer);

    transmit_string("==================\r\n");
}

/**
 * Read potentiometer and map to range with visual feedback
 */
uint8_t read_potentiometer_range(uint8_t min_val, uint8_t max_val, const char *label)
{
    uint16_t pot_value = read_adc(POT_PIN);
    uint8_t result = (uint8_t)map_value(pot_value, 0, 1023, min_val, max_val);

    show_config_display(label, result);

    return result;
}

/**
 * Show configuration value on display
 */
void show_config_display(const char *label, uint8_t value)
{
    char display_buffer[5];
    sprintf(display_buffer, " %3d", value);
    display_string(display_buffer);

    char serial_buffer[50];
    sprintf(serial_buffer, "%s amount: %d\r\n", label, value);
    transmit_string(serial_buffer);

    for (uint16_t i = 0; i < CONFIG_DISPLAY_MS; i++)
    {
        display_update(1);
        _delay_ms(1);
    }
}
