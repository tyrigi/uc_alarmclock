#define F_CPU 16000000
#define TRUE 1
#define FALSE 0
#define DEBOUNCE_TIME 50
#define SHOW_TIME 2
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

char lcd_str[16];

uint8_t chk_buttons(uint8_t button) {
    // check requested button's state. Use debouncing.
    static uint16_t state[8];
    state[button] = (state[button] << 1) | (! bit_is_clear(PINA, button)) | 0xE000;
//    update(state[button]);
    if (state[button] == 0xF000) return 1;
    return 0;
}

uint8_t chk_btn0(uint8_t button) {
    static uint16_t btn_state = 0;
    btn_state = (btn_state << 1) | (! bit_is_clear(PINA, button)) | 0xE000;
//    update(btn_state);
    if (btn_state == 0xF000) return 1;
    return 0;
}

void strobe_lcd(void){
    PORTF |= 0x08;
    PORTF &= ~0x08;
}

void clear_display(void){
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	SPDR = 0x00;    //command, not data
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	SPDR = 0x01;    //clear display command
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	strobe_lcd();   //strobe the LCD enable pin
	_delay_ms(1.6);   //obligatory waiting for slow LCD
}

void home_line2(void){
	SPDR = 0x00;    //command, not data
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	SPDR = 0xC0;   // cursor go home on line 2
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	strobe_lcd(); 
	_delay_us(37);
}

void char2lcd(char a_char){
 	//sends a char to the LCD
	//usage: char2lcd('H');  // send an H to the LCD
	SPDR = 0x01;   //set SR for data xfer with LSB=1
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	SPDR = a_char; //send the char to the SPI port
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	strobe_lcd();  //toggle the enable bit
	_delay_us(37);
}

void string2lcd(char *lcd_str){
	int count;
	for (count=0; count<=(strlen(lcd_str)-1); count++){
		while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
		SPDR = 0x01; //set SR for data
		while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
		SPDR = lcd_str[count]; 
		while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
		strobe_lcd();
		_delay_us(37);	// Max delay for this function is 48us
	}
} 

void spi_init(void){
	DDRF |= 0x08;  //port F bit 3 is enable for LCD
	PORTB |= 0x00; //port B initalization for SPI
	DDRB |= 0x07;  //Turn on SS, MOSI, SCLK 
	//Master mode, Clock=clk/2, Cycle half phase, Low polarity, MSB first  
	SPCR = 0x50;
	SPSR = 0x01;
}

void lcd_init(void){
	int i;
	//initalize the LCD to receive data
	_delay_ms(15);   
	for(i=0; i<=2; i++){ //do funky initalize sequence 3 times
		SPDR = 0x00;
		while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
		SPDR = 0x30;
		while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
		strobe_lcd();
		_delay_us(37);
	}

	SPDR = 0x00;
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	SPDR = 0x38;
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	strobe_lcd();
	_delay_us(37);

	SPDR = 0x00;
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	SPDR = 0x08;
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	strobe_lcd();
	_delay_us(37);

	SPDR = 0x00;
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	SPDR = 0x01;
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	strobe_lcd();
	_delay_ms(1.6);

	SPDR = 0x00;
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	SPDR = 0x06;
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	strobe_lcd();
	_delay_us(37);

	SPDR = 0x00;
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	SPDR = 0x0E;
	while (!(SPSR & 0x80)) {}	// Wait for SPI transfer to complete
	strobe_lcd();
	_delay_us(37);
}

void update(uint8_t val){
    char buffer[16];
    spi_init();
    lcd_init();
    clear_display();
    itoa(val, buffer, 16);
    string2lcd(buffer);

    SPCR = 0x00;
    DDRB = 0xFF;
}

uint8_t main() {
    DDRC = 0b11111111;
    uint16_t count = 0;
    uint8_t debounce_counter = 0;
    // set Port A to input
    DDRA = 0b00000000;
    // Activate pullup resistors
    PORTA = 0b11111111;

    DDRB = 0b11111111;
    PORTB = 0b11111111;
    
    _delay_ms(1000);

    while(1){
        PORTC &= ~(1<<PC0);
        
        // check all buttons
        for (uint8_t x=0; x<8; x++){
            // if button is pressed, add the defined number to the total count
            if (chk_buttons(x)) {count += 0x1;}//<< x;}
            // increment sum, deal with rollover
            if (count == 1024) {count = 1;}
        }

        //if (chk_buttons(0)) {count++;}
        //if (chk_btn0(0)) {count++;}

        PORTB = count;

        // disable pushbuttons
        PORTC |= (1<<PC0);
        // convert sum to display codes

        // set Port A to output
        // flash all 4 digits with their appropriate number
        _delay_ms(SHOW_TIME);
        _delay_ms(SHOW_TIME);
        _delay_ms(SHOW_TIME);
        _delay_ms(SHOW_TIME);
        //_delay_ms(500);

    }
}









