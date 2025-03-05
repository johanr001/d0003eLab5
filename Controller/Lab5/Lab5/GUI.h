#ifndef GUI_H_
#define GUI_H_

#include "TinyTimber.h"
#include <avr/io.h>
#include <stdint.h>

typedef struct {
	Object super;
} GUI;


#define initGUI() { initObject()}
	
void updateDisplay(GUI *self, int arg);

void printNorth(GUI *self, int arg);
void PrintSouth(GUI *self, int arg);
void PrintBridge(GUI *self, int arg);

void lcd_init(void);
void writeChar(char ch, int pos);
void printAt(long num, int pos);

#endif /* GUI_H_ */
