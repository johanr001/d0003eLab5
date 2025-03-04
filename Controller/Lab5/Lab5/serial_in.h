#ifndef SERIAL_IN_H_
#define SERIAL_IN_H_

#include "TinyTimber.h"
#include <avr/io.h>

typedef struct {
	Object super;
} SerialIn;


#define initSerialIn() { initObject()}


#endif /* SERIAL_IN_H_ */