#ifndef SERIAL_H
#define SERIAL_H

#include "TinyTimber.h"
#include <avr/io.h>

typedef struct {
  Object super;
} SerialCom;

#define initSerialCom() {initObject()}

int USARTtransmit(SerialCom *self, int arg);
int USARTreceive(SerialCom *self, int arg);

#endif
