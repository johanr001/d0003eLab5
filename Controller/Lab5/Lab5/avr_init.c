#include "avr_init.h"
#include <avr/io.h>
#define FOSC 8000000// Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

// avr_init() initierar klockan, timer1, prescalers etc.
void avr_init(){
	CLKPR = 0x80;
	CLKPR = 0x00;

	
	TCCR1B |= (1 << CS10) | (1 << CS12) | (1 << WGM12);
	
	TCNT1 = 0;
	
	TIMSK1 |= (1 << OCIE1A);
	OCR1A=194; //f_oCnA = (f_clk_I/O)/(2*N*(1+OCRnA) 194 eller 3905 för en sekund
	
	UBRR0H |= (MYUBRR>>8);
	UBRR0L |= MYUBRR;
	// Reciever, transmitter, och interrupt.
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	// 8 Data, 1 stop bit
	UCSR0C |= (1 << UCSZ01) | (3 << UCSZ00);
	
	
};
