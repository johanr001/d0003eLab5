#include <avr/io.h>
#include "TinyTimber.h"
#include "GUI.h"
#include "serialCom.h"
#include "controller.h"
#include "avr_init.h"

SerialCom serial = initSerialCom();

Controller controller = initController(&serial);

GUI gui = initGUI(&controller);

int startProgram(GUI *self, int arg) {
	ASYNC(self, updateDisplay, 0);
	ASYNC(self, periodicUpdate, 0);
	ASYNC(self->controller, idleState, 0);
	return 0;
}

int main() {
	
	avr_init();
	lcd_init();

	INSTALL(&controller, bitParser, IRQ_USART0_RX);

	return TINYTIMBER(&gui, startProgram, 0);

}

