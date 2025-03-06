#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "Tinytimber.h"


typedef struct {
	Object super;
	GUI *gui;
	uint64_t NorthQueue;
	uint64_t SouthQueue;
	uint64_t Bridge;
	unsigned char SensorStatus;
	unsigned char LightStatus;
	} Controller;

#define initController() { initObject(), gui, 0, 0, 0 }

#endif /* CONTROLLER_H_ */