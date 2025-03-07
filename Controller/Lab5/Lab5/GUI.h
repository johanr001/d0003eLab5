#ifndef GUI_H_
#define GUI_H_

#include "TinyTimber.h"
#include <avr/io.h>
#include <stdint.h>
#include "Controller.h"

typedef struct {
	Object super;
	Controller *controller;
} GUI;


#define initGUI(controller) { initObject(), controller}
	
void updateDisplay(GUI *self, int arg);

void lcd_init(void);
void writeChar(char ch, int pos);
void printAt(long num, int pos);

#endif /* GUI_H_ */
