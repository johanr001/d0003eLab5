#include <fcntl.h>   // Filkontroll
#include <pthread.h> // Pthreads
#include <stdio.h>   // Standard lib för I/O
#include <stdlib.h>  // Standard lib för generalla funktioner (exit)
#include <termios.h> // Terminalkontroll
#include <unistd.h>  // POSIX funktioner

#define SENSOR_NORTH_ARRIVAL 0b00000001 // Sensorbit för ankomst northbound
#define SENSOR_NORTH_ENTRY                                                     \
  0b00000010 // Sensorbit för att en bil faktiskt kör in på bron från northbound
#define SENSOR_SOUTH_ARRIVAL 0b00000100 // Sensorbit för ankomst southbound
#define SENSOR_SOUTH_ENTRY                                                     \
  0b00001000 // Sensorbit för att en bil faktiskt kör in på bron southbound

// Lampstatusbitar som tas emot från styrenheten
#define LIGHT_NORTH_GREEN 0b00000001 // Lampbit för grönt ljus northbound
#define LIGHT_NORTH_RED 0b00000010   // Lampbit för rött ljus northbound
#define LIGHT_SOUTH_GREEN 0b00000100 // Lampbit för grönt ljus southbound
#define LIGHT_SOUTH_RED 0b00001000   // Lampbit för rött ljus southbound

// Global/Delade Variabler.
volatile int running = 1; // Indikerar om simulatorn körs
int serialFd = -1;        // Fildeskriptor för serial port.
int northQueue = 0;       // Antal bilar i kö northbound
int southQueue = 0;       // Antal bilar i kö southbound
int carsOnBridge = 0;     // Antal bilar som befinner sig på bron just nu

// Aktuella lampbitar från styrenheten
int northGreen = 0; // 1 om northbound ljus är grönt
int northRed = 0;   // 1 om northbound ljus är rött
int southGreen = 0; // 1 om southbound ljus är grönt
int southRed = 0;   // 1 om southbound ljus är rött

const char *redColor = "\033[31m";   // ANSI för röd text
const char *greenColor = "\033[32m"; // ANSI för grön text
const char *resetColor = "\033[0m";  // Återställ färg till standard

// Mutex för att skydda alla delade variabler.
pthread_mutex_t simLock = PTHREAD_MUTEX_INITIALIZER;

// ASCII NULL tecken.
char cmdChar = '\0';

// Variabler för att spara & restora terminalinställningar.
struct termios oldTermios;

// Restora gamla terminal
void restoreTerminalSettings(void) {
  tcsetattr(STDIN_FILENO, TCSANOW, &oldTermios);
}

/// setupSerial: öppnar och konfigurerar en serialport
int setupSerial(const char *devicePath) {
  // Öppna serialporten read & write, utan att göra den till en terminal
  // (O_NOCTTY) och med synkroniserad I/O (O_SYNC).
  int fd = open(devicePath, O_RDWR);

  struct termios tty;

  // Hämta nuvarande inställningar för serialporten
  tcgetattr(fd, &tty);

  // Sätt baudrate till 9600
  cfsetospeed(&tty, B9600);
  cfsetispeed(&tty, B9600);

  // Sätt till raw mode för serial port
  cfmakeraw(&tty);

  tty.c_cflag |=
      (CLOCAL | CREAD);   // Ignorera modem control lines och aktivera mottagare
  tty.c_cflag &= ~PARENB; // Inge parity
  tty.c_cflag &= ~CSTOPB; // En stoppbit
  tty.c_cflag &= ~CRTSCTS; // Inge hardware flow control

  // Sätt timeout och minimiantal tecken för läsning
  tty.c_cc[VMIN] = 1;  // Läs minst 1 tecken
  tty.c_cc[VTIME] = 0; // Ingen timeout

  // Sätt nya inställningar för serialporten
  tcsetattr(fd, TCSANOW, &tty);

  return fd;
}

// setupTerminal: konfigurerar terminalen för raw mode
void setupTerminal(void) {
  // Spara nuvarande terminalinställningar
  tcgetattr(STDIN_FILENO, &oldTermios);

  // Skapa en kopia av de nuvarande inställningarna
  struct termios raw = oldTermios;

  // Sätt terminalen till raw mode
  cfmakeraw(&raw);

  // Sätt nya inställningar för terminalen
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);

  // restoreTerminalSettings vid exit.
  atexit(restoreTerminalSettings);
}

// redrawInterface: rensar skärmen och visar status
void redrawInterface(void) {
  // ANSI för att rensa skärmen
  printf("\033[H\033[J");

  // Bestäm vilken text för lamporna beroende på grönt eller rött
  const char *northStatus = (northGreen ? "GREEN" : "RED  ");
  const char *southStatus = (southGreen ? "GREEN" : "RED  ");

  // Skriv ut allt på en rad.
  printf("NORTH: %s%s%s, SOUTH: %s%s%s | North=%-2d, Bridge=%-2d, South=%-2d, "
         "Input: ",
         (northGreen ? greenColor : redColor), northStatus, resetColor,
         (southGreen ? greenColor : redColor), southStatus, resetColor,
         northQueue, carsOnBridge, southQueue);

  // Om vi redan har ett inmatat tecken (cmdChar), skriv ut det
  if (cmdChar != '\0') { // Om cmdChar inte är null så har vi en karaktär
    putchar(cmdChar);    // Skriv ut den till terminalen.
  }
  fflush(stdout); // Skriv ut direkt
}

// carCrossingThread: tråd som simulerar att en bil kör över bron i 5 sekunder
void *carCrossingThread(void *arg) {
  sleep(5); // Simulerar 5 sekunder över bron

  // När bilen är "färdig" minskar vi antalet bilar på bron
  pthread_mutex_lock(&simLock);
  carsOnBridge--;
  redrawInterface();
  pthread_mutex_unlock(&simLock);

  return 0;
}

// userInputThread: läser ett tecken i taget från användaren. Lagrar tecknet i
// cmdChar. Vid ENTER tolkas kommandot.
void *userInputThread(void *unused) {
  while (running) {
    int c = getchar(); // Läs nästa tangenttryckning
    if (c == EOF) {
      break; // Om det inte finns något mer att läsa
    }

    pthread_mutex_lock(&simLock); // Lås mutex en gång per iteration

    if (c == 127) {
      // Hantera backspace
      if (cmdChar != '\0') {
        cmdChar = '\0';
        redrawInterface();
      }

    } else if (c == '\r') {
      // Hantera ENTER
      if (cmdChar != '\0') {
        char tempChar = cmdChar;
        cmdChar = '\0';
        redrawInterface(); // Uppdatera efter att cmdChar rensats

        switch (tempChar) {
        case 'e':
          running = 0;
          pthread_mutex_unlock(&simLock);
          exit(0); // Avsluta omedelbart

        case 'n':
          northQueue++;
          redrawInterface();
          pthread_mutex_unlock(&simLock);
          write(serialFd, &(unsigned char){SENSOR_NORTH_ARRIVAL}, 1);
          continue; // Hoppa till nästa iteration utan att köra resten av koden

        case 's':
          southQueue++;
          redrawInterface();
          pthread_mutex_unlock(&simLock);
          write(serialFd, &(unsigned char){SENSOR_SOUTH_ARRIVAL}, 1);
          continue; // Hoppa till nästa iteration utan att köra resten av koden
        }
      }

    } else if (cmdChar == '\0') {
      // Om vi inte har något inmatat kommando lagrat, lagra tecknet
      cmdChar = c;
      redrawInterface();
    }

    pthread_mutex_unlock(&simLock);
  }

  return 0;
}

// deviceReaderThread: läser lampbitar från AVR:en.
void *deviceReaderThread(void *unused) {
  while (running) {
    unsigned char bits;
    int n = read(serialFd, &bits, 1); // Läs en byte från serial port
    if (n == 0) {
      // Om ingen data , fortsätt vänta så länge vi kör
      if (!running)
        break;
    }

    pthread_mutex_lock(&simLock);

    // Uppdatera lampbitar med mottagen data
    northGreen = (bits & LIGHT_NORTH_GREEN) ? 1 : 0;
    northRed = (bits & LIGHT_NORTH_RED) ? 1 : 0;
    southGreen = (bits & LIGHT_SOUTH_GREEN) ? 1 : 0;
    southRed = (bits & LIGHT_SOUTH_RED) ? 1 : 0;

    // Om norr är grönt och det finns bilar i kö, släpp igenom en
    if (northGreen && northQueue > 0) {
      northQueue--;
      carsOnBridge++;
      redrawInterface();

      // Skicka sensorbit för att bilen verkligen kör in på bron
      unsigned char sensorByte = SENSOR_NORTH_ENTRY;
      write(serialFd, &sensorByte, 1);

      // Starta en tråd som simulerar bilens överfart
      pthread_t crossing;
      pthread_create(&crossing, NULL, carCrossingThread, NULL);
      pthread_detach(crossing);
    }

    // Om söder är grönt och det finns bilar i kö, släpp igenom en
    if (southGreen && southQueue > 0) {
      southQueue--;
      carsOnBridge++;
      redrawInterface();

      // Skicka sensorbit för att bilen verkligen kör in på bron
      unsigned char sensorByte = SENSOR_SOUTH_ENTRY;
      write(serialFd, &sensorByte, 1);

      // Starta en tråd som simulerar bilens överfart
      pthread_t crossing;
      pthread_create(&crossing, NULL, carCrossingThread, NULL);
      pthread_detach(crossing);
    }

    redrawInterface();
    pthread_mutex_unlock(&simLock);
  }
  return 0;
}

int main(int argc, char *argv[]) {
  // Om inga argument använd som standard
  const char *devPath = "/dev/ttyUSB0";
  if (argc > 1) { // Annars använd argumentet
    devPath = argv[1];
  }

  // Öppna och configura serialporten.
  serialFd = setupSerial(devPath);
  setupTerminal();

  // Visa första
  pthread_mutex_lock(&simLock);
  redrawInterface();
  pthread_mutex_unlock(&simLock);

  // Starta trådarna för användarinmatning och för att läsa från styrenheten
  pthread_t User, Device;
  pthread_create(&User, NULL, userInputThread, NULL);
  pthread_create(&Device, NULL, deviceReaderThread, NULL);

  // Vänta på att trådarna avslutas
  pthread_join(User, NULL);
  running = 0; // Sätt running till 0.
  pthread_join(Device, NULL);

  // Stäng serialport och DESTROYA mutex
  close(serialFd);
  pthread_mutex_destroy(&simLock);

  return 0;
}
