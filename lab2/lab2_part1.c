#include "globals.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

// Note: on our robot, 0: Right, 1: Left
void motor(uint8_t num, int8_t speed)
{
    set_servo(num, 127 + (speed * 0.3));
}

int main(void)
{
    init(); // initialize board hardware

    // reset motors
    motor(0, 0);
    motor(1, 0);

    u08 currSpeed = 0;

    print_string("waiting");
    _delay_ms(5000);

    clear_screen();
    print_string("go!");

    while (1)
    {
        // speed up forward
        while (currSpeed < 100)
        {
            motor(0, currSpeed * -1);
            motor(1, currSpeed);
            currSpeed += 5;
            clear_screen();
            print_num(currSpeed);
            _delay_ms(100);
        }
        // slow down forward
        while (currSpeed > 0)
        {
            motor(0, currSpeed * -1);
            motor(1, currSpeed);
            currSpeed -= 5;
            clear_screen();
            print_num(currSpeed);
            _delay_ms(100);
        }
        // speed up reverse
        while (currSpeed < 100)
        {
            motor(0, currSpeed);
            motor(1, currSpeed * -1);
            currSpeed += 5;
            clear_screen();
            print_num(currSpeed);
            _delay_ms(100);
        }
        // slow down reverse
        while (currSpeed > 0)
        {
            motor(0, currSpeed);
            motor(1, currSpeed * -1);
            currSpeed -= 5;
            clear_screen();
            print_num(currSpeed);
            _delay_ms(100);
        }
    }

    return 0;
}