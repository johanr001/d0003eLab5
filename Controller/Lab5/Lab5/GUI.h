/*
 * GUI.h
 *
 * Created: 04/03/2025 15:04:24
 *  Author: johan_csf2sgl
 */ 


#ifndef GUI_H_
#define GUI_H_


#include "TinyTimber.h"
#include "pulsegen.h"


typedef struct {
	Object super;
	
	
	
} GUI;


#define initGUI(gen1, gen2) { initObject(), gen1, gen2, 0 }


#endif /* GUI_H_ */