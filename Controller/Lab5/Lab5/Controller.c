#include "Controller.h"

// TODO: FIXA LOGIKEN F�R TRAFFICLIGHTCONTROLLER OCH BITPARSERN.


// trafficLightController: Best�mmer vilken sida som blir gr�n/r�d utifr�n aktuell k� och hur m�nga bilar som finns p� bron.
void trafficLightController(Controller *self, int arg)
{
	// Om det redan �r bilar p� bron och vi precis forcade r�tt, v�nta p� afterRedGap
	if (self->BridgeAmount > 0) {
		// Om vi �r i "all-red gap", s� avvaktar vi tills afterRedGap kallar funktionen
		// Om det redan �r gr�nt, g�r inget
		return;
	}

	unsigned long NQ = self->NorthQueue;
	unsigned long SQ = self->SouthQueue;
	unsigned char newLights = NORTHRED_SOUTHRED;  // Standard: r�tt f�r b�da

	// Inga bilar i n�gon k� -> b�da r�da
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
	// B�da sidor har bilar i k�
	else {
		newLights = self->lastQueueSensor ? NORTHGREEN_SOUTHRED :  NORTHRED_SOUTHGREEN;
		}
	}

	// Om de nya lampinst�llningarna skiljer sig, uppdatera och skicka via serial.
	if (newLights != self->LightStatus) {
		self->LightStatus = newLights;
		ASYNC(self->serialCom, USARTtransmit, newLights);
	}
}

// carLeavesBridge: AFTER 5 sekunder efter att en bil k�r in.
int carLeavesBridge(Controller *self, int arg) {
	// Minskar antalet bilar p� bron om det �r minst 1
	if (self->BridgeAmount > 0) {
		self->BridgeAmount--;
	}
	// Anropa trafficLightController f�r att uppdatera ljusen igen efter att bilen har l�mnat
	ASYNC(self, trafficLightController, 0);
	return 0;
}

// afterRedGap: kort period med r�tt ljus p� b�da sidor efter att en bil har k�rt in
int afterRedGap(Controller *self, int arg) {
	// Anropa trafficLightController igen f�r att best�mma vilka lampor som ska lysa
	ASYNC(self, trafficLightController, 0);
	return 0;
}

// bitParser: Hanterar inkommande bitar  (arrival och entry)
void bitParser(Controller *self, int arg) {
	// HANTERA ANKOMSTER
	if (arg & NORTH_ARRIVAL) {
		self->NorthQueue++;
		self->lastQueueSensor = true;  // Senaste k�n var fr�n northbound.
	}
	if (arg & SOUTH_ARRIVAL) {
		self->SouthQueue++;
		self->lastQueueSensor = false; // Senaste k�n var fr�n southbound.
	}

	// Hantera entries.
	if (arg & NORTH_ENTRY) {
		// Kolla om north ljuset �r gr�nt
		if (self->LightStatus & NORTH_GREEN) {
			if (self->NorthQueue > 0) {
				self->NorthQueue--;
			}
			self->BridgeAmount++;
			// Planera att bilen l�mnar bron efter 5 sek
			AFTER(MSEC(BRIDGE_TIME_MS), self, carLeavesBridge, 1);

			// S�tt b�da sidor till r�tt direkt n�r bilen g�tt in
			self->LightStatus = NORTHRED_SOUTHRED;
			ASYNC(self->serialCom, USARTtransmit, NORTHRED_SOUTHRED);

			// Efter en kort bara r�tt period, anropas afterRedGap
			AFTER(MSEC(ALL_RED_GAP_MS), self, afterRedGap, 0);
		}
	}

	if (arg & SOUTH_ENTRY) {
		// Kolla om det south ljuset �r gr�nt
		if (self->LightStatus & SOUTH_GREEN) {
			if (self->SouthQueue > 0) {
				self->SouthQueue--;
			}
			self->BridgeAmount++;
			// Planera att bilen l�mnar bron efter 5 sek
			AFTER(MSEC(BRIDGE_TIME_MS), self, carLeavesBridge, 0);

			// S�tt b�da sidor till r�tt n�r bilen precis g�tt in
			self->LightStatus = NORTHRED_SOUTHRED;
			ASYNC(self->serialCom, USARTtransmit, NORTHRED_SOUTHRED);

			// Efter en kort bara r�tt period, anropas afterRedGap
			AFTER(MSEC(ALL_RED_GAP_MS), self, afterRedGap, 0);
		}
	}

	// Anropa trafficLightController ifall nya bilar i k� �ndrar logiken
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
