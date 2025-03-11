#ifndef GUI_H
#define GUI_H

#include "TinyTimber.h"
#include <avr/io.h>
#include <stdbool.h>

struct Controller; // Forward declaration

typedef struct {
  Object super;
  struct Controller
      *controller; // Anv�nd forward declared struct, d� kan man passa pointer.
} GUI;

#define initGUI(controller) {initObject(), controller}

int updateDisplay(GUI *self, int arg);

void lcd_init(void);
void writeChar(char ch, int pos);
void printAt(long num, int pos);

#endif
