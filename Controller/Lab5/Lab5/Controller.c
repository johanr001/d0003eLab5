#include "Controller.h"

// TODO: FIXA LOGIKEN FÖR TRAFFICLIGHTCONTROLLER OCH BITPARSERN.


// trafficLightController: Bestämmer vilken sida som blir grön/röd utifrån aktuell kö och hur många bilar som finns på bron.
void trafficLightController(Controller *self, int arg)
{
	// Om det redan är bilar på bron och vi precis forcade rött, vänta på afterRedGap
	if (self->BridgeAmount > 0) {
		// Om vi är i "all-red gap", så avvaktar vi tills afterRedGap kallar funktionen
		// Om det redan är grönt, gör inget
		return;
	}

	unsigned long NQ = self->NorthQueue;
	unsigned long SQ = self->SouthQueue;
	unsigned char newLights = NORTHRED_SOUTHRED;  // Standard: rött för båda

	// Inga bilar i någon kö -> båda röda
	if (NQ == 0 && SQ == 0) {
		newLights = NORTHRED_SOUTHRED;
	}
	// Endast bilar i northbound
	else if (NQ > 0 && SQ == 0) {
		newLights = NORTHGREEN_SOUTHRED;
	}
	// Endast bilar i southbound
	else if (SQ > 0 && NQ == 0) {
		newLights = NORTHRED_SOUTHGREEN;
	}
	// Båda sidor har bilar i kö
	else {
		newLights = self->lastQueueSensor ? NORTHGREEN_SOUTHRED :  NORTHRED_SOUTHGREEN;
		}
	}

	// Om de nya lampinställningarna skiljer sig, uppdatera och skicka via serial.
	if (newLights != self->LightStatus) {
		self->LightStatus = newLights;
		ASYNC(self->serialCom, USARTtransmit, newLights);
	}
}

// carLeavesBridge: AFTER 5 sekunder efter att en bil kör in.
int carLeavesBridge(Controller *self, int arg) {
	// Minskar antalet bilar på bron om det är minst 1
	if (self->BridgeAmount > 0) {
		self->BridgeAmount--;
	}
	// Anropa trafficLightController för att uppdatera ljusen igen efter att bilen har lämnat
	ASYNC(self, trafficLightController, 0);
	return 0;
}

// afterRedGap: kort period med rött ljus på båda sidor efter att en bil har kört in
int afterRedGap(Controller *self, int arg) {
	// Anropa trafficLightController igen för att bestämma vilka lampor som ska lysa
	ASYNC(self, trafficLightController, 0);
	return 0;
}

// bitParser: Hanterar inkommande bitar  (arrival och entry)
void bitParser(Controller *self, int arg) {
	// HANTERA ANKOMSTER
	if (arg & NORTH_ARRIVAL) {
		self->NorthQueue++;
		self->lastQueueSensor = true;  // Senaste kön var från northbound.
	}
	if (arg & SOUTH_ARRIVAL) {
		self->SouthQueue++;
		self->lastQueueSensor = false; // Senaste kön var från southbound.
	}

	// Hantera entries.
	if (arg & NORTH_ENTRY) {
		// Kolla om north ljuset är grönt
		if (self->LightStatus & NORTH_GREEN) {
			if (self->NorthQueue > 0) {
				self->NorthQueue--;
			}
			self->BridgeAmount++;
			// Planera att bilen lämnar bron efter 5 sek
			AFTER(MSEC(BRIDGE_TIME_MS), self, carLeavesBridge, 1);

			// Sätt båda sidor till rött direkt när bilen gått in
			self->LightStatus = NORTHRED_SOUTHRED;
			ASYNC(self->serialCom, USARTtransmit, NORTHRED_SOUTHRED);

			// Efter en kort bara rött period, anropas afterRedGap
			AFTER(MSEC(ALL_RED_GAP_MS), self, afterRedGap, 0);
		}
	}

	if (arg & SOUTH_ENTRY) {
		// Kolla om det south ljuset är grönt
		if (self->LightStatus & SOUTH_GREEN) {
			if (self->SouthQueue > 0) {
				self->SouthQueue--;
			}
			self->BridgeAmount++;
			// Planera att bilen lämnar bron efter 5 sek
			AFTER(MSEC(BRIDGE_TIME_MS), self, carLeavesBridge, 0);

			// Sätt båda sidor till rött när bilen precis gått in
			self->LightStatus = NORTHRED_SOUTHRED;
			ASYNC(self->serialCom, USARTtransmit, NORTHRED_SOUTHRED);

			// Efter en kort bara rött period, anropas afterRedGap
			AFTER(MSEC(ALL_RED_GAP_MS), self, afterRedGap, 0);
		}
	}

	// Anropa trafficLightController ifall nya bilar i kö ändrar logiken
	ASYNC(self, trafficLightController, 0);
}

int sensorEvent(Controller *self, int sensorData) {
	bitParser(self, (uint8_t) sensorData);
	return 0;
}

int getNorthQueue(Controller *self, int unused) {
	return self->NorthQueue;
}

int getSouthQueue(Controller *self, int unused) {
	return self->SouthQueue;
}

int getBridgeAmount(Controller *self, int unused) {
	return self->BridgeAmount;
}
