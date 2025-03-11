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

#define IDLE_DELAY_MSEC 500 // Tiden för idle att loopa.

#define GREEN_LIGHT_TIME_MSEC                                                  \
500 // Låt grönt ljus vara på en liten stund så att man faktiskt ser den
// ändras.

#define MAX_PASS_SAME_SIDE 10 // Starvation gräns.

#define TIME_QUEUE                                                             \
1 // Tiden det tar för bilar att queuea på bron efter varandra.
#define BRIDGE_TIME 5 // Tiden det tar för bilar att åka över bron.

struct GUI; // Forward declaration

typedef struct {
	Object super;
	SerialCom *serialCom;
    struct GUI *gui; // Använd forward declared struct, då kan man passa pointer.
  unsigned long NorthQueue;   // Antalet bilar som finns i NorthBound kön.
  unsigned long SouthQueue;   // Antalet bilar som finns i SouthBound kön.
  unsigned long BridgeAmount; // Antalet bilar på bron just nu.
  unsigned long BridgePassedSameDir; // Resetta och checka hur många bilar som
  // passerat bridge, för starvation.
  bool lastBridgeDir; // Om 1 var northbound den bridge arrival, om 0 var
  // southbound den sista på bridge.
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