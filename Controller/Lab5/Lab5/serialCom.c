#include "serialCom.h"

int USARTreceiver(SerialCom *self, int arg){
	!( (PINB >> 6) & 0x01 ) // TODO: Fixa
}

int USARTtransmit(SerialCom *self, int arg){
}