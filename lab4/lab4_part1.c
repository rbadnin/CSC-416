void init_encoder() {
    // enable encoder interrupts
    EIMSK = 0;
    EIMSK |= _BV(PCIE1) | _BV(PCIE0);

    PCMSK1 |= _BV(PCINT13); //PB5 - digital 5
    PCMSK0 |= _BV(PCINT6);  //PE6 - digital 4

    // enable pullups
    PORTE |= _BV(PE6);
    PORTB |= _BV(PB5);
}


volatile uint16_t left_encoder = 0;
volatile uint16_t right_encoder = 0;


ISR(PCINT0_vect) {
   left_encoder++;  //increment left encoder
}


ISR(PCINT1_vect) {
   right_encoder++;  //increment right encoder
}