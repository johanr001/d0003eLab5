/*
 * interruptHandler.h
 *
 * Created: 04/03/2025 15:12:24
 *  Author: johan_csf2sgl
 */ 


#ifndef INTERRUPTHANDLER_H_
#define INTERRUPTHANDLER_H_

#include "TinyTimber.h"
#include "button.h"

typedef struct{

	Object super;

	
	
} Interrupthandler;

#define initInterruptHandler(button) { initObject(), button}

int horizontal(Interrupthandler *self, int arg);
int vertandcent(Interrupthandler *self, int arg);

#endif /* INTERRUPTHANDLER_H_ */