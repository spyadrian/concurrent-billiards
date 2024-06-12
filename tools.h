/*
 * Name        : tools.h
 * Author      : VP
 * Date        : 12.2023
 * Description : utility functions: test ball collisions, timings, etc.
 */

#ifndef TOOLS_H_
#define TOOLS_H_

#include <stdint.h>
#include <stdbool.h>

// collisions with screen border (see function test_collision)
enum {
	NO_COLLISION=-1,
	TOUCH_LEFT=-2,
	TOUCH_RIGHT=-3,
	TOUCH_UP=-4,
	TOUCH_DOWN=-5,
};

#define ACTIVE -2
#define STOPPED -1
#define NOT_ACTIVE 6	
#define EXPL_PHASE_DURATION 10			// multiplied by 20 ms

typedef struct
{
	float x;
	float y;
} coord_t;

typedef struct
{
	int x;
	int y;
} coord_fx_t;


// note that coord_t contains "float" values that must be finally cast
// on integers to display the balls 
typedef struct
{
	coord_t pos;					// defines the position of the center of the ball
	coord_t speed;				// speed vector defining the next ball position
	uint8_t radius;				// ball size (number of pixels considered around pos in x and y) 
	int state;						// can be ACTIVE, STOPPED, exploding (0 to 5) or NOT_ACTIVE
	int timer;						// counter of 20 ms laps time (used to manage ball explosion)
} ball_t;


/* Description: detects all collisions among all balls in every direction.
 * Parameter: ball_id: index of the ball testing collisions
 	            balls: pointer on the array of all balls
 	            nb_of_balls: total number of balls
 * Return: collision ball index or NO_COLLISION. If a border is touched, TOUCH_LEFT,
 	         TOUCH_RIGHT, TOUCH_UP or TOUCH_DOWN is returned
 */
int test_collision(int ball_id, ball_t *balls, int nb_of_balls);

/* return updated value: time_start + offset_ms. wait until current time reaches
 the time value returned. If delta time is negative or zero, do not wait. */
struct timespec wait_until(struct timespec time_start, unsigned long offset_ms);

// calculates and returns the time delay between current time and time_start
long timespec_diff_ms(struct timespec time_start);


#define MAX(x,y) (((x)>(y))?(x):(y))
#define MIN(x,y) (((x)<(y))?(x):(y))
#define ABS(x)  (((x)<0)?-(x):(x))

#endif /* TOOLS_H_ */
