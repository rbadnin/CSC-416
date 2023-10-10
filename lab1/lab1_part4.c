/**
    Name:  Riley Badnin and Justin Brunings
    Lab 1 part 4
    Description: A simple pong game with LEDs
*/

#include "globals.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>

int rotateArrayRight(char *nameArr)
{
    char tempNameArr[10];

    memcpy(tempNameArr, nameArr, 10);
    nameArr[0] = tempNameArr[9];
    memcpy(&nameArr[1], tempNameArr, 10);

    return 0;
}

int rotateArrayLeft(char *nameArr)
{
    char tempNameArr[10];

    memcpy(tempNameArr, nameArr, 10);
    memcpy(&nameArr[0], tempNameArr + 1, 9);
    nameArr[9] = tempNameArr[0];

    return 0;
}

int scrollName(char *name)
{
    clear_screen();

    print_string(name);

    return 0;
}

int main(void)
{
    init(); // initialize board hardware

    // get_accel_x moves up and down the screen, get_accel_y moves side to side
    int xMove = 0;
    int yMove = 0;
    int yLoc = 5;
    int delayTime = 0;
    int count;
    int currRow = 0;

    char str[] = "   416    ";

    while (1)
    {
        xMove = get_accel_x();
        yMove = get_accel_y();

        // handle y movement
        if ((yMove < 251 && yMove > 128) && (yLoc < 9))
        {
            delayTime = 1000 / (256 - yMove) * 2;
            rotateArrayRight(str);
            scrollName(str);
            yLoc++;
        }
        else if ((yMove > 5 && yMove < 128) && (yLoc > 0))
        {
            delayTime = 1000 / yMove * 2;
            rotateArrayLeft(str);
            scrollName(str);
            yLoc--;
        }

        if (xMove < 251 && xMove > 128)
        {
            currRow = 1;
        }
        else if (xMove > 5 && xMove < 128)
        {
            currRow = 0;
        }

        clear_screen();
        lcd_cursor(0, currRow);
        print_string(str);

        for (count = 0; count < delayTime; count++)
            _delay_ms(1);
    }

    return 0;
}