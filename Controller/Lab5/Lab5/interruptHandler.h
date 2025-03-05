#ifndef INTERRUPTHANDLER_H_
#define INTERRUPTHANDLER_H_

#include "TinyTimber.h"
#include "serialCom.h"

typedef struct{

	Object super;

	SerialCom *serialCom;	
	
} Interrupthandler;

#define initInterruptHandler(serialCom) { initObject(), serialCom}

void interruptreceiver(Interrupthandler *self, int arg);

#endif /* INTERRUPTHANDLER_H_ */