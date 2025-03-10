#include "Controller.h"

// Bitparser tolkar data från USART och uppdaterar queue och bron.
int bitParser(Controller *self, int arg) {
	int data = SYNC(self->serialCom, USARTreceive, 0); // Hämtar datan från USART:en.
	// Om en bil kommer från NORTH, öka NORTH.
	if (data & NORTH_ARRIVAL) {
		self->NorthQueue++;
	}
	// Om en bil kommer från SOUTH, öka SOUTH.
	if (data & SOUTH_ARRIVAL) {
		self->SouthQueue++;
	}
	// Om en bil kör in på bron från NORTH.
	if (data & NORTH_ENTRY) {
		self->NorthQueue--; // Minska NorthQueue.
		self->BridgeAmount++; // Öka bilar på bron.
		self->BridgePassedSameDir++; // Öka hur många bilar som har åkt i samma riktning.
		AFTER(CURRENT_OFFSET() + SEC(BRIDGE_TIME), self, updateBridgeAmount, -1); // Minska antal bilar på bron när den kört över. (Tidsbaserat).
	}
	// Om en bil kör in på bron från SOUTH.
	if (data & SOUTH_ENTRY) {
		self->SouthQueue--; // Minska SouthQueue
		self->BridgeAmount++; // Öka bilar på bron.
		self->BridgePassedSameDir++; // Öka hur många bilar som har åkt i samma riktning.
		AFTER(CURRENT_OFFSET() + SEC(BRIDGE_TIME), self,updateBridgeAmount, -1); // Minska antal bilar på bron när den kört över. (Tidsbaserat).
	}

	return 0;
}
// updateBridgeAmount så man can calla med AFTER i bitParsern.
int updateBridgeAmount(Controller *self, int arg) {
	self->BridgeAmount += arg;
	return 0;
}
//idleState hanterar bron när den är tom, och bestämmer vilken bil som ska passera
int idleState(Controller *self, int arg) {
	self->BridgePassedSameDir = 0; // Nollställ.
	// Bestämmer vilken queue som ska prioriteras baserat på lastBridgeDir.
	int *priorityQueue = self->lastBridgeDir ? &self->SouthQueue : &self->NorthQueue;
	int *secondaryQueue = self->lastBridgeDir ? &self->NorthQueue : &self->SouthQueue;
	// Om den prioriterade queuen har bilar, ändra riktning och skicka nästa bil.
	if (*priorityQueue > 0) {
		self->lastBridgeDir = !self->lastBridgeDir;
		ASYNC(self, dispatchNextCar, 0);
		} 
	// Om den andra kön har bilar, skicka nästa bil.
	else if (*secondaryQueue > 0) {
		ASYNC(self, dispatchNextCar, 0);
		} 
	// Om inga bilar finns i någon kö, loopa igen.
	else {
		AFTER(MSEC(IDLE_DELAY_MSEC), self, idleState, 0);
	}
	return 0;
}
// Väntar tills bron är tom innan en ny bil kan skickas.
int waitForBridgeClearance(Controller *self, int arg) {
	if (self->BridgeAmount == 0) { // Om bron är tom, gå till idle.
		ASYNC(self, idleState, 0);
		} 
	else {
		AFTER(CURRENT_OFFSET(), self, waitForBridgeClearance, 0); // Kontrollera igen senare.
	}
	return 0;
}
// dispatchNextCar avgör om nästa bil kan skickas ut på bron.
int dispatchNextCar(Controller *self, int arg) {
	int *currentQueue = self->lastBridgeDir ? &self->NorthQueue : &self->SouthQueue;
	int *oppositeQueue = self->lastBridgeDir ? &self->SouthQueue : &self->NorthQueue;
	// Om kön är tom eller för många bilar passerat i samma riktning, vänta på att bron blir tom.
	if (!(*currentQueue) || (self->BridgePassedSameDir >= MAX_PASS_SAME_SIDE && *oppositeQueue > 0)) {
		ASYNC(self, waitForBridgeClearance, 0);
		return 0;
	}
	// Skicka signal att tända grönt för nästa bil.
	ASYNC(self, signalGreenLight, 0);
	return 0;
}
// singnalGreenLight tänder ljuset åt rätt riktning.
int signalGreenLight(Controller *self, int arg) {
	int bits = self->lastBridgeDir ? NORTH_GREEN : SOUTH_GREEN;
	ASYNC(self->serialCom, USARTtransmit, bits); // Skicka signal för rätt ljus.

	// Vänta en kort stund innan bilen får köra in.
	AFTER(CURRENT_OFFSET() + MSEC(GREEN_LIGHT_TIME_MSEC), self, monitorCarEntry, 0);
	return 0;
}
// monitorCarEntry ser till att en bil kör in på bron efter att ha fått grönt.
int monitorCarEntry(Controller *self, int arg) {
	if (self->BridgeAmount > 0) {  // Om en bil har kört in på bron.
		int bits = self->lastBridgeDir ? NORTH_RED : SOUTH_RED;
		ASYNC(self->serialCom, USARTtransmit, bits); // Tänd rött ljus för att stoppa nästa bil.
		AFTER(CURRENT_OFFSET() + SEC(TIME_QUEUE), self, dispatchNextCar, 0); // Vänta innan nästa bil skickas.
		} else {
		AFTER(CURRENT_OFFSET(), self, monitorCarEntry, 0); // Kontrollera igen om ingen bil har kört in.
	}
	return 0;
}

int getNorthQueue(Controller *self, int arg) {
	return self->NorthQueue;
}

int getSouthQueue(Controller *self, int arg) {
	return self->SouthQueue;
}

int getBridgeAmount(Controller *self, int arg) {
	return self->BridgeAmount;
}