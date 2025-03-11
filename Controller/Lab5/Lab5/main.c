#include "Controller.h"
#include "GUI.h"
#include "TinyTimber.h"
#include "avr_init.h"
#include "serialCom.h"
#include <avr/io.h>

int startProgram(GUI *self, int arg) {
  ASYNC(self, updateDisplay, 0);
  ASYNC(self->controller, idleState, 0);
  return 0;
}

int main() {
  avr_init();
  lcd_init();

  // Initializa serialCom.
  SerialCom serial = initSerialCom();

  // Initializa controller, med 0 för GUI
  Controller controller = initController(&serial, 0);

  // Initializa GUI med controller.
  GUI gui = initGUI(&controller);

  // Och nu assigna guit till controller.
  controller.gui = &gui;

  INSTALL(&controller, bitParser, IRQ_USART0_RX);

  return TINYTIMBER(&gui, startProgram, 0);
}
