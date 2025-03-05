#ifndef SERIALCOM_H_
#define SERIALCOM_H_

#include "TinyTimber.h"
#include <avr/io.h>

typedef struct {
	Object super;
} SerialCom;


#define initSerialCom() { initObject()}
	
void USARTreceiver(SerialCom *self, int arg);

#endif /* SERIALCOM_H_ */