/*
 * tools.c
 *
 *  Created on:12.2023
 *  Author: VP
 */

#include "tools.h"

#include <math.h>
#include <time.h>
#include <unistd.h>

#include "display_and_sound.h"

typedef long long LL;

/* detect all collisions among all balls in every direction
 * Return: TOUCH_LEFT if left side of window is touched, TOUCH_RIGHT
 * for right side side or the ball index if a collision occurs with
 * a ball which is not exploding (or inactive). NO_COLLISION is returned
 * if no collision has been detected in front of the ball (considering
 * its speed direction)
 */
int test_collision(int ball_id, ball_t *balls, int nb_of_balls)
{
    int col_dir_idx = NO_COLLISION;
    coord_t pos;

    // Test collision at next position of the ball[ball_id]. ! to rounding !
    pos.x = balls[ball_id].pos.x + balls[ball_id].speed.x;
    pos.y = balls[ball_id].pos.y + balls[ball_id].speed.y;

    // Cast positions to integers for display purposes
    int int_pos_x = (int)pos.x;
    int int_pos_y = (int)pos.y;

    // Test screen bounds on x axis
    if (int_pos_x + (balls[ball_id].radius * 2) >= WINDOW_WIDTH)
    {
        return TOUCH_RIGHT;
    }
    if (int_pos_x <= 0)
    {
        return TOUCH_LEFT;
    }

    // Test screen bounds on y axis
    if (int_pos_y + (balls[ball_id].radius * 2) >= WINDOW_HEIGHT)
    {
        return TOUCH_DOWN;
    }
    if (int_pos_y <= 0)
    {
        return TOUCH_UP;
    }
    // technique de collision
    // https://developer.mozilla.org/en-US/docs/Games/Techniques/2D_collision_detection
    // Test collision with another ball:
    for (int i = 0; i < nb_of_balls; i++)
    {
        if (i != ball_id && balls[i].state == ACTIVE)
        {
            coord_t ball_actuel = balls[i].pos;
            ball_actuel.x = balls[i].pos.x + balls[i].speed.x;
            ball_actuel.y = balls[i].pos.y + balls[i].speed.y;

            float dx = pos.x - ball_actuel.x;
            float dy = pos.y - ball_actuel.y;
            float distance = sqrt(dx * dx + dy * dy);

            // Calculate the sum of the radii
            int sommes_radius = balls[ball_id].radius + balls[i].radius;

            // Check for collision
            if (distance < sommes_radius)
            {
                return i;
            }
        }
    }

    return col_dir_idx;
}

struct timespec wait_until(struct timespec time_start, unsigned long offset_ms)
{
    struct timespec current_time, ret_time;
    LL end_ns, delay_ns;

    end_ns = (LL)time_start.tv_sec * (LL)1000000000 + (LL)time_start.tv_nsec +
             (LL)offset_ms * (LL)1000000;
    ret_time.tv_sec = end_ns / 1000000000;
    ret_time.tv_nsec = end_ns - ((LL)ret_time.tv_sec * (LL)1000000000);
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    delay_ns = end_ns - ((LL)current_time.tv_sec * (LL)1000000000 +
                         (LL)current_time.tv_nsec);
    if (delay_ns > 0)
    {
        // printf("wait: %ld us\n", (long)(delay_ns/1000));
        usleep((unsigned long)(delay_ns / 1000));
    }
    return ret_time;
}

long timespec_diff_ms(struct timespec time_start)
{
    struct timespec current_time;
    long start_ms;

    start_ms =
        (LL)time_start.tv_sec * (LL)1000 + (LL)time_start.tv_nsec / 1000000;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    return (long int)((current_time.tv_sec * 1000 +
                       (long)(current_time.tv_nsec / 1000000LL)) -
                      start_ms);
}
