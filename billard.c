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


int main()
{
	game_state_t game_state;

	display_func(NULL);
	
	// example of call of calc_collision_speeds() (! variables not intialised here !)
	calc_collision_speeds(&game_state.ball[0], &game_state.ball[1]);
	
	return 0;
}
