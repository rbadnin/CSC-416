#include "globals.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>

void motor(uint8_t num, int8_t speed)
{
    set_servo(num, 127 + (speed * 0.3));
}

int calculateErr(u08 actual)
{
    u08 error = 100 - actual;
    if (error > 100)
        return 100;
    if (error < 0)
        return 0;
    return error;
}

int main(void)
{
    u08 rightSensor;
    u08 leftSensor;
    u08 white = 100;
    u08 black = 220;
    u08 diff = black - white;

    u16 timeSinceTurning = 0;

    init();
    set_servo(0, 127);
    set_servo(1, 127);
    digital_dir(5, 0);
    digital_dir(4, 0);

    motor(0, 0);
    motor(1, 0);

    while (1)
    {
        timeSinceTurning += 20;

        leftSensor = analog(4);
        rightSensor = analog(5) - 5;

        u08 maxSpeed = 20;
        clear_screen();
        print_num(leftSensor);
        print_string(" ");
        print_num(rightSensor);

        // Speed up if less than 0 (should balance out the robot in the center of the line)
        if ((leftSensor < 215))
        {
            motor(0, maxSpeed + ((210 - leftSensor) * 5));
        }
        if ((rightSensor < 215))
        {
            motor(1, (maxSpeed + ((210 - rightSensor) * 5)) * -1);
        }

        if ((leftSensor > 210 && rightSensor > 210) || abs(leftSensor - rightSensor) < 2)
        {
            motor(0, 40);
            motor(1, -40);
            _delay_ms(100);
        }
        else if (rightSensor > 214)
        { // turn right
            if (timeSinceTurning > 1000)
            {
                motor(1, 0);
                motor(0, 39);
                _delay_ms(1950);
                timeSinceTurning = 0;
            }
        }
        else if (leftSensor > 214)
        {
            if (timeSinceTurning > 1000)
            {
                motor(1, -39);
                motor(0, 0);
                _delay_ms(1950);
                timeSinceTurning = 0;
            }
        }

        _delay_ms(1);
    }

    return 0;
}