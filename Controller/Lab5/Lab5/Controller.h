#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "TinyTimber.h"
#include "serialCom.h"
#include "GUI.h"
#include <stdbool.h>

#define NORTH_ARRIVAL 0b00000001
#define NORTH_ENTRY 0b00000010
#define SOUTH_ARRIVAL 0b00000100
#define SOUTH_ENTRY 0b00001000

#define NORTH_GREEN 0b00000001
#define NORTH_RED 0b00000010
#define SOUTH_GREEN 0b00000100
#define SOUTH_RED 0b00001000

#define IDLE_DELAY_MSEC 500 // Tiden f�r idle att loopa.

#define GREEN_LIGHT_TIME_MSEC                                                  \
500 // L�t gr�nt ljus vara p� en liten stund s� att man faktiskt ser den
// �ndras.

#define MAX_PASS_SAME_SIDE 10 // Starvation gr�ns.

#define TIME_QUEUE                                                             \
1 // Tiden det tar f�r bilar att queuea p� bron efter varandra.
#define BRIDGE_TIME 5 // Tiden det tar f�r bilar att �ka �ver bron.

struct GUI; // Forward declaration

typedef struct {
	Object super;
	SerialCom *serialCom;
    struct GUI *gui; // Anv�nd forward declared struct, d� kan man passa pointer.
  unsigned long NorthQueue;   // Antalet bilar som finns i NorthBound k�n.
  unsigned long SouthQueue;   // Antalet bilar som finns i SouthBound k�n.
  unsigned long BridgeAmount; // Antalet bilar p� bron just nu.
  unsigned long BridgePassedSameDir; // Resetta och checka hur m�nga bilar som
  // passerat bridge, f�r starvation.
  bool lastBridgeDir; // Om 1 var northbound den bridge arrival, om 0 var
  // southbound den sista p� bridge.
  } Controller;

#define initController(serialCom, gui) {initObject(), serialCom, gui, 0, 0, 0, 0, 0}

int bitParser(Controller *self, int arg);
int removeFromBridge(Controller *self, int arg);
int idleState(Controller *self, int arg);
int waitForBridgeClearance(Controller *self, int arg);
int dispatchNextCar(Controller *self, int arg);
int signalGreenLight(Controller *self, int arg);
int monitorCarEntry(Controller *self, int arg);

int getNorthQueue(Controller *self, int arg);
int getBridgeAmount(Controller *self, int arg);
int getSouthQueue(Controller *self, int arg);

#endif