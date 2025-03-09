#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "serialCom.h"
#include "TinyTimber.h"
#include <avr/io.h>
#include <stdbool.h>

#define NORTH_ARRIVAL  0b00000001
#define NORTH_ENTRY    0b00000010
#define SOUTH_ARRIVAL  0b00000100
#define SOUTH_ENTRY    0b00001000

#define NORTH_GREEN    0b00000001
#define NORTH_RED      0b00000010
#define SOUTH_GREEN    0b00000100
#define SOUTH_RED      0b00001000

#define NORTHRED_SOUTHRED     (NORTH_RED   | SOUTH_RED)
#define NORTHGREEN_SOUTHRED   (NORTH_GREEN | SOUTH_RED)
#define NORTHRED_SOUTHGREEN   (NORTH_RED   | SOUTH_GREEN)

#define BRIDGE_TIME_MS   5000  // 5 sekunders bro tid.
#define ALL_RED_GAP_MS   1000  // 1 sekunds all-red gap


#define MAX_PASS_SAME_SIDE 4 // Starvation gr�ns.

// Controller-struktur som h�ller reda p� k�er, brol�ge, lampstatus och referens till SerialCom
typedef struct {
	Object     super;
	SerialCom *serialCom;
	unsigned long NorthQueue; // Hur m�nga bilar finns i NorthQueue?
	unsigned long SouthQueue; // Hur m�nga bilar finns i SouthQueue?
	unsigned long BridgeAmount; // Hur m�nga bilar finns p� bron just nu?
	unsigned char LightStatus; // Bitsen f�r vilka lampor
	unsigned long BridgePassedSameDir; // Resetta och checka hur m�nga bilar som passerat bridge, f�r starvation.
	bool lastBridgeDir; // Om 1 var northbound den bridge arrival, om 0 var southbound den sista p� bridge.
	bool lastQueueSensor; // Sista h�llet sensorn var fr�n bil k�n. 1 f�r north och 0 f�r south.
} Controller;

// Konstruktor-makro f�r att initiera Controller
#define initController(s) { initObject(), s, 0, 0, 0, NORTHRED_SOUTHRED, 0, 0, 0 }

void bitParser(Controller *self, int arg);

int sensorEvent(Controller *self, int arg);
int getNorthQueue(Controller *self, int arg);
int getSouthQueue(Controller *self, int arg);
int getBridgeAmount(Controller *self, int arg);

int LampController(Controller *self, int arg);
int carLeavesBridge(Controller *self, int arg);
int afterRedGap(Controller *self, int arg);

#endif /* CONTROLLER_H_ */
