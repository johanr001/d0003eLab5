#ifndef GUI_H
#define GUI_H

#include "TinyTimber.h"
#include "Controller.h"
#include <stdbool.h>
#include <avr/io.h>

typedef struct {
	Object super;
	Controller *controller;
} GUI;

#define initGUI(controller) { initObject(), controller }

int updateDisplay(GUI *self, int arg);
int periodicUpdate(GUI *self, int arg);

void lcd_init(void);
void writeChar(char ch, int pos);
void printAt(long num, int pos);

#endif