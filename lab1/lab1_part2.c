/**
    Name:  Riley Badnin and Justin Brunings
    Lab 1 part 2
    Description: Scrolls both names across the screen, changing on button press
*/

#include "globals.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>


int rotateArray(char* nameArr) 
{
    char tempNameArr[8];

    memcpy(tempNameArr, nameArr, 8);
    nameArr[0] = tempNameArr[7];
    memcpy(&nameArr[1], tempNameArr, 7);

    return 0;
}


int scrollName(char* name) 
{
    while (get_btn());

    clear_screen();

    while (!get_btn()) 
    {
        print_string(name);
        _delay_ms(200);
        clear_screen();
        rotateArray(name);
        print_string(name);
    }

    return 0;
}


int main(void) 
{
    init(); 
    int isRiley = 1;

    while(1) 
    {
        if (isRiley) 
        {
            scrollName("Riley   ");
            isRiley = 0;
        }
        else 
        {
            scrollName("Justin  ");
            isRiley = 1;
        }
    }

    return 0;
}

