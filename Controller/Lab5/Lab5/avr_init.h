#ifndef AVR_INIT_H_
#define AVR_INIT_H_

#include <avr/io.h>

#define FOSC 8000000UL // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

void avr_init(void);

#endif /* AVR_INIT_H_ */