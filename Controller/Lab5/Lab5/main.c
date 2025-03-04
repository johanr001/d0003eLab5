#include "TinyTimber.h"
#include "GUI.h"
#include "SerialCom.h"
#include "interruptHandler.h"

// Skapar GUI-objektet.
GUI gui = initGUI();

SerialCom serial = initSerialCom();
Interrupthandler interrupt = initInterruptHandler();


int main(void) {
	// Initiera timer/klocka via avr_init().
	avr_init();
	
	// Initiera LCD innan vi börjar skriva ut.
	lcd_init();

	// Installera USART som interrupthandler.

	INSTALL(&interrupt, interruptreceiver, IRQ_USART0_RX);

	// TINYTIMBER startar kernel. Vi anropar startProgram på gui som första metod.
	return TINYTIMBER(&gui, startProgram, 0);
}
