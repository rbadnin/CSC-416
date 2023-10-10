/**
    Name:  Riley Badnin and Justin Brunings
    Lab 1 part 3
    Description: A simple pong game with LEDs
*/

#include "globals.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

int main(void)
{
    init(); // initialize board hardware

    digital_dir(0, 1);
    digital_dir(1, 1);
    digital_dir(2, 1);
    digital_dir(3, 1);
    digital_dir(4, 1);

    int currLight = 1;
    int dir = 0;
    int currDelay = 500;
    int loopCount = 0;
    int buttonPressed = 0;
    char currDelayStr[3];

    print_string("Ready");
    _delay_ms(1000);
    clear_screen();

    print_string("Set");
    _delay_ms(1000);
    clear_screen();

    print_string("Go");
    _delay_ms(1000);
    clear_screen();

    while (1)
    {
        digital_out(currLight, 1);

        for (loopCount = 0; loopCount < currDelay; loopCount++)
        {
            _delay_ms(1);
            if (currLight == 0 || currLight == 4)
            {
                if (get_btn())
                    buttonPressed = 1;
            }
        }

        if (currLight == 0 || currLight == 4)
        {
            if (buttonPressed != 1)
            {
                sprintf(currDelayStr, "%d", currDelay);
                print_string(currDelayStr);
                break;
            }
            else
                buttonPressed = 0;
        }

        digital_out(currLight, 0);

        if (dir == 0)
        {
            if (currLight == 4)
            {
                dir = 1;
                currLight--;
            }
            else
                currLight++;
        }
        else
        {
            if (currLight == 0)
            {
                dir = 0;
                currLight++;
            }
            else
                currLight--;
        }

        currDelay *= 0.99;
    }

    return 0;
}