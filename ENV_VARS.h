// --- PROGRAM ---
// Change number of electrode matrix rows
#define ROWS 13

// Change number of electrode matrix columns
#define COLUMNS 4

// number of trials 
#define TRIALS 3



// --- ANALOG DISCOVERY 2 ---
// Analog I/O pin where flex sensor is connected
int FLEX_CHANNEL = 0;

// Analog I/O pin where potentiometer is connected
int POT_CHANNEL = 1;

// Voltage output of Analog Discovery 2 voltage supply. Max 5 volts
float SUPPLY_VOLTAGE = 5;

// Delay in voltage reading. In milliseconds
int READ_RATE = 100;
