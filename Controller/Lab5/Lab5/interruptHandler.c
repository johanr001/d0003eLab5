#include "interruptHandler.h"

// interruptreceiver: Hanterar avbrott från USART RX, arg = IRQ-index, men i detta fall ignoreras det. Den inlästa byten finns i UDR0.
int interruptreceiver(Interrupthandler *self, int arg) {
	// Läs tecknet från UDR0
	char sensorByte = UDR0;

	// Skicka vidare till Controller som ett sensorEvent
	ASYNC(self->controller, sensorEvent, sensorByte);

	return 0;
}
