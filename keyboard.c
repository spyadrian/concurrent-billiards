#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "billard.h"

void wait_key_release()
{
    SDL_Event event;
    while (1)
    {
        if (SDL_PollEvent(&event))
            if (event.type == SDL_KEYUP)
                break;
    }
}

void *keyboard_func(void *param)
{
    // --- Initialise ---

    game_state_t *game_state = param;
    ball_t *blue_ball = &(game_state->ball[0]);

    SDL_Event event;
    int nb_of_keys;
    uint8_t *keys;

    // --- Key press ---

    do
    {
        SDL_WaitEvent(&event);  // passive waiting
        switch (event.type)
        {
            case SDL_QUIT:

                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                }
                break;
            case SDL_KEYDOWN:
                keys = (uint8_t *)SDL_GetKeyboardState(&nb_of_keys);
                if (keys[SDL_SCANCODE_ESCAPE])
                {
                    game_state->game_playing = false;
                    pthread_cond_broadcast(game_state->cond);
                }

                pthread_mutex_lock(game_state->mutex[0]);

                if (keys[SDL_SCANCODE_RIGHT])
                {
                    blue_ball->speed.x += BALL0_ACCEL;
                }
                if (keys[SDL_SCANCODE_LEFT])
                {
                    blue_ball->speed.x -= BALL0_ACCEL;
                }
                if (keys[SDL_SCANCODE_UP])
                {
                    blue_ball->speed.y -= BALL0_ACCEL;
                }
                if (keys[SDL_SCANCODE_DOWN])
                {
                    blue_ball->speed.y += BALL0_ACCEL;
                }

                if (fabs(blue_ball->speed.x) > SPEED_MAX)
                    blue_ball->speed.x = SPEED_MAX * SIGN(blue_ball->speed.x);

                if (fabs(blue_ball->speed.y) > SPEED_MAX)
                    blue_ball->speed.y = SPEED_MAX * SIGN(blue_ball->speed.y);

                pthread_mutex_unlock(game_state->mutex[0]);

                if (keys[SDL_SCANCODE_S])
                {
                    for (uint8_t i = 0; i < NB_OF_BALLS; i++)
                        pthread_mutex_lock(game_state->mutex[i]);

                    init_balls_param(game_state);

                    for (uint8_t i = 0; i < NB_OF_BALLS; i++)
                        pthread_mutex_unlock(game_state->mutex[i]);

                    pthread_cond_broadcast(game_state->cond);
                }
        }  // switch
    } while (game_state->game_playing);

    return NULL;
}
