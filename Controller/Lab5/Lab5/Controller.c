#include "Controller.h"

int getNorthQueue(Controller *self, int arg){
	return self->NorthQueue
}

int getSouthQueue(Controller *self, int arg){
	return self->SouthQueue
	
}

int getBridgeAmount(Controller *self, int arg){
	return self->BridgeAmount
	
}



int LampController(Controller *self, int arg){
	
	
	// Om mer än x bilar har kommit från NORTH (Checka om northbound bridge entry sensor var sist och BridgePassedSameDir)
	// och det finns bilar i SouthQUEUE, sätt NORTH till röd och SOUTH till grön. Resetta också hur många som har passed.
	
	// Mer än 10 bilar har passerat, south är inte tom, och den sista bilen över bron kom från NORTH.
	if(self->BridgePassedSameDir > maxStarvation) && (self->SouthQueue > 0) && (self->lastBridgeDir == 1) {
		ASYNC(self->SerialCom, USARTtransmit, NORTHREDSOUTHRED); // Stoppa först trafiken.
		AFTER(MSEC(BRIDGETIME),self->SerialCom, signalOut, NORTHREDSOUTHGREEN); // Sedan när alla bilar är över låt south gå.
		
	}
	
	// Om mer än x bilar har kommit från SOUTH (Checka om northbound bridge entry sensor var sist och BridgePassedSameDir)
	// och det finns bilar i NorthQUEUE, sätt SOUTH till röd och NORTH till grön. Resetta också hur många som har passed.
	
	// Mer än 10 bilar har passerat, north är inte tom, och den sista bilen kom över bron kom från SOUTH.
	if(self->BridgePassedSameDir > maxStarvation) && (self->NorthQueue > 0) && (self->lastBridgeDir == 0) {
		ASYNC(self->SerialCom, USARTtransmit, NORTHREDSOUTHRED); // Stoppa först trafiken.
		AFTER(MSEC(BRIDGETIME),self->SerialCom, signalOut, NORTHGREENSOUTHRED); // Sedan när alla bilar är över låt north gå.
		
	}

	// Om det inte finns några bilar alls. Sätt båda NORTH och SOUTH till röd.
	
	if ((self->NorthQueue == 0) && (self->SouthQueue == 0)) {
		ASYNC(self->SerialCom, USARTtransmit, NORTHREDSOUTHRED);
	}
	
	// Om det bara finns bilar på NORTH, sätt SOUTH till röd och NORTH till grön.
	
	if ((self->NorthQueue > 0) && (self->SouthQueue == 0)) {
		ASYNC(self->SerialCom, USARTtransmit, NORTHGREENSOUTHRED);
	}
	
	// Om det bara finns bilar på SOUTH, sätt NORTH till röd, och SOUTH till grön.
	
	if ((self->SouthQueue > 0) && (self->NorthQueue == 0)) {
		ASYNC(self->SerialCom, USARTtransmit, NORTHREDSOUTHGREEN);
	}
	
	// Om det finns bilar i båda köerna
	if ((self->SouthQueue > 0) && (self->NorthQueue > 0)){
		if (lastQueueSensor){ //Om sista var north så var south först, låt south gå.
			ASYNC(self->SerialCom, USARTtransmit, NORTHREDSOUTHGREEN);
		}
	}
	
}
