#include "Controller.h"

// Bitparser tolkar data fr�n USART och uppdaterar queue och bron.
int bitParser(Controller *self, int arg) {
	int data = SYNC(self->serialCom, USARTreceive, 0); // H�mtar datan fr�n USART:en.
	// Om en bil kommer fr�n NORTH, �ka NORTH.
	if (data & NORTH_ARRIVAL) {
		self->NorthQueue++;
	}
	// Om en bil kommer fr�n SOUTH, �ka SOUTH.
	if (data & SOUTH_ARRIVAL) {
		self->SouthQueue++;
	}
	// Om en bil k�r in p� bron fr�n NORTH.
	if (data & NORTH_ENTRY) {
		self->NorthQueue--; // Minska NorthQueue.
		self->BridgeAmount++; // �ka bilar p� bron.
		self->BridgePassedSameDir++; // �ka hur m�nga bilar som har �kt i samma riktning.
		AFTER(CURRENT_OFFSET() + SEC(BRIDGE_TIME), self, updateBridgeAmount, -1); // Minska antal bilar p� bron n�r den k�rt �ver. (Tidsbaserat).
	}
	// Om en bil k�r in p� bron fr�n SOUTH.
	if (data & SOUTH_ENTRY) {
		self->SouthQueue--; // Minska SouthQueue
		self->BridgeAmount++; // �ka bilar p� bron.
		self->BridgePassedSameDir++; // �ka hur m�nga bilar som har �kt i samma riktning.
		AFTER(CURRENT_OFFSET() + SEC(BRIDGE_TIME), self,updateBridgeAmount, -1); // Minska antal bilar p� bron n�r den k�rt �ver. (Tidsbaserat).
	}

	return 0;
}
// updateBridgeAmount s� man can calla med AFTER i bitParsern.
int updateBridgeAmount(Controller *self, int arg) {
	self->BridgeAmount += arg;
	return 0;
}
//idleState hanterar bron n�r den �r tom, och best�mmer vilken bil som ska passera
int idleState(Controller *self, int arg) {
	self->BridgePassedSameDir = 0; // Nollst�ll.
	// Best�mmer vilken queue som ska prioriteras baserat p� lastBridgeDir.
	int *priorityQueue = self->lastBridgeDir ? &self->SouthQueue : &self->NorthQueue;
	int *secondaryQueue = self->lastBridgeDir ? &self->NorthQueue : &self->SouthQueue;
	// Om den prioriterade queuen har bilar, �ndra riktning och skicka n�sta bil.
	if (*priorityQueue > 0) {
		self->lastBridgeDir = !self->lastBridgeDir;
		ASYNC(self, dispatchNextCar, 0);
		} 
	// Om den andra k�n har bilar, skicka n�sta bil.
	else if (*secondaryQueue > 0) {
		ASYNC(self, dispatchNextCar, 0);
		} 
	// Om inga bilar finns i n�gon k�, loopa igen.
	else {
		AFTER(MSEC(IDLE_DELAY_MSEC), self, idleState, 0);
	}
	return 0;
}
// V�ntar tills bron �r tom innan en ny bil kan skickas.
int waitForBridgeClearance(Controller *self, int arg) {
	if (self->BridgeAmount == 0) { // Om bron �r tom, g� till idle.
		ASYNC(self, idleState, 0);
		} 
	else {
		AFTER(CURRENT_OFFSET(), self, waitForBridgeClearance, 0); // Kontrollera igen senare.
	}
	return 0;
}
// dispatchNextCar avg�r om n�sta bil kan skickas ut p� bron.
int dispatchNextCar(Controller *self, int arg) {
	int *currentQueue = self->lastBridgeDir ? &self->NorthQueue : &self->SouthQueue;
	int *oppositeQueue = self->lastBridgeDir ? &self->SouthQueue : &self->NorthQueue;
	// Om k�n �r tom eller f�r m�nga bilar passerat i samma riktning, v�nta p� att bron blir tom.
	if (!(*currentQueue) || (self->BridgePassedSameDir >= MAX_PASS_SAME_SIDE && *oppositeQueue > 0)) {
		ASYNC(self, waitForBridgeClearance, 0);
		return 0;
	}
	// Skicka signal att t�nda gr�nt f�r n�sta bil.
	ASYNC(self, signalGreenLight, 0);
	return 0;
}
// singnalGreenLight t�nder ljuset �t r�tt riktning.
int signalGreenLight(Controller *self, int arg) {
	int bits = self->lastBridgeDir ? NORTH_GREEN : SOUTH_GREEN;
	ASYNC(self->serialCom, USARTtransmit, bits); // Skicka signal f�r r�tt ljus.

	// V�nta en kort stund innan bilen f�r k�ra in.
	AFTER(CURRENT_OFFSET() + MSEC(GREEN_LIGHT_TIME_MSEC), self, monitorCarEntry, 0);
	return 0;
}
// monitorCarEntry ser till att en bil k�r in p� bron efter att ha f�tt gr�nt.
int monitorCarEntry(Controller *self, int arg) {
	if (self->BridgeAmount > 0) {  // Om en bil har k�rt in p� bron.
		int bits = self->lastBridgeDir ? NORTH_RED : SOUTH_RED;
		ASYNC(self->serialCom, USARTtransmit, bits); // T�nd r�tt ljus f�r att stoppa n�sta bil.
		AFTER(CURRENT_OFFSET() + SEC(TIME_QUEUE), self, dispatchNextCar, 0); // V�nta innan n�sta bil skickas.
		} else {
		AFTER(CURRENT_OFFSET(), self, monitorCarEntry, 0); // Kontrollera igen om ingen bil har k�rt in.
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