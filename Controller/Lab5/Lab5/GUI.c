#include "GUI.h"
#include "Controller.h" // Så man kan calla getAmount i updateDisplay.

// font[] innehåller bitmönster för siffrorna 0-9 på LCD:n.
// Varje siffra representeras som ett 16-bitars värde.
const uint16_t font[] = {
    0x1551, // 0
    0x0110, // 1
    0x1E11, // 2
    0x1B11, // 3
    0x0B50, // 4
    0x1B41, // 5
    0x1F41, // 6
    0x0111, // 7
    0x1F51, // 8
    0x1B51  // 9
};

// Starta LCD
void lcd_init() {
  // LCD Control Register A:
  // Turn on LCD (LCDEN = 1)
  // Low Power Waveform (LCDAB = 1)
  LCDCRA = (1 << LCDEN) | (1 << LCDAB);

  // LCD Control Register B:
  // Asynchronous Clock (LCDCS = 1)
  // 1/3 Bias (LCD2B = 0)
  // 1/4 Duty (LCDMUX1 & LCDMUX0 = 1)
  // 25 Segments (LCDPM2 & LCDPM1 & LCDPM0 = 1)
  // Prescaler N = 16 (LCDPS2 & LCDPS1 & LCDPS0 = 0)
  // Clock Divide = 8 (LCDCD2 & LCDCD1 & LCDCD0 = 1
  LCDCRB = (1 << LCDCS) | (1 << LCDMUX1) | (1 << LCDMUX0) | (1 << LCDPM2) |
           (1 << LCDPM1) | (1 << LCDPM0) | (1 << LCDCD2) | (1 << LCDCD1) |
           (1 << LCDCD0);

  // LCD Contrast Control Register:
  // Drive Time = 300 µs (LCDDC2 & LCDDC1 & LCDDC0 = 0)
  // Contrast Control Vol	tage = 3.35 V (LCDCC3 & LCDCC2 & LCDCC1 & LCDCC0
  // = 1)
  LCDCCR = (1 << LCDCC3) | (1 << LCDCC2) | (1 << LCDCC1) | (1 << LCDCC0);
}

// Karaktär (alltså 0-9) för positionen 0-5
void writeChar(char ch, int pos) {
  if (pos < 0 || pos > 5)
    return;                     // Checka att position är korrekt.
  uint16_t segments = 0x0000;   // Default är tom.
  if (ch >= '0' && ch <= '9') { // Checka om det är siffra
    segments =
        font[ch -
             '0']; // Hämta font data för siffra array positionen, alltså t.ex
                   // ASCII för 8 blir till: 56-48=8. Och font[8] = 0x1F51.
  }
  uint8_t *base =
      (uint8_t *)&LCDDR0 +
      pos / 2; // Beräkna basregistret för positionen. Displayen använder
  // LCDDR0-LCDDR19. *base dereference pointer till minnesadressen
  // av korrekt. Läs sida 8 av butterfly_lcd. De är i par. Så pos =
  // 0,1 blir LCDDR0 Type cast också ifall det inte fungerar annars
  // kan testa utan det fungerar säkert. Beräkna offset alltså low
  // eller high nibble för registret om den är jämn eller ojämn.
  // pos = 0,2,4 blir till 0 offset.
  int offset = (pos % 2) * 4;

  base[0] = (base[0] & ~(0xF << offset)) |
            ((segments & 0xF) << offset); // Uppdatera nibble i första registret
  base[5] = (base[5] & ~(0xF << offset)) |
            (((segments >> 4) & 0xF)
             << offset); // Uppdatera nibble i LCDDRx+5 registret, alltså
  // pointer offset med base[5].
  base[10] =
      (base[10] & ~(0xF << offset)) |
      (((segments >> 8) & 0xF)
       << offset); // Om offset 4 till exempel. 00001111 blir till 11110000,
  // sedan 00001111, och bitwise AND vilket gör de 4 bits som
  // har bestämts med offset till 0000.
  base[15] = (base[15] & ~(0xF << offset)) |
             (((segments >> 12) & 0xF)
              << offset); // Sedan segments & 0xF blir 000000000000xxxx, AND med
                          // segments ger bara värdet för dessa 4 bits. Samma
                          // fast för offset för de andra raderna. Alltså om vi
                          // har aaaabbbbccccdddd, så ger varje rad,
                          // dddd,cccc,bbbb,aaaa. Sedan << offset för att flytta
                          // vänster om det behövs. Sedan | OR för att kombinera
                          // med hälften tom och hälften redan fylld. Alltså
                          // 0000xxxx om ingen offset, eller xxxx0000 om offset.
}

void printAt(long num, int pos) {
  int pp = pos;
  writeChar((num % 100) / 10 + '0', pp);
  pp++;
  writeChar(num % 10 + '0', pp);
}

// updateDisplay() Hämtar värden för NorthQueue, BridgeAmount, SouthQueue och
// skriver på LCD
int updateDisplay(GUI *self, int arg) {
  printAt(SYNC(self->controller, getNorthQueue, 0), 0);
  printAt(SYNC(self->controller, getBridgeAmount, 0), 2);
  printAt(SYNC(self->controller, getSouthQueue, 0), 4);
  return 0;
}
