#ifndef INTERRUPTHANDLER_H_
#define INTERRUPTHANDLER_H_

#include "TinyTimber.h"
#include "Controller.h"
#include <avr/io.h>

typedef struct {
	Object     super;
	Controller *controller;
} Interrupthandler;

#define initInterruptHandler(controller) { initObject(), controller }

int interruptreceiver(Interrupthandler *self, int arg);

#endif /* INTERRUPTHANDLER_H_ */
