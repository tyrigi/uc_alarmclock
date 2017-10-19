// rotary_sel.c
// Tyler Gilbert
// Oct. 18 2017

// This program uses two rotary encoders to increment or decrement a count
// depending on which way they are rotated. Buttons can be used to toggle a 
// mode where the count is incremented by 2, and a mode where the count is
// incremented by 4. If both modes are active, the count is not incremented or 
// decremented. Count rolls over at 1023 and 0.

// S0 - toggle +-2 mode
// S1 - toggle +-4 mode
// S6 - Set count to 1020
// S7 - Set count to 0

// Port information:
// PA0 -  SegA & S0     PB0 - Bar Graph Storage Reg Clk
// PA1 -  SegB & S1     PB1 - Bar Graph & Encoder SPI Clk
// PA2 -  SegC & S2     PB2 - Bar Graph SPI Data In
// PA3 -  SegD & S3     PB3 - Encoder SPI Data Out
// PA4 -  SegE & S4     PB4 - 7-Seg Sel0
// PA5 -  SegF & S5     PB5 - 7-Seg Sel1
// PA6 -  SegG & S6     PB6 - 7-Seg Sel2
// PA7 - SegDP & S7     PB7 - 7-Seg Brightness

// PC0 - Pushbutton Enable
// PC1 - Encoder Serial Load
// PC2 - Encoder Clk Enable

#define F_CPU 16000000
#define SHOW_TIME 500

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Stores the count
uint16_t count = 0;

// Stores the digit info for the 7-seg display
uint8_t segment_data[5] = {
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
};

// Stores the encodings for all digits the 7-seg
// can display
uint8_t seg_digits[17] = {
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
    0b10001000, // A
    0b10000011, // B
    0b11000110, // C
    0b10100001, // D
    0b10000110, // E
    0b10001110, // F
    0b11111111, // Blank
};

void spi_init(void){
    DDRB |= 0x07;
    SPCR |= (1<<SPE) | (1<<MSTR);
    SPSR |= (1<<SPI2X);
}

uint8_t spi_read(void){
    PORTC &= ~(1<<PC1);
    _delay_us(10);
    PORTC |= (1<<PC1);
    SPDR = 0x00;
    while (bit_is_clear(SPSR,SPIF)){}
    return(SPDR);
}

uint8_t serial_update(uint8_t value){
    // Write an 8-bit value to the bar graph and return the encoder values
    PORTC &= ~((1<<PC2)|(1<<PC1));
    _delay_us(1);
    PORTC |= (1<<PC1);
    SPDR = value;
    while (bit_is_clear(SPSR,SPIF)){}
    PORTB |= (1<<PB0);
    PORTB &= ~(1<<PB0);
    PORTC |= (1<<PC2);
    return(SPDR);
}

uint8_t chk_buttons(uint8_t button) {
    // Checks a specified button's state, utilizes debouncing

    // Button states
    static uint16_t state[] = {0, 0, 0, 0, 0, 0, 0, 0};

    // Check the button's state
    state[button] = (state[button] << 1) | (!bit_is_clear(PINA, button)) | 0xE000;

    // If the button has been pressed the last 12 times it has been checked, return true
    if (state[button] == 0xF000){return 1;}
    return 0;
}

void segsum(uint16_t sum){
    // convert the 16-bit count into the base-10 display code

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

    // Leading zero suppression. Note that '0x10' is blank
    if (tous == 0){
        tous = 0x10;
        if (huns == 0){
            huns = 0x10;
            if (tens == 0){tens = 0x10;}
        }
    }
    segment_data[4] = tous;
    segment_data[3] = huns;
    segment_data[1] = tens;
    segment_data[0] = ones;
}

void decode_enc(uint8_t encoders, uint8_t jump){
    static uint8_t encA_state;
    static uint8_t encA_prev;
    static uint8_t encB_state;
    static uint8_t encB_prev;
    static uint8_t encA_cw;
    static uint8_t encA_ccw;
    static uint8_t encB_cw;
    static uint8_t encB_ccw;
    
    // Check the bit pattern for the two encoders to determine
    // what state both encoders are in. 
        //Check for bit 1 set
        if (encoders & 0x1){
            if(encoders & 0x2){encA_state=0x3;}
            if(!(encoders & 0x2)){encA_state=0x1;}
        }
        // Check for bit 1 clear
        if (!(encoders & 0x1)){
            if(encoders & 0x2){encA_state=0x2;}
            if(!(encoders & 0x2)){encA_state=0x0;}
        }
        // Check for bit 3 set
        if (encoders & 0x4){
            if(encoders & 0x8){encB_state=0x3;}
            if(!(encoders & 0x8)){encB_state=0x1;}
        }
        // Check for bit 3 clear
        if (!(encoders & 0x4)){
            if(encoders & 0x8){encB_state=0x2;}
            if(!(encoders & 0x8)){encB_state=0x0;}
        }

    // Use the state information to determine whether the encoder
    // is rotating clockwise or counter-clockwise. 
        switch (encA_state){
            case 0x0:
                if (encA_prev == 0x1){encA_ccw++;}
                if (encA_prev == 0x2){encA_cw++;}
                encA_prev = 0x0;
                break;
            case 0x1:
                if (encA_prev == 0x0){encA_cw++;}
                if (encA_prev == 0x3){encA_ccw++;}
                encA_prev = 0x1;
                break;
            case 0x2:
                if (encA_prev == 0x3){encA_cw++;}
                if (encA_prev == 0x0){encA_ccw++;}
                encA_prev = 0x2;
                break;
            case 0x3:
                if (encA_prev == 0x1){encA_cw++;}
                if (encA_prev == 0x2){encA_ccw++;}
                encA_prev = 0x3;
                break;
        }
        
    // Use the state information to determine whether the encoder
    // is rotating clockwise or counter-clockwise. 
        switch (encB_state){
            case 0x0:
                if (encB_prev == 0x1){encB_ccw++;}
                if (encB_prev == 0x2){encB_cw++;}
                encB_prev = 0x0;
                break;
            case 0x1:
                if (encB_prev == 0x0){encB_cw++;}
                if (encB_prev == 0x3){encB_ccw++;}
                encB_prev = 0x1;
                break;
            case 0x2:
                if (encB_prev == 0x3){encB_cw++;}
                if (encB_prev == 0x0){encB_ccw++;}
                encB_prev = 0x2;
                break;
            case 0x3:
                if (encB_prev == 0x1){encB_cw++;}
                if (encB_prev == 0x2){encB_ccw++;}
                encB_prev = 0x3;
                break;
        }

    // Adjust the number of steps for each encoder to coincide with 
    // the positive retention in the encoder's rotation.
        if (encA_cw >= 0x4){
            count+=jump;
            if (count > 1023){count -= 1024;}
            encA_cw = 0;
        }
        if (encA_ccw >= 0x4){
            count-=jump;
            if (count > 1023){count -= 1024;}
            encA_ccw = 0;
        }
        if (encB_cw >= 0x4){
            count+=jump;
            if (count > 1023){count -= 1024;}
            encB_cw = 0;
        }
        if (encB_ccw >= 0x4){
            count-=jump;
            if (count > 1023){count -= 1024;}
            encB_ccw = 0;
        }

    // Ensure the count never goes above 1023
        count = count & 0x03FF;
}

uint8_t main() {
    spi_init();
    DDRB |= (1<<PB7) | (1<<PB6) | (1<<PB5) | (1<<PB4);
    DDRC |= (1<<PC2) | (1<<PC1) | (1<<PC0);
    PORTB &= ~(1<<PB7);
    uint8_t encoders;
    uint8_t mode = 0x00;
    uint8_t modifier;

    while(1){

        DDRA = 0x00;
        PORTA = 0xFF;
        PORTC &= ~(1<<PC0);
        _delay_us(10);

        // Use button 0 to toggle +2 mode
        if (chk_buttons(0)){mode ^= 0xF0;}

        // Use button 1 to toggle +4 mode
        if (chk_buttons(1)){mode ^= 0x0F;}

        // Shortcut for resetting the count
        if (chk_buttons(7)){count = 0;}

        // Shortcut for getting to the other end 
        // of the count
        if (chk_buttons(6)){count = 1020;}

        switch(mode){
            case 0x00:
                modifier = 0x01;
                break;
            case 0x0F:
                modifier = 0x04;
                break;
            case 0xF0:
                modifier = 0x02;
                break;
            case 0xFF:
                modifier = 0x00;
                break;
        }

        // Disable pushbuttons
        PORTC |= (1<<PC0);


        // Send the mode indicator to the bar graph and poll the encoders
        encoders = serial_update(mode);

        // Decode the encoder pattern into clockwise or counter-clockwise
        // rotation and modify the count accordingly
        decode_enc(encoders,modifier);

        // Decode the count into base-10 numbers and prepare them for display
        // on the 7-segment display
        segsum(count);

        // Set PORTA to output
        DDRA = 0xFF;

        // Select Right-most digit
        PORTB &= ~((1<<PB6)|(1<<PB5)|(1<<PB4));
        PORTA = seg_digits[segment_data[0]];
        _delay_us(SHOW_TIME);

        // Select Second to right digit
        PORTB |= (1<<PB4);
        PORTA = seg_digits[segment_data[1]];
        _delay_us(SHOW_TIME);

        // Select second to left digit
        PORTB |= (1<<PB5) | (1<<PB4);
        PORTA = seg_digits[segment_data[3]];
        _delay_us(SHOW_TIME);

        // Select left-most digit
        PORTB &= ~((1<<PB5)|(1<<PB4));
        PORTB |= (1<<PB6);
        PORTA = seg_digits[segment_data[4]];
        _delay_us(SHOW_TIME);

        PORTB |= (1<<PB6) | (1<<PB5) | (1<<PB4);
    }
}
