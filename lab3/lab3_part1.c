#include "globals.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>

// Declarations

struct motor_command
{
    uint8_t left_speed;
    uint8_t right_speed;
}

struct motor_command
compute_proportional(uint8_t left, uint8_t right);

// makes use of compute_proprtional below
int main(void)
{
    return 0;
}


/*
    The function should take 2 uint8_t values as parameters (these will represent 2 sensor readings).  
    The function should return a struct motor_command containing the speeds to assign to left and right motors.  
    Verify that your proportional controller can run the track smoothly and without any jittery motion.
*/
motor_command compute_proportional(uint8_t left, uint8_t right)
{
    struct motor_command result_command;

    leftSensor = analog(4);
    rightSensor = analog(5) - 5;

    // finalize the logic in oval.c before doing this

    return result_command;
}
