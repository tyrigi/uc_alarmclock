// digit_counter.c
// Tyler Gilbert
// Oct. 3 2017

// This program increments a decimal number displayed on the OSU 4-digit board
// depending on which of 8 buttons are pressed. 

// S0 - +1      S1 - +2
// S2 - +4      S3 - +8
// S4 - +16     S5 - +32
// S6 - +64     S7 - +128

// Display maxes out at 1023, then rolls over to 1. 

// Hardware Setup:
// PA0 -  SegA & S0
// PA1 -  SegB & S1
// PA2 -  SegC & S2
// PA3 -  SegD & S3
// PA4 -  SegE & S4
// PA5 -  SegF & S5
// PA6 -  SegG & S6
// PA7 - SegDP & S7

// PB4 - Sel0
// PB5 - Sel1
// PB6 - Sel2
// PB7 - PWM

// PC0 - Button Enable

#define F_CPU 16000000
#define TRUE 1
#define FALSE 0
#include <avr/io.h>
#include <util/delay.h>

// Stores the seven-segment display data. 0 is ON
uint8_t segment_data[5]
// Stores the 7-seg patterns for different decimal numbers. 0 is ON
uint8_t dec_to_7seg[12]

uint8_t chk_buttons(uint8_t button) {
    // check requested button's state. Use debouncing.
}

void segsum(uint16_t sum) {
    // converts the 16-bit value 'sum' into the cooresponding display code
    // and stores it in segment_data
}

uint8_t main() {
    // port setup
    while(1){
        // debounce delay
        
        // set Port A to input
        // Enable pushbuttons
        
        // check all buttons
        // increment sum, deal with rollover

        // disable pushbuttons
        // convert sum to display codes

        // set Port A to output
        // flash all 4 digits with their appropriate number

    }
}









