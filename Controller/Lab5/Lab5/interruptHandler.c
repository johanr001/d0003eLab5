
#include <avr/io.h>

#include "TinyTimber.h"

#include "interruptHandler.h"

void interruptreceiver(Interrupthandler *self, int arg) {
	ASYNC(self->serialCom, USARTreciever, 0);
	return 0;
}
