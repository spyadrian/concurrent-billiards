#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "billard.h"

void wait_key_release()
{
	SDL_Event event;
	while(1)
  	{
  		if (SDL_PollEvent(&event))
    		if (event.type==SDL_KEYUP)
          		break;
  	}
}

void *keyboard_func(void *param)
{
	SDL_Event event;
	int nb_of_keys;
	uint8_t *keys;

  do
  {
  	SDL_WaitEvent(&event);				// passive waiting 
   	switch (event.type) 
  	{
  		case SDL_QUIT:

  			break;
  		case SDL_WINDOWEVENT:
  			if (event.window.event==SDL_WINDOWEVENT_CLOSE)
    		{
   			}
  			break;
  		case SDL_KEYDOWN:
  			keys=(uint8_t *)SDL_GetKeyboardState(&nb_of_keys);
    			if (keys[SDL_SCANCODE_ESCAPE])
    			{
   				}
    			if (keys[SDL_SCANCODE_RIGHT])
    			{
   				}
     			if (keys[SDL_SCANCODE_LEFT])
     			{
     			}
     			if (keys[SDL_SCANCODE_UP])
     			{
     			}
     			if (keys[SDL_SCANCODE_DOWN])
     			{
     			}   				
     			if (keys[SDL_SCANCODE_S])		
     			{
     		  }
  	} // switch
  } while(1);

  return NULL;
}
