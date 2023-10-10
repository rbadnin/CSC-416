/**
    Name:  Riley Badnin and Justin Brunings
    Lab 1 part 1
    Description: Alternates fading both LEDs on and off
*/

#include "globals.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>


int delay(int count) 
{
    int i;  

    for (i = 0; i < count; i++)
    {
        _delay_us(1);
    }
    return 0;
}

int fade(int led) 
{
    int delayCt = 0;

    led_off(led);

    while (delayCt < 1000) 
    {
        led_on(led);
        delay(delayCt);
        led_off(led);
        delay(1000 - delayCt);
        delayCt++;
    }

    delayCt = 0;

    while (delayCt < 1000) 
    {
        led_off(led);
        delay(delayCt);
        led_on(led);
        delay(1000 - delayCt);
        delayCt++;
    }

    led_off(led);

    return 0;
}

int main(void) 
{
    init();

    while (1) 
    {
        fade(0);
        fade(1);
    }

    return 0;
}

