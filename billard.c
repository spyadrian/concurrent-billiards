// Template of PCO TP: billard
// spring 2024

#include "billard.h"

#include <SDL2/SDL_mixer.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "calc_collision_speeds.h"
#include "display_and_sound.h"
#include "keyboard.h"
#include "tools.h"

extern Mix_Chunk *sound_chock;

typedef struct
{
    game_state_t *game_state;
    uint8_t id;
} param_t;

void my_calc_collision_speeds(ball_t *b1, ball_t *b2, pthread_mutex_t *m1, pthread_mutex_t *m2)
{
    coord_t dif_pos = (coord_t){b2->pos.x - b1->pos.x, b2->pos.y - b1->pos.y};
    coord_t dif_speed = (coord_t){b2->speed.x - b1->speed.x, b2->speed.y - b1->speed.y};
    float distance = sqrt(dif_pos.x * dif_pos.x + dif_pos.y * dif_pos.y);

    coord_t normal = (coord_t){dif_pos.x / distance, dif_pos.y / distance};
    float dot = dif_speed.x * normal.x + dif_speed.y * normal.y;

    pthread_mutex_lock(m1);
    b1->speed.x += dot * normal.x;
    b1->speed.y += dot * normal.y;

    if (fabs(b1->speed.x) > SPEED_MAX)
        b1->speed.x = SPEED_MAX * SIGN(b1->speed.x);

    if (fabs(b1->speed.y) > SPEED_MAX)
        b1->speed.y = SPEED_MAX * SIGN(b1->speed.y);

    pthread_mutex_unlock(m1);

    pthread_mutex_lock(m2);
    b2->speed.x -= dot * normal.x;
    b2->speed.y -= dot * normal.y;

    if (fabs(b2->speed.x) > SPEED_MAX)
        b2->speed.x = SPEED_MAX * SIGN(b2->speed.x);

    if (fabs(b2->speed.y) > SPEED_MAX)
        b2->speed.y = SPEED_MAX * SIGN(b2->speed.y);

    pthread_mutex_unlock(m2);
}

void init_balls_param(game_state_t *game_state)
{
    srand(time(NULL));

    for (uint8_t i = 0; i < NB_OF_BALLS; i++)
    {
        game_state->ball[i].pos = (coord_t){i * (WINDOW_WIDTH / NB_OF_BALLS), i * (WINDOW_HEIGHT / NB_OF_BALLS)};
        game_state->ball[i].radius = BALL_RADIUS;
        game_state->ball[i].speed = (coord_t){(rand() % (int)SPEED_MAX * 2) - SPEED_MAX, (rand() % (int)SPEED_MAX * 2) - SPEED_MAX};
        game_state->ball[i].state = ACTIVE;
        game_state->ball[i].timer = 0;
    }

    // custom blue ball
    game_state->ball[0].pos = (coord_t){1, 1};
    game_state->ball[0].speed = (coord_t){0, 0};
}

void *boule_func(void *args)
{
    // --- Initialize ---

    param_t *thread_param = (param_t *)args;
    uint8_t idx = thread_param->id;
    ball_t *ball = &(thread_param->game_state->ball[idx]);
    pthread_mutex_t *mutex = thread_param->game_state->mutex[idx];

    // --- Thread Routine ---

    while (thread_param->game_state->game_playing)
    {
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        if (ball->state >= STOPPED)  // if stopped or destroyed, block the ball
        {
            pthread_mutex_lock(thread_param->game_state->mutex_cond);
            pthread_cond_wait(thread_param->game_state->cond, thread_param->game_state->mutex_cond);
            pthread_mutex_unlock(thread_param->game_state->mutex_cond);
        }

        if (ball->state == ACTIVE)
        {
            // - test collision -
            int result = test_collision(idx, thread_param->game_state->ball, NB_OF_BALLS);

            // collision with a wall
            pthread_mutex_lock(mutex);
            switch (result)
            {
                case TOUCH_DOWN:
                    ball->speed.y = ball->speed.y * -1;
                    break;
                case TOUCH_UP:
                    ball->speed.y = ball->speed.y * -1;
                    break;
                case TOUCH_LEFT:
                    ball->speed.x = ball->speed.x * -1;
                    break;
                case TOUCH_RIGHT:
                    ball->speed.x = ball->speed.x * -1;
                    break;
            }
            pthread_mutex_unlock(mutex);

            // collision with another ball
            if (result >= 0)
            {
                // printf("touch ! : %d -> %d\n", idx, result);
                if (thread_param->game_state->ball[result].state == STOPPED)
                {
                    thread_param->game_state->ball[result].state = 0;  // destroy ball
                }
                else
                {
                    my_calc_collision_speeds(ball, &thread_param->game_state->ball[result], mutex, thread_param->game_state->mutex[result]);
                }

                Mix_PlayChannel(-1, sound_chock, 0);
            }

            pthread_mutex_lock(mutex);

            // - compute movement -

            // add friction
            ball->speed.x *= SPEED_FACTOR;
            ball->speed.y *= SPEED_FACTOR;

            // move ball
            ball->pos.x += ball->speed.x;
            ball->pos.y += ball->speed.y;

            // check for stopped ball
            if ((fabs(ball->speed.x) + fabs(ball->speed.y)) < SPEED_MIN)
            {
                ball->speed = (coord_t){0, 0};
                if (idx != 0)
                    ball->state = STOPPED;
            }

            pthread_mutex_unlock(mutex);
        }

        wait_until(current_time, BALL_DELAY);
    }

    return NULL;
}

int main()
{
    // --- Initialize ---

    game_state_t game_state = {0};
    game_state.game_playing = true;

    // - concurrent object -
    for (uint8_t i = 0; i < NB_OF_BALLS; i++)
    {
        game_state.mutex[i] = &(pthread_mutex_t){{__PTHREAD_MUTEX_INITIALIZER(PTHREAD_MUTEX_ERRORCHECK_NP)}};
    }

    game_state.mutex_cond = &(pthread_mutex_t){{__PTHREAD_MUTEX_INITIALIZER(PTHREAD_MUTEX_ERRORCHECK_NP)}};
    game_state.cond = &(pthread_cond_t)PTHREAD_COND_INITIALIZER;

    // - ball's param -
    init_balls_param(&game_state);

    // - threads -
    pthread_t t[NB_OF_BALLS + 2] = {0};

    param_t params[NB_OF_BALLS];
    for (uint8_t i = 0; i < NB_OF_BALLS; i++)
    {
        params[i].game_state = &game_state;
        params[i].id = i;
        pthread_create(&t[i], NULL, boule_func, &params[i]);
    }
    pthread_create(&t[NB_OF_BALLS], NULL, keyboard_func, &game_state);
    pthread_create(&t[NB_OF_BALLS + 1], NULL, display_func, &game_state);

    // --- Threads handling ---

    for (uint8_t i = 0; i < NB_OF_BALLS + 2; i++)
    {
        pthread_join(t[i], NULL);
    }

    // --- Destroy ---

    for (uint8_t i = 0; i < NB_OF_BALLS; i++)
    {
        pthread_mutex_destroy(game_state.mutex[i]);
    }

    pthread_mutex_destroy(game_state.mutex_cond);
    pthread_cond_destroy(game_state.cond);

    return 0;
}
