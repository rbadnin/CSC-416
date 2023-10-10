#include "globals.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

int main(void) {
   init();  //initialize board hardware
   print_string("Hello, World");
   return 0;
}
