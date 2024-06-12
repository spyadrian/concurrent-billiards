#include "display_and_sound.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "billard.h"
#include "tools.h"

#define FRAME_RATE 20

SDL_Texture *create_texture_from_image_file(SDL_Renderer *renderer, const char *img_filename, SDL_Rect *img_rect)
{
    SDL_Surface *img_surface;
    SDL_Texture *texture;
    int img_flags = IMG_INIT_PNG;
    static bool img_init = false;

    if (!img_init)
    {
        img_init = true;
        if (!(IMG_Init(img_flags) & img_flags))
        {
            printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
            exit(1);
        }
    }
    img_surface = IMG_Load(img_filename);
    SDL_GetClipRect(img_surface, img_rect);

    assert((texture = SDL_CreateTextureFromSurface(renderer, img_surface)) != NULL);

    SDL_FreeSurface(img_surface);
    return texture;
}

void *display_func(void *param)
{
    // --- initialize ---

    // - thread param -
    game_state_t *game_state = param;

    // - SDL param -
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *red_ball_texture[6], *blue_ball_texture, *black_ball_texture;
    SDL_Rect ball_rect;
    char filename[100];
    SDL_Rect ball_current_pos;
    Mix_Chunk *sound_chock;   // sound pointer (used in case of chock between 2 moving balls)
    Mix_Chunk *sound_broken;  // sound pointer (used at the beginning of a ball explosion)

    // - graphic initialization -
    assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0);
    assert((window = SDL_CreateWindow("Billard", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL)) != NULL);
    assert((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)) != NULL);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);  // allow transparent mode!
    assert((red_ball_texture[0] = create_texture_from_image_file(renderer, "images/red_ball.png", &ball_rect)) != NULL);

    for (int i = 0; i < 5; i++)
    {
        sprintf(filename, "images/explode%d.png", i);
        assert((red_ball_texture[i + 1] = create_texture_from_image_file(renderer, filename, &ball_rect)) != NULL);
    }

    // - create texture -
    assert((blue_ball_texture = create_texture_from_image_file(renderer, "images/blue_ball.png", &ball_rect)) != NULL);
    assert((black_ball_texture = create_texture_from_image_file(renderer, "images/black_ball.png", &ball_rect)) != NULL);
    ball_current_pos.w = ball_rect.w;
    ball_current_pos.h = ball_rect.h;

    // - sound initializazion -
    assert(Mix_OpenAudio(20000, MIX_DEFAULT_FORMAT, 2, 512) >= 0);

    assert((sound_chock = Mix_LoadWAV("sounds/chock.wav")) != 0);
    assert((sound_broken = Mix_LoadWAV("sounds/broken.wav")) != 0);

    // --- Thread routine ---

    SDL_RenderClear(renderer);

    struct timespec start_main_task = {0};
    while (1)
    {
        clock_gettime(CLOCK_MONOTONIC, &start_main_task);

        // - render blue ball -
        ball_current_pos.x = game_state->ball[0].pos.x;
        ball_current_pos.y = game_state->ball[0].pos.x;

        SDL_RenderCopy(renderer, blue_ball_texture, NULL, &ball_current_pos);

        // - render red ball -
        for (size_t i = 1; i < NB_OF_BALLS; i++)
        {
            ball_t *ball = &(game_state->ball[i]);

            if (ball->state == NOT_ACTIVE)
                continue;

            // get pos
            ball_current_pos.x = ball->pos.x;
            ball_current_pos.y = ball->pos.y;

            // choose texture
            if (ball->state == ACTIVE || ball->state == STOPPED)
            {
                SDL_RenderCopy(renderer, red_ball_texture[0], NULL, &ball_current_pos);
            }
            else  // EXPLODING
            {
                if (ball->timer == EXPL_PHASE_DURATION)
                {
                    ball->timer = -1;
                    ball->state += 1;
                }

                ball->timer += 1;
                SDL_RenderCopy(renderer, red_ball_texture[ball->state], NULL, &ball_current_pos);
            }
        }

        // Mix_PlayChannel(-1, sound_chock, 0);

        SDL_RenderPresent(renderer);
        SDL_RenderClear(renderer);

        wait_until(start_main_task, FRAME_RATE);
    }

    // - destroy everything -

    SDL_DestroyTexture(black_ball_texture);
    SDL_DestroyTexture(blue_ball_texture);
    for (int i = 0; i < 6; i++)
        SDL_DestroyTexture(red_ball_texture[i]);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return NULL;
}
