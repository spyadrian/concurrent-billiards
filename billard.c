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
    pthread_mutex_unlock(m1);

    pthread_mutex_lock(m2);
    b2->speed.x -= dot * normal.x;
    b2->speed.y -= dot * normal.y;
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
    // --- initialize ---
    param_t *thread_param = (param_t *)args;
    uint8_t idx = thread_param->id;
    ball_t *my_ball = &(thread_param->game_state->ball[idx]);
    pthread_mutex_t *my_mutex = thread_param->game_state->mutex[idx];

    // --- Thread Routine ---
    while (thread_param->game_state->game_playing)
    {
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        if (my_ball->state >= STOPPED)
        {
            pthread_mutex_lock(thread_param->game_state->mutex_cond);
            pthread_cond_wait(&(thread_param->game_state->cond), thread_param->game_state->mutex_cond);
            pthread_mutex_unlock(thread_param->game_state->mutex_cond);
        }

        if (my_ball->state == ACTIVE)
        {
            int result = test_collision(idx, thread_param->game_state->ball, NB_OF_BALLS);

            pthread_mutex_lock(my_mutex);
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
            pthread_mutex_unlock(my_mutex);
            // collision with another ball
            if (result >= 0)
            {
                printf("touch ! : %d -> %d\n", idx, result);
                if (thread_param->game_state->ball[result].state == STOPPED)
                {
                    thread_param->game_state->ball[result].state = 0;
                }
                else
                {
                    my_calc_collision_speeds(my_ball, &thread_param->game_state->ball[result], my_mutex, thread_param->game_state->mutex[result]);
                }

                Mix_PlayChannel(-1, sound_chock, 0);
            }

            pthread_mutex_lock(my_mutex);

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

            pthread_mutex_unlock(my_mutex);
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
    game_state.cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK_NP);

    for (uint8_t i = 0; i < NB_OF_BALLS; i++)
    {
        game_state.mutex[i] = calloc(1, sizeof(pthread_mutex_t));
        pthread_mutex_init(game_state.mutex[i], &mutex_attr);
    }

    game_state.mutex_cond = calloc(1, sizeof(pthread_mutex_t));
    pthread_mutex_init(game_state.mutex_cond, &mutex_attr);

    pthread_mutexattr_destroy(&mutex_attr);

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
        free(game_state.mutex[i]);
    }

    free(game_state.mutex_cond);
    pthread_cond_destroy(&(game_state.cond));

    return 0;
}
