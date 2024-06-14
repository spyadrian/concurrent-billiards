#ifndef BILLARD_H
#define BILLARD_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <pthread.h>
#include <semaphore.h>

#include "tools.h"

#define NB_OF_BALLS 10
#define SPEED_MAX 3.       // pixels/7 ms
#define REBOUND_FACTOR 1   // no loss of speed in cas of border rebound
#define SPEED_FACTOR .998  // speed loss/7 ms
#define SPEED_MIN 0.05     // minimum speed. Considered to be 0 below
#define BALL_DELAY 7       // [ms] between each move of a ball
#define BALL0_ACCEL 0.1    // speed increase of the user ball when pushed (=activated by keyboard)
#define BALL_RADIUS 20

typedef struct
{
    ball_t ball[NB_OF_BALLS];  // ball coordinates
    bool game_playing;
    uint8_t counter;
    pthread_mutex_t *mutex[NB_OF_BALLS];
} game_state_t;

void init_balls_param(game_state_t *game_state);

#endif
