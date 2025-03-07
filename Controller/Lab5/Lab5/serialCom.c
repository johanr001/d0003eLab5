#include "serialCom.h"

int USARTreceiver(SerialCom *self, int arg){
	while ( !(UCSR0A & (1<<RXC0)) );
	return UDR
}

int USARTtransmit(SerialCom *self, int arg){
	while ( !( UCSR0A & (1<<UDRE0)) );
	UDR = arg;
}