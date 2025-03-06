// MAIN FIL FÖR CONTROLLER

#include "TinyTimber.h"
#include "GUI.h"
#include "SerialCom.h"
#include "interruptHandler.h"

// Skapar GUI-objektet.
GUI gui = initGUI();

SerialCom serial = initSerialCom();
Interrupthandler interrupt = initInterruptHandler();


// startProgram() körs vid uppstart. Uppdatera bara displayen.
int startProgram(GUI *self) {

	// Uppdatera displayen omedelbart så vi ser initialfrekvenser (0).
	ASYNC(self, updateDisplay, 0);

	return 0;
}

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
