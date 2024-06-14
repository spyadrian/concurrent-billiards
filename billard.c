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
    // --- initialize ---
    param_t *thread_param = (param_t *)args;
    uint8_t idx = thread_param->id;
    ball_t *my_ball = &(thread_param->game_state->ball[idx]);

    // --- Thread Routine ---
    while (thread_param->game_state->game_playing)
    {
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        if (my_ball->state <= STOPPED)  // is not destroy
        {
            int result = test_collision(idx, thread_param->game_state->ball, NB_OF_BALLS);
            switch (result)
            {
                case TOUCH_DOWN:
                    my_ball->speed.y = my_ball->speed.y * -1;
                    break;
                case TOUCH_UP:
                    my_ball->speed.y = my_ball->speed.y * -1;
                    break;
                case TOUCH_LEFT:
                    my_ball->speed.x = my_ball->speed.x * -1;
                    break;
                case TOUCH_RIGHT:
                    my_ball->speed.x = my_ball->speed.x * -1;
                    break;
            }
            // collision with another ball
            if (result >= 0)
            {
                // printf("touch ! : %d -> %d\n", idx, result);
                if (idx == 0 && thread_param->game_state->ball[result].state == STOPPED)
                {
                    // printf("ball %d destroyed\n", result);
                    thread_param->game_state->ball[result].state = 0;  // destroy ball
                }
                else if (result == 0 && thread_param->game_state->ball[idx].state == STOPPED)
                {
                    // printf("ball %d destroyed\n", idx);
                    thread_param->game_state->ball[idx].state = 0;  // destroy ball
                }
                else
                {
                    calc_collision_speeds(my_ball, &thread_param->game_state->ball[result]);
                }

                Mix_PlayChannel(-1, sound_chock, 0);
            }

            if (my_ball->state <= STOPPED)  // is not destroy
            {
                // ralentissement obligatoire du au frottement
                my_ball->speed.x *= SPEED_FACTOR;
                my_ball->speed.y *= SPEED_FACTOR;
                my_ball->pos.x += my_ball->speed.x;
                my_ball->pos.y += my_ball->speed.y;

                // calcule de la vitesse pour l'arret
                if ((fabs(my_ball->speed.x) + fabs(my_ball->speed.y)) < SPEED_MIN)
                {
                    my_ball->speed = (coord_t){0, 0};
                    if (idx != 0)
                        my_ball->state = STOPPED;
                }
                else
                {
                    my_ball->state = ACTIVE;
                }
            }
        }

        wait_until(current_time, BALL_DELAY);
    }

    return NULL;
}

int main()
{
    game_state_t game_state = {0};

    init_balls_param(&game_state);
    game_state.game_playing = true;

    // mutex for indexes counter
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK_NP);

    game_state.mutex = calloc(1, sizeof(pthread_mutex_t));
    pthread_mutex_init(game_state.mutex, &mutex_attr);
    pthread_mutexattr_destroy(&mutex_attr);

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

    // --- Threads Handling ---

    for (uint8_t i = 0; i < NB_OF_BALLS + 2; i++)
    {
        pthread_join(t[i], NULL);
    }

    free(game_state.mutex);

    return 0;
}
