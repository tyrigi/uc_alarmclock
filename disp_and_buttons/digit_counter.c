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
#define DEBOUNCE_TIME 50
#define SHOW_TIME 500
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
    0b11111111, // Blank
    0b11111111  // ?
};

uint8_t chk_buttons(uint8_t button) {
    // check requested button's state. Use debouncing.

    // Array of static state variables, one for each button
    static uint16_t state[] = {0, 0, 0, 0, 0, 0, 0, 0};

    // Check the button's state
    state[button] = (state[button] << 1) | (! bit_is_clear(PINA, button)) | 0xE000;

    // If the button has been pressed for the last 12 checks, return true
    if (state[button] == 0xF000) {return 1;}
    return 0;
}

void segsum(uint16_t sum) {
    // converts the 16-bit value 'sum' into the cooresponding display code
    // and stores it in segment_data
    uint8_t tous = 0;
    uint8_t huns = 0;
    uint8_t tens = 0;
    uint8_t ones = 0;
    while (sum >= 1000) {
        sum -= 1000;
        tous++;
    }
    while (sum >= 100) {
        sum -= 100;
        huns++;
    }
    while (sum >= 10) {
        sum -= 10;
        tens++;
    }
    while (sum >= 1) {
        sum -= 1;
        ones++;
    }
    // Leading zero suppression. Note that a value of '10' for a digit means blank.
    if (tous == 0){
        if (huns == 0){
            if (tens == 0){
                tous = 10;
                huns = 10;
                tens = 10;
            } else {
                tous = 10;
                huns = 10;
            }
        } else {
            tous = 10;
        }
    }
    segment_data[4] = tous; // Store the thousands digit
    segment_data[3] = huns; // Store the hundreds digit
    segment_data[1] = tens; // Store the tens digit
    segment_data[0] = ones; // Store the ones digit
}

uint8_t main() {
    // Configure outputs
    DDRB |= (1<<PB7) | (1<<PB6) | (1<<PB5) | (1<<PB4);
    DDRC |= (1<<PC0);
    uint16_t count = 0;
    uint8_t debounce_counter = 0;

    // Set PWM pin Low to keep display on
    PORTB &= ~(1<<PB7);

    while(1){
        // set Port A to input
        DDRA = 0b00000000;
        // Activate pullup resistors
        PORTA = 0b11111111;
        // Enable pushbuttons
        PORTC &= ~(1<<PC0);
        
        // check all buttons
        for (uint8_t x=0; x<8; x++){
            // if button is pressed, add the defined number to the total count
            if (chk_buttons(x)) {count += 0x1 << x;}
            // increment sum, deal with rollover
            if (count >= 1024) {count = 1;}
        }

        // disable pushbuttons
        PORTC |= (1<<PC0);
        
        // convert sum to display codes
        segsum(count);

        // set Port A to output
        DDRA = 0b11111111;

        // flash all 4 digits with their appropriate number
        PORTB &= ~((1<<PB6)|(1<<PB5)|(1<<PB4)); // Select the Ones digit
        PORTA = dec_to_7seg[segment_data[0]];
        _delay_us(SHOW_TIME);
        PORTB |= (1<<PB4); // Select Tens Digit
        PORTA = dec_to_7seg[segment_data[1]];
        _delay_us(SHOW_TIME);
        PORTB |= (1<<PB5)|(1<<PB4); // Select Hundreds digit
        PORTA = dec_to_7seg[segment_data[3]];
        _delay_us(SHOW_TIME);
        PORTB &= ~((1<<PB5)|(1<<PB4)); //select Thousands bit
        PORTB |= (1<<PB6);
        PORTA = dec_to_7seg[segment_data[4]];
        _delay_us(SHOW_TIME);
        PORTB |= (1<<PB6) | (1<<PB5) | (1<<PB4); // Prevent button presses from turning on segments

    }
}









