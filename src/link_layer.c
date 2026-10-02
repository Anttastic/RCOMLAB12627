// RCOM 2026/2027
//
// Link layer protocol implementation

#include "link_layer.h"
#include "serial_port.h"

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source
#define BUF_SIZE 256

// Super vision frame fields
#define FLAG  0x7E
#define A_SND 0x03
#define A_RCV 0x01
#define C_SET 0x03
#define C_UA  0x07

//Retransmission
#define TIMEOUT_S   3
#define MAX_RETRIES 3

// Super vision frames (FLAG | A | C | BCC1 = A^C | FLAG)
static const unsigned char SET_FRAME[5] = {0x7E, 0x03, 0x03, 0x00, 0x7E};
static const unsigned char UA_FRAME[5]  = {0x7E, 0x03, 0x07, 0x04, 0x7E};

// State machine states
typedef enum
{
    START,
    FLAG_RCV,
    ST_A_RCV,
    C_RCV,
    BCC_OK,
    STOP
} State;    

// Alarm
static int alarmEnabled = FALSE;
static int alarmCount = 0;

static void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;
}

static State nextState(State state, unsigned char byte, unsigned char a, unsigned char c) {
    switch (state) {
        case START:
            if (byte == FLAG) return FLAG_RCV;
            return START;
        case FLAG_RCV:
            if (byte == a) return ST_A_RCV;
            if (byte == FLAG) return FLAG_RCV;
            return START;
        case ST_A_RCV:
            if (byte == c) return C_RCV;
            if (byte == FLAG) return FLAG_RCV;
            return START;
        case C_RCV:
            if (byte == (a^c)) return BCC_OK;
            if (byte == FLAG) return FLAG_RCV;
            return START;
        case BCC_OK:
            if (byte == FLAG) return STOP;
            return START;
        default:
            return state;
    }
}

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llOpenTx(LinkLayer llParameters)
{
    // ----------------------------------------------------
    // This example code shows how to open the serial port and send a string.
    // TODO: Adapt and extend this code according to the specifications of the project.
    // ----------------------------------------------------

    if (openSerialPort(llParameters.serialPort, llParameters.baudRate) < 0)
    {
        perror("openSerialPort");
        return -1;
    }

    printf("Serial port %s opened\n", llParameters.serialPort);

    // Set alarm function handler
    struct sigaction act = {0};
    act.sa_handler = &alarmHandler;

    if (sigaction(SIGALRM, &act, NULL) == -1) {
        perror("sigaction");
        return -1;
    }

    State state = START;
    alarmCount = 0;

    // Enviar o SET e esperar por UA
    while (alarmCount < MAX_RETRIES && state != STOP) {
        int bytes = writeBytesSerialPort(SET_FRAME, 5);
        printf("SET sent: %d bytes written (attempt %d)\n", bytes, alarmCount + 1);
        
        alarm(TIMEOUT_S);
        alarmEnabled = TRUE;
        state = START;

        while (alarmEnabled == TRUE && state != STOP) {
            unsigned char byte;
            if (readByteSerialPort(&byte) > 0) {
                state = nextState(state, byte, A_SND, C_UA);
            }
        }
    }

    // Dar disable em alarms à espera
    alarm(0);

    if (state != STOP) {
        printf("No UA received after %d attempts\n", MAX_RETRIES);
        return -1;
    }

    printf("UA received. Connection established.\n");

    return 0;
}

int llOpenRx(LinkLayer llParameters)
{
    // ----------------------------------------------------
    // This example code shows how to open the serial port and receive a string.
    // TODO: Adapt and extend this code according to the specifications of the project.
    // ----------------------------------------------------

    if (openSerialPort(llParameters.serialPort, llParameters.baudRate) < 0)
    {
        perror("openSerialPort");
        return -1;
    }

    printf("Serial port %s opened\n", llParameters.serialPort);

    // Read from serial port until the 'z' char is received.

    // NOTE: This while() cycle is a simple example showing how to read from the serial port.
    // It must be changed in order to respect the specifications of the protocol indicated in the Lab guide.

    State state = START;

    while (state != STOP) {
        unsigned char byte;
        if (readByteSerialPort(&byte) > 0) {
            state = nextState(state, byte, A_SND, C_SET);
        }
    }

    printf("SET received\n");

    // Responder com UA
    int bytes = writeBytesSerialPort(UA_FRAME, 5);
    printf("UA sent: %d bytes written\n", bytes);
    
    return 0;
}

////////////////////////////////////////////////
// LLSEND
////////////////////////////////////////////////
int llSend(const unsigned char *buf, int bufSize)
{
    // TODO: Implement this function

    return 0;
}

////////////////////////////////////////////////
// LLRECEIVE
////////////////////////////////////////////////
int llReceive(unsigned char *packet)
{
    // TODO: Implement this function

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llCloseTx()
{
    // TODO: Implement this function

    return 0;
}

int llCloseRx()
{
    // TODO: Implement this function

    return 0;
}
