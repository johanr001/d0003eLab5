#include <stdio.h>      // Standard lib för I/O
#include <stdlib.h>     // Standard lib för generalla funktioner (exit)
#include <unistd.h>     // POSIX funktioner
#include <fcntl.h>      // Filkontroll
#include <termios.h>    // Terminalkontroll
#include <pthread.h>    // Pthreads



#define SENSOR_NORTH_ARRIVAL  0b00000001  // Sensorbit för ankomst northbound
#define SENSOR_NORTH_ENTRY    0b00000010  // Sensorbit för att en bil faktiskt kör in på bron från northbound
#define SENSOR_SOUTH_ARRIVAL  0b00000100  // Sensorbit för ankomst southbound
#define SENSOR_SOUTH_ENTRY    0b00001000  // Sensorbit för att en bil faktiskt kör in på bron southbound

// Lampstatusbitar som tas emot från styrenheten
#define LIGHT_NORTH_GREEN     0b00000001  // Lampbit för grönt ljus northbound
#define LIGHT_NORTH_RED       0b00000010  // Lampbit för rött ljus northbound
#define LIGHT_SOUTH_GREEN     0b00000100  // Lampbit för grönt ljus southbound
#define LIGHT_SOUTH_RED       0b00001000  // Lampbit för rött ljus southbound

// Global/Delade Variabler.
volatile int running = 1;         // Indikerar om simulatorn körs
int serialFd = -1;                // Fildeskriptor för seriell port
int northQueue = 0;               // Antal bilar i kö northbound
int southQueue = 0;               // Antal bilar i kö southbound
int carsOnBridge = 0;             // Antal bilar som befinner sig på bron just nu

// Aktuella lampbitar från styrenheten
int northGreen = 0;               // 1 om northbound ljus är grönt
int northRed   = 0;               // 1 om northbound ljus är rött
int southGreen = 0;               // 1 om southbound ljus är grönt
int southRed   = 0;               // 1 om southbound ljus är rött


const char *redColor   = "\033[31m";   // ANSI för röd text
const char *greenColor = "\033[32m";  // ANSI för grön text
const char *resetColor = "\033[0m";   // Återställ färg till standard

// Mutex för att skydda alla delade variabler.
pthread_mutex_t simLock = PTHREAD_MUTEX_INITIALIZER;

// ASCII NULL tecken.
char cmdChar = '\0';

// Variabler för att spara/återställa terminalinställningar. 
struct termios oldTermios;

// setupSerial: öppnar och konfigurerar en serialport
int setupSerial(const char *devicePath)
{
    // Öppna den serialporten read & write, utan att göra den till en terminal (O_NOCTTY) och med synkroniserad I/O (O_SYNC).
    int fd = open(devicePath, O_RDWR | O_NOCTTY | O_SYNC);

    struct termios tty;

    // Sätt baudrate till 9600
    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);
    
    // Sätt 8 databitar (CS8), ingen paritet, 1 stoppbit (8N1)
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= (CLOCAL | CREAD);      // Ignorera modem control lines, slå på reciever
    tty.c_cflag &= ~(PARENB | PARODD);    // No parity
    tty.c_cflag &= ~CSTOPB;               // 1 stoppbit
    tty.c_cflag &= ~CRTSCTS;              // Ingen flow control
    

    tty.c_iflag = IGNBRK; // IGNBRK ignorerar break-tecken
    tty.c_lflag = 0;     // Sätt raw mode.
    tty.c_oflag = 0; // Slå av all output processing.
    


    tty.c_cc[VMIN]  = 1; // VMIN=1: blockera tills minst 1 byte har lästs
    tty.c_cc[VTIME] = 0; // VTIME=0: ingen timeout

    return fd;
}


// redrawInterface: rensar skärmen och visar status

void redrawInterface(void)
{
    // ANSI-sekvens för att rensa skärmen
    printf("\033[H\033[J");

    // Bestäm textsträng för lamporna beroende på grönt eller rött
    const char *northStatus = (northGreen ? "GREEN" : "RED  ");
    const char *southStatus = (southGreen ? "GREEN" : "RED  ");

    // Utskrift av status och (eventuellt) inmatat kommando på samma rad
    printf("NORTH: %s%s%s, SOUTH: %s%s%s | North=%-2d, Bridge=%-2d, South=%-2d, Input: ",
           (northGreen ? greenColor : redColor), northStatus, resetColor,
           (southGreen ? greenColor : redColor), southStatus, resetColor,
           northQueue, carsOnBridge, southQueue);
    
    // Om vi redan har ett inmatat tecken (cmdChar), skriv ut det
    if (cmdChar != '\0') { // Om cmdChar inte är null så har vi en karaktär
        putchar(cmdChar); // Skriv ut den till terminalen.	
    }
    fflush(stdout); // Skriv ut direkt
}


// carCrossingThread: tråd som simulerar att en bil kör över bron i 5 sekunder
void *carCrossingThread(void *arg)
{
    sleep(5);  // Simulerar 5 sekunders färd över bron

    // När bilen är "färdig" minskar vi antalet bilar på bron
    pthread_mutex_lock(&simLock);
    carsOnBridge--;
    redrawInterface();
    pthread_mutex_unlock(&simLock);

return 0;
}


// enableRawMode: sätter stdin i "rawmode". // Gör att vi kan läsa tecken omedelbart utan att vänta på returtryckning.
void enableRawMode(void)
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &oldTermios);  // Spara nuvarande inställningar

    raw = oldTermios;
    // Stäng av kanoniskt läge och ekofunktion
    raw.c_lflag &= ~(ICANON | ECHO);
    // Ange att read() ska blockera för minst 1 tecken
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSANOW, &raw); // Tillämpa ändringarna direkt
}


// disableRawMode: återställer ursprungligt läge för stdin
void disableRawMode(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &oldTermios);
}

// userInputThread: läser ett tecken i taget från användaren. Lagrar tecknet i cmdChar. Vid ENTER tolkas kommandot.
void *userInputThread(void *unused)
{
    while (running) {
        int c = getchar(); // Läs nästa tangenttryckning
        if (c == EOF) {
            break; // Om det inte finns något mer att läsa
        }

        // Hantera backspace
        if (c == 127 || c == 8) {
            pthread_mutex_lock(&simLock);
            cmdChar = '\0';  // Töm den sparade kommandotecknet
            redrawInterface();
            pthread_mutex_unlock(&simLock);
            continue;
        }

        // Hantera ENTER (ny rad)
        if (c == '\n' || c == '\r') {
            pthread_mutex_lock(&simLock);
            if (cmdChar != '\0') {
                char localCmd = cmdChar;
                cmdChar = '\0';  // Nollställ cmdChar
                // Uppdatera gränssnitt efter att cmdChar rensats
                redrawInterface();
                pthread_mutex_unlock(&simLock);

                // Välj åtgärd beroende på vilken bokstav användaren matat in
                if (localCmd == 'e') {
                    // Avsluta simulatorn
                    disableRawMode();
                    exit(0);  
                } else if (localCmd == 'n') {
                    // Lägg till en bil i kön northbound
                    pthread_mutex_lock(&simLock);
                    northQueue++;
                    redrawInterface();
                    pthread_mutex_unlock(&simLock);

                    // Meddela styrenheten via seriell port
                    unsigned char sensorByte = SENSOR_NORTH_ARRIVAL;
                    write(serialFd, &sensorByte, 1);

                } else if (localCmd == 's') {
                    // Lägg till en bil i kön söder
                    pthread_mutex_lock(&simLock);
                    southQueue++;
                    redrawInterface();
                    pthread_mutex_unlock(&simLock);

                    // Meddela styrenheten via seriell port
                    unsigned char sensorByte = SENSOR_SOUTH_ARRIVAL;
                    write(serialFd, &sensorByte, 1);
                }
            } else {
                // Inget tecken inmatat, lås upp och fortsätt
                pthread_mutex_unlock(&simLock);
            }
            continue;
        }

        // Om vi inte har något inmatat kommando lagrat, lagra detta tecken (som gemener)
        pthread_mutex_lock(&simLock);
        if (cmdChar == '\0') {
            cmdChar = c;
            redrawInterface();
        }
        pthread_mutex_unlock(&simLock);
        // Om cmdChar redan är satt ignorerar vi ytterligare tecken
    }
    return NULL;
}


// deviceReaderThread: läser lampbitar från AVR:en.
void *deviceReaderThread(void *unused)
{
    while (running) {
        unsigned char b;
        int n = read(serialFd, &b, 1);  // Läs en byte från seriell port
        if (n <= 0) {
            // Om ingen data eller fel, fortsätt vänta så länge vi kör
            if (!running) break;
            continue;
        }

        pthread_mutex_lock(&simLock);

        // Uppdatera lampbitar med mottagen data
        northGreen = (b & LIGHT_NORTH_GREEN) ? 1 : 0;
        northRed   = (b & LIGHT_NORTH_RED)   ? 1 : 0;
        southGreen = (b & LIGHT_SOUTH_GREEN) ? 1 : 0;
        southRed   = (b & LIGHT_SOUTH_RED)   ? 1 : 0;

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
    return NULL;
}


int main(int argc, char *argv[])
{
    // Om inga argument använd som standard
    const char *devPath = "/dev/ttyUSB0";
    if (argc > 1) { // Annars använd argumentet
        devPath = argv[1];
    }

    // Öppna och configura serialporten.
    serialFd = setupSerial(devPath);
  
    // Sätt terminalen i rawmode för att läsa tecken direkt
    enableRawMode();

    // Rensa skärmen och visa första
    pthread_mutex_lock(&simLock);
    redrawInterface();
    pthread_mutex_unlock(&simLock);

    // Starta trådarna för användarinmatning och för att läsa från styrenheten
    pthread_t User, Device;
    pthread_create(&User, NULL, userInputThread, NULL);
    pthread_create(&Device, NULL, deviceReaderThread, NULL);

    // Vänta på att nmatningstråden avslutas
    pthread_join(User, NULL);

    // Avsluta
    running = 0;
    close(serialFd);            // Stäng serialport
    pthread_join(Device, NULL);
    disableRawMode();           // Återställ terminal

    return 0;
}
