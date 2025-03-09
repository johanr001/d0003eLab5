#include "serialCom.h"

// USARTtransmit: Skickar en byte via USART
int USARTtransmit(SerialCom *self, int arg) {
	// V�nta tills send buffern �r tom
	while (!(UCSR0A & (1 << UDRE0))) {
	}
	// Skriv byten till UDR0 f�r s�ndning
	UDR0 = (uint8_t) arg;
	return 0;
}
