#include "interruptHandler.h"

// interruptreceiver: Hanterar avbrott fr�n USART RX, arg = IRQ-index, men i detta fall ignoreras det. Den inl�sta byten finns i UDR0.
int interruptreceiver(Interrupthandler *self, int arg) {
	// L�s tecknet fr�n UDR0
	char sensorByte = UDR0;

	// Skicka vidare till Controller som ett sensorEvent
	ASYNC(self->controller, sensorEvent, sensorByte);

	return 0;
}
