#include "serialCom.h"

// USARTtransmit: Skickar en byte via USART
int USARTtransmit(SerialCom *self, int arg) {
	// Vänta tills send buffern är tom
	while (!(UCSR0A & (1 << UDRE0))) {
	}
	// Skriv byten till UDR0 för sändning
	UDR0 = (uint8_t) arg;
	return 0;
}
