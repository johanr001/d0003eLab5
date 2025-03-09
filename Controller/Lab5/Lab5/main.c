// MAIN FIL FÖR CONTROLLER

#include "TinyTimber.h"
#include "GUI.h"
#include "SerialCom.h"
#include "interruptHandler.h"
#include "Controller.h"
#include "avr_init.h"

// Skapar alla objekt.
SerialCom	serial = initSerialCom();
Controller	controller = initController(&serial);
GUI	gui = initGUI(&controller);
Interrupthandler ih = initInterruptHandler(&controller);

// startProgram() körs vid uppstart. Uppdatera bara displayen.
int startProgram(GUI *self, int arg) {
	// Skriv initial display
	ASYNC(self, updateDisplay, 0);
	// Starta uppdatering varje 500 ms
	ASYNC(self, periodicUpdate, 0);
	return 0;
}

int main(void) {
	// Initiera timer/klocka via avr_init().
	avr_init();
	
	// Initiera LCD innan vi börjar skriva ut.
	lcd_init();

	// Installera USART som interrupthandler.
	INSTALL(&ih, interruptreceiver, IRQ_USART0_RX);
	
	// TINYTIMBER startar kernel. Vi anropar startProgram på gui som första metod.
	return TINYTIMBER(&gui, startProgram, 0);
}
