#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "Tinytimber.h"

typedef struct {
	Object super;
	GUI *gui;
	SerialCom *serialcom;
	uint64_t NorthQueue; // Hur många bilar finns i NorthQueue?
	uint64_t SouthQueue; // Hur många bilar finns i SouthQueue?
	uint64_t BridgeAmount; // Hur många bilar finns på bron just nu?
	unsigned char SensorStatus; // Bitsen för vilka sensorer som är på.
	unsigned char LightStatus; // Bitsen för vilka lampor
	uint64_t BridgePassedSameDir; // Resetta och checka hur många bilar som passerat bridge, för starvation. 
	bool lastBridgeDir; // Om 1 var north den bridge arrival, om 0 var south den sista på bridge.
	bool lastQueueSensor; // Sista hållet sensorn var från bil kön. 1 för north och 0 för south.
	} Controller;

#define initController(gui) { initObject(), gui, 0, 0, 0 }

#define NORTHARRVIAL 0b0001
#define NORTHBRIARR  0b0010
#define SOUTHARRIVAl 0b0100
#define SOUTHBRIARR  0b1000

#define NORTHGREEN 0b0001
#define NORTHRED   0b0010
#define SOUTHGREEN 0b0100
#define SOUTHRED   0b1000

#define NORTHGREENSOUTHGREEN 0b0101
#define NORTHGREENSOUTHRED   0b1001
#define NORTHREDSOUTHGREEN   0b0110
#define NORTHREDSOUTHRED     0b1010

#define BRIDGETIME 5000
#define maxStarvation 10

int getNorthQueue(Controller *self, int arg);
int getSouthQueue(Controller *self, int arg);
int getBridgeQueue(Controller *self, int arg);

int LampController(Controller *self, int arg);

#endif /* CONTROLLER_H_ */