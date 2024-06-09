#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "display_and_sound.h"
#include "billard.h"
#include "tools.h"

SDL_Texture *create_texture_from_image_file(SDL_Renderer *renderer, const char *img_filename, SDL_Rect *img_rect)
{
	SDL_Surface *img_surface;
	SDL_Texture *texture;
	int img_flags=IMG_INIT_PNG;
	static bool img_init=false;
		
	if (!img_init)
	{
		img_init=true;
  	if( !( IMG_Init( img_flags ) & img_flags ) )
  	{
      printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
      exit(1);
  	}
  }
  img_surface = IMG_Load(img_filename);
  SDL_GetClipRect(img_surface, img_rect);
 		 
  assert((texture = SDL_CreateTextureFromSurface(renderer, img_surface))!=NULL);
  
  SDL_FreeSurface(img_surface);
  return texture;
}

void *display_func(void *param) 
{
	SDL_Window *window;
	SDL_Renderer *renderer;
  SDL_Texture *red_ball_texture[6], *blue_ball_texture, *black_ball_texture;
  SDL_Rect ball_rect;
	char filename[100];
	SDL_Rect ball_current_pos;
	Mix_Chunk* sound_chock;		// sound pointer (used in case of chock between 2 moving balls)
	Mix_Chunk* sound_broken;	// sound pointer (used at the beginning of a ball explosion)

  // Graphic initialization	
	assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0);
	assert((window = SDL_CreateWindow("Billard", SDL_WINDOWPOS_UNDEFINED,
	                          SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, 
	                          WINDOW_HEIGHT, SDL_WINDOW_OPENGL))!=NULL);
	assert((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED))!=NULL); 
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);   // allow transparent mode!                            
	assert((red_ball_texture[0]=create_texture_from_image_file(renderer, "images/red_ball.png", 
	                                                    &ball_rect))!=NULL); 
	for (int i=0; i<5; i++)
	{
		sprintf(filename, "images/explode%d.png",i);
		assert((red_ball_texture[i+1]=create_texture_from_image_file(renderer, filename, 
	                                                    &ball_rect))!=NULL);   
	}                                                                                               
	assert((blue_ball_texture=create_texture_from_image_file(renderer, "images/blue_ball.png", 
	                                                    &ball_rect))!=NULL);                                               
	assert((black_ball_texture=create_texture_from_image_file(renderer, "images/black_ball.png", 
	                                                    &ball_rect))!=NULL);                                               
	ball_current_pos.w=ball_rect.w;
	ball_current_pos.h=ball_rect.h;
	
	// Sound initializazion:
	assert(Mix_OpenAudio(20000, MIX_DEFAULT_FORMAT, 2, 512) >= 0); 
  // Load the sound files
  assert((sound_chock = Mix_LoadWAV("sounds/chock.wav"))!=0);
  assert((sound_broken = Mix_LoadWAV("sounds/broken.wav"))!=0);
  
  // Display example:
  ball_current_pos.x=40;
  ball_current_pos.y=50;
  SDL_RenderCopy(renderer, red_ball_texture[0], NULL, &ball_current_pos);	
	
  SDL_RenderPresent(renderer);		// display virtual image on screen
 
  // sound example:
  Mix_PlayChannel(-1, sound_chock, 0);
 
  sleep(3);
		
	SDL_DestroyTexture(black_ball_texture);
	SDL_DestroyTexture(blue_ball_texture);
	for (int i=0; i<6; i++)
  	SDL_DestroyTexture(red_ball_texture[i]);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return NULL;
}

