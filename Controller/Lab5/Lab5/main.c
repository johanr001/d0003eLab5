#include "TinyTimber.h"
#include "GUI.h"


// Skapar GUI-objektet.
GUI gui = initGUI();




int main(void) {
	// Initiera timer/klocka via avr_init().
	avr_init();
	
	// Initiera LCD innan vi b�rjar skriva ut.
	lcd_init();

	// Installera knappobjektet som interrupthandler f�r PCINT0 och PCINT1.

	INSTALL(&interrupt, horizontal, IRQ_PCINT0);
	INSTALL(&interrupt, vertandcent, IRQ_PCINT1);

	// TINYTIMBER startar kernel. Vi anropar startProgram p� gui som f�rsta metod.
	return TINYTIMBER(&gui, startProgram, 0);
}
