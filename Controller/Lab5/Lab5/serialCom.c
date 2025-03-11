#include "serialCom.h"

int USARTtransmit(SerialCom *self, int arg) {
  // Vänta tills send buffern är tom
  while (!(UCSR0A & (1 << UDRE0))) {
  }
  // Skriv byten till UDR0 för sändning
  UDR0 = (uint8_t)arg;
  return 0;
}

int USARTreceive(SerialCom *self, int arg) {
  // Vänta tills data finns att läsa
  while (!(UCSR0A & (1 << RXC0))) {
  }

  int bits = UDR0;
  return bits;
}
