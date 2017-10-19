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
#define DEBOUNCE_TIME 32
#define SHOW_TIME 2
#define LOOPS 25
#include <avr/io.h>
#include <util/delay.h>

// Stores the seven-segment display data. 0 is ON
uint8_t segment_data[5] = {
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
};
// Stores the 7-seg patterns for different decimal numbers. 0 is ON
uint8_t dec_to_7seg[12] = {
    0b11000000, // 0
    0b11111001, // 1
    0b10100100, // 2
    0b10110000, // 3
    0b10011001, // 4
    0b10010010, // 5
    0b10000010, // 6
    0b11111000, // 7
    0b10000000, // 8
    0b10010000, // 9
    0b11111111, // ?
    0b11111111  // ?
};

uint8_t main() {
    DDRB = 0b11111111;
    DDRC = 0b11111111;
    uint8_t x=0;
    uint8_t wait=0;
    // port setup
    while(1){
        PORTC |= (1<<PC0);
        // convert sum to display codes

        // set Port A to output
        DDRA = 0b11111111;
        // flash all 4 digits with their appropriate number
        segment_data[0] = x;
        segment_data[1] = x;
        segment_data[3] = x;
        segment_data[4] = x;
        PORTB = 0b00001111;
        //PORTB &= ~((1<<PB6)|(1<<PB5)|(1<<PB4)); // Select the Ones digit
        PORTA = dec_to_7seg[x];//segment_data[0]];
        _delay_ms(SHOW_TIME);
        PORTB = 0b00011111;
        //PORTB |= (1<<PB4); // Select Tens Digit
        PORTA = dec_to_7seg[x];//segment_data[1]];
        _delay_ms(SHOW_TIME);
        PORTB = 0b00111111;
        //PORTB |= (1<<PB5)|(1<<PB4); // Select Hundreds digit
        PORTA = dec_to_7seg[x];//segment_data[3]];
        _delay_ms(SHOW_TIME);
        PORTB = 0b01001111;
        //PORTB &= (1<<PB6) | ~((1<<PB5)|(1<<PB4)); //select Thousands bit
        PORTA = dec_to_7seg[x];//segment_data[4]];
        _delay_ms(SHOW_TIME);
        PORTB |= (1<<PB7);
        //_delay_ms(DEBOUNCE_TIME);
        wait++;
        if (wait == LOOPS){
            x++;
            if (x>=10) {x=0;}
            wait = 0;
        }
    }
}









