#include "globals.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>

struct motor_command
{
    uint8_t left_speed;
    uint8_t right_speed;
}

struct motor_command
compute_proportional(uint8_t left, uint8_t right);

int main(void)
{
    return 0;
}