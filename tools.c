/*
 * tools.c
 *
 *  Created on:12.2023
 *  Author: VP
 */

#include <math.h>
#include <time.h>
#include <unistd.h>
#include "tools.h"
#include "display_and_sound.h"

typedef long long LL;

/* detect all collisions among all balls in every direction
 * Return: TOUCH_LEFT if left side of window is touched, TOUCH_RIGHT
 	 for right side side or the ball index if a collision occurs with
 	 a ball wich is not explodind (or inactive). NO_COLLISION is returned
 	 if no collision has been detected in front of the ball (considering
 	 its speed direction)
 */
int test_collision(int ball_id, ball_t *balls, int nb_of_balls)
{
	int col_dir_idx=NO_COLLISION; 
	coord_t pos;

	// test collision at next position of the ball[ball_id]. ! to rounding !
	pos.x=balls[ball_id].pos.x+balls[ball_id].speed.x;
	pos.y=balls[ball_id].pos.y+balls[ball_id].speed.y;
	// test screen bounds:
	//...
	//test collision with another ball:
	//...
	
	return col_dir_idx;
}


struct timespec wait_until(struct timespec time_start, unsigned long offset_ms)
{
	struct timespec current_time, ret_time;
	LL end_ns, delay_ns;
	
	end_ns=(LL)time_start.tv_sec*(LL)1000000000+(LL)time_start.tv_nsec + (LL)offset_ms*(LL)1000000;
	ret_time.tv_sec=end_ns/1000000000;
	ret_time.tv_nsec=end_ns-((LL)ret_time.tv_sec*(LL)1000000000); 
	clock_gettime(CLOCK_MONOTONIC, &current_time);
	delay_ns=end_ns-((LL)current_time.tv_sec*(LL)1000000000+(LL)current_time.tv_nsec);
	if (delay_ns>0)
	{
			//printf("wait: %ld us\n", (long)(delay_ns/1000));
			usleep((unsigned long)(delay_ns/1000));
	}
	return ret_time;
}


long timespec_diff_ms(struct timespec time_start)
{
	struct timespec current_time;
	long start_ms;

	start_ms=(LL)time_start.tv_sec*(LL)1000+(LL)time_start.tv_nsec/1000000;
	clock_gettime(CLOCK_MONOTONIC, &current_time);
	return (long int)((current_time.tv_sec*1000+(long)(current_time.tv_nsec/1000000LL))-start_ms);
}