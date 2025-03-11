#include "serialCom.h"

int USARTtransmit(SerialCom *self, int arg) {
  // V�nta tills send buffern �r tom
  while (!(UCSR0A & (1 << UDRE0))) {
  }
  // Skriv byten till UDR0 f�r s�ndning
  UDR0 = (uint8_t)arg;
  return 0;
}

int USARTreceive(SerialCom *self, int arg) {
  // V�nta tills data finns att l�sa
  while (!(UCSR0A & (1 << RXC0))) {
  }

  int bits = UDR0;
  return bits;
}
