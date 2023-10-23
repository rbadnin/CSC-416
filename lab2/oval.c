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
    u08 error = abs(40 - actual);
    if (error > 100)
        return 100;
    if (error < 0)
        return 20;
    return error;
}


int main(void)
{
    u08 rightSensor;
    u08 leftSensor;

    init();
    set_servo(0, 127);
    set_servo(1, 127);
    digital_dir(0, 0);
    digital_dir(1, 0);

    motor(0, 0);
    motor(1, 0);

    while (1)
    {
        leftSensor = analog(0);
        rightSensor = analog(1);

        u08 maxSpeed = 20;
        u08 rightWheelBuffer = 2;
        clear_screen();
        print_num(rightSensor);
        print_string(" ");
        print_num(leftSensor);

        if (leftSensor < 100 && rightSensor < 100)
        {
            motor(0, maxSpeed);
            motor(1, -1 * (maxSpeed + rightWheelBuffer));
        }
        else 
        {
            motor(0, calculateErr(rightSensor) * 0.4);
            motor(1, ((calculateErr(leftSensor) * 0.4) + rightWheelBuffer) * -1);
        }

        _delay_ms(200);
    }

    return 0;
}