// Template of PCO TP: billard
// spring 2024

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "keyboard.h"
#include "display_and_sound.h"
#include "billard.h"
#include "tools.h"
#include "calc_collision_speeds.h"
#include <SDL2/SDL_mixer.h>

void init_balls_param(game_state_t *game_state)
{
	for (uint8_t i = 0; i < NB_OF_BALLS; i++)
	{
		game_state->ball[i].pos = {i * 10f + 30f, i * 10f + 30f};
		game_state->ball[i].radius = 5;
		game_state->ball[i].speed = {0f, 0f};
		game_state->ball[i].state = STOPPED;
		game_state->ball[i].timer = 0;
	}
}

void *boule_func(void *args)
{
	uint8_t idx;
	// --- initialize ---
	game_state_t *thread_param = (game_state_t *)args;

	// get index via counter with a mutex
	pthread_mutex_lock(thread_param->mutex);
	idx = *(thread_param->counter);
	*(thread_param->counter) = *(thread_param->counter) + 1;
	pthread_mutex_unlock(thread_param->mutex);

	// --- Thread Routine ---
	while (thread_param->game_playing)
	{
		struct timespec current_time;
		clock_gettime(CLOCK_MONOTONIC, &current_time);
		if (test_collision(idx,thread_param->ball,NB_OF_BALLS))
		{
			
		}
		

		//ralentissement obligatoire du au frottement
		thread_param->ball[idx].speed.x -= 0.002;
		thread_param->ball[idx].speed.y -= 0.002;
		wait_until(current_time, BALL_DELAY);
	}

	return NULL;
}

int main()
{
	game_state_t game_state;

	// mutex for indexes counter
	pthread_mutexattr_t mutex_attr;

	pthread_mutexattr_init(&mutex_attr);
	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);

	game_state.mutex = calloc(1, sizeof(pthread_mutex_t));
	pthread_mutex_init(&game_state.mutex, &mutex_attr);
	pthread_mutexattr_destroy(&mutex_attr);

	// - threads -
	pthread_t t[NB_OF_BALLS + 2] = {0};

	for (uint8_t i = 0; i < NB_OF_BALLS; i++)
	{
		pthread_create(&t[i], NULL, boule_func, &game_state);
	}
	pthread_create(&t[NB_OF_BALLS], NULL, keyboard_func, &game_state);
	pthread_create(&t[NB_OF_BALLS + 1], NULL, display_func, &game_state);

	// --- Threads Handling ---

	for (uint8_t i = 0; i < NB_OF_BALLS + 2; i++)
	{
		pthread_join(t[i], NULL);
	}

	// example of call of calc_collision_speeds() (! variables not intialised here !)
	calc_collision_speeds(&game_state.ball[0], &game_state.ball[1]);

	return 0;
}
