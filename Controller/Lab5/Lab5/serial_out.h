#ifndef SERIAL_OUT_H_
#define SERIAL_OUT_H_

#include "TinyTimber.h"
#include <avr/io.h>

typedef struct {
	Object super;
} SerialOut;


#define initSerialOut() { initObject()}


#endif /* SERIAL_OUT_H_ */