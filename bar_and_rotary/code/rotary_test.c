#define F_CPU 16000000
#define SHOW_TIME 500//0000 //500

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint16_t count;

uint8_t segment_data[5] = {
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
};

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

void tcnt0_init(void){
    ASSR |= (1<<AS0);
    TIMSK |= (1<<TOIE0);
    TCCR0 |= (1<<CS00);
}

uint8_t spi_read(void){
    PORTC &= ~(1<<PC1);
    _delay_us(10);
    PORTC |= (1<<PC1);
    SPDR = 0x00;
    while (bit_is_clear(SPSR,SPIF)){}
    return(SPDR);
}

uint8_t spi_write(uint8_t value){
    SPDR = value;
    while (bit_is_clear(SPSR,SPIF)){}
    PORTB |= (1<<PB0);
    PORTB &= ~(1<<PB0);
    return(SPDR);
}

void seg_decode(uint16_t display_val){
    segment_data[0] = display_val & 0x000F;
    segment_data[1] = (display_val & 0x00F0)>>4;
    segment_data[3] = (display_val & 0x0F00)>>8;
    segment_data[4] = (display_val & 0xF000)>>12;
}

void decode_enc(uint8_t encoders){
    static uint8_t encA_state;
    static uint8_t encA_prev;
    static uint8_t encB_state;
    static uint8_t encB_prev;
    static uint8_t encA_cw;
    static uint8_t encA_ccw;
    static uint8_t encB_cw;
    static uint8_t encB_ccw;
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

        if (encA_cw >= 0x4){
            count++;
            encA_cw = 0;
        }
        if (encA_ccw >= 0x4){
            count--;
            encA_ccw = 0;
        }
        if (encB_cw >= 0x4){
            count++;
            encB_cw = 0;
        }
        if (encB_ccw >= 0x4){
            count--;
            encB_ccw = 0;
        }
}

ISR(TIMER0_OVF_vect){
    static uint8_t count_7ms = 0;
    static uint8_t display_count = 0x1;

    count_7ms++;
    if((count_7ms % 16)==0){
        SPDR = display_count;
        while (bit_is_clear(SPSR,SPIF)){}
        PORTB |= (1<<PB0);
        PORTB &= ~(1<<PB0);
        display_count = display_count<<1;
    }
    if(display_count == 0x08){display_count++;}
    if(display_count == 0x48){display_count++;}
    if(display_count == 0x00){display_count++;}
}

uint8_t main() {
    spi_init();
    tcnt0_init();
    DDRB |= (1<<PB7) | (1<<PB6) | (1<<PB5) | (1<<PB4);
    DDRC |= (1<<PC2) | (1<<PC1) | (1<<PC0);
    DDRA = 0b11111111;
    PORTB &= ~(1<<PB7);
    PORTC &= ~(1<<PC2);
    sei();
    uint8_t encoders;
    uint8_t disp_bytes[4];

    while(1){
        PORTC &= ~(1<<PC2);
        encoders = spi_read();
        PORTC |= (1<<PC2);
        decode_enc(encoders);

        seg_decode(count);

        // Select Right-most digit
        PORTB &= ~((1<<PB6)|(1<<PB5)|(1<<PB4));
        PORTA = seg_digits[segment_data[0]];
        //spi_write(segment_data[0]);
        _delay_us(SHOW_TIME);

        // Select Second to right digit
        PORTB |= (1<<PB4);
        PORTA = seg_digits[segment_data[1]];
        //spi_write(segment_data[1]);
        _delay_us(SHOW_TIME);

        // Select second to left digit
        PORTB |= (1<<PB5) | (1<<PB4);
        PORTA = seg_digits[segment_data[3]];
        //spi_write(segment_data[3]);
        _delay_us(SHOW_TIME);

        // Select left-most digit
        PORTB &= ~((1<<PB5)|(1<<PB4));
        PORTB |= (1<<PB6);
        PORTA = seg_digits[segment_data[4]];
        //spi_write(segment_data[4]);
        _delay_us(SHOW_TIME);

        PORTB |= (1<<PB6) | (1<<PB5) | (1<<PB4);
    }
}
