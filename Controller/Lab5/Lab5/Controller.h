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


#define MAX_PASS_SAME_SIDE 4 // Starvation gräns.

// Controller-struktur som håller reda på köer, broläge, lampstatus och referens till SerialCom
typedef struct {
	Object     super;
	SerialCom *serialCom;
	unsigned long NorthQueue; // Hur många bilar finns i NorthQueue?
	unsigned long SouthQueue; // Hur många bilar finns i SouthQueue?
	unsigned long BridgeAmount; // Hur många bilar finns på bron just nu?
	unsigned char LightStatus; // Bitsen för vilka lampor
	unsigned long BridgePassedSameDir; // Resetta och checka hur många bilar som passerat bridge, för starvation.
	bool lastBridgeDir; // Om 1 var northbound den bridge arrival, om 0 var southbound den sista på bridge.
	bool lastQueueSensor; // Sista hållet sensorn var från bil kön. 1 för north och 0 för south.
} Controller;

// Konstruktor-makro för att initiera Controller
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
