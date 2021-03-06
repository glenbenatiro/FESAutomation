/*
 * FES Automation
 * Developed by Louille Glen Benatiro. 2020.
 */

#include <sstream>
#include <ctime> 
#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <iomanip>
#include <fstream>
#include <cmath> 
#include "sample.h"
#include <ctime> 
#include <algorithm>

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define ENTER 13
#define BACKSPACE 8

using namespace std;

class selectionMatrix {
	public:
		int x, y, xAbs, yAbs;
};

#include "ENV_VARS.h"

// --- RUN-TIME GLOBAL VARIABLES ---
ofstream file;
char name[50]; 							// 
char temp[50]; 							// temporary variable for sprintf operations

int e2Size = 0;
int sFreq[5];
bool isElectrodeRead[COLUMNS][ROWS];
double flexFlatMatrix[COLUMNS][ROWS];
double flexRestMatrix[COLUMNS][ROWS];
double flexFullMatrix[COLUMNS][ROWS];
double flexBendingPercentage[COLUMNS][ROWS];
double vFlexMatrix[COLUMNS][ROWS][TRIALS];
double vPotMatrix[COLUMNS][ROWS][TRIALS];
char startTime[30];
char endTime[30];
double arraySum;
int electrodeOffset;

//digital potentiometer settings
int dioCS = 0;
int dioSCK = 1;
int dioSDIO = 2;
double steps = 256;
float seconds;
unsigned char increment[1] = { 0x04 }, decrement[1] = { 0x08 };
unsigned short zero[1] = { 0x0000 };


// for AD
HDWF hdwf;
double hzSys;
char szError[512] = { 0 };
char szDeviceName[32];

//analog signal settings
int analogChannel = 0;
double aCarrierFreq = 10e3;
double aCarrierAmp = 1.65;
double aAMFreq = 50.0;
int aAMAmp = 100;
int aAMOffset = -100;

// class for getting x and y values in UI function
selectionMatrix choice;

#include "auxil.h"
#include "UI.h"

// -------------------------------------------------------------------------------


void init()
{
	// init vars
	int breakFlag = 0;
	
	cls();
	
	if(FLEX_CHANNEL == POT_CHANNEL) {
		cout << "Flex sensor and potentiometer are set at the same analog I/O channel in code." << endl;
		cout << "See ENV_VARS.h to double-check.";
		
		getch();
		exit(0);
	}
	
	// init all global result arrays to 0
	for(int c = 0; c < COLUMNS; c++) {
		for(int d = 0; d < ROWS; d++) {
			for(int t = 0; t < TRIALS; t++) {
				vFlexMatrix[c][d][t]= 0;	
				vPotMatrix[c][d][t]	= 0;
			}		
			flexFlatMatrix[c][d] 		= 0;
			flexRestMatrix[c][d]    	= 0;
			flexFullMatrix[c][d] 		= 0;
			flexBendingPercentage[c][d]	= 0;
			isElectrodeRead[c][d]		= 0;
		}
	}	
				
	// get name of subject
	cls();
	xy(0,0); cout << "Subject  : ";
	cin >> name;
	xy(0,1); cout << "E2 Offset: ";
	cin >> electrodeOffset;	

	
	while(!breakFlag) {
		
		fflush(stdin);
		
		// clear line, 20 spaces
		for(int i = 3; i < 23; i++) {
			xy(i,3);
			cout << " ";
		}
		
		xy(0,3); cout << "Electrode 2 size (1 for small, 2 for big): ";
		cin >> e2Size;	
		
		if(e2Size == 1 || e2Size == 2) 
			breakFlag = 1;	
	}
	
	// store session start time
	getTime(startTime);
}


void initAD()
{
	FDwfDigitalOutInternalClockInfo(hdwf, &hzSys);
	
	// enable and set supply channel to 5V
	FDwfAnalogIOChannelNodeSet(hdwf, 0, 0, 1);
	FDwfAnalogIOChannelNodeSet(hdwf, 0, 1, SUPPLY_VOLTAGE);
	
	FDwfAnalogIOEnableSet(hdwf, true);
	FDwfAnalogOutCustomAMFMEnableSet(hdwf, analogChannel, true);
    FDwfAnalogOutNodeEnableSet(hdwf, analogChannel, AnalogOutNodeCarrier, true);
    FDwfAnalogOutNodeFunctionSet(hdwf, analogChannel, AnalogOutNodeCarrier, funcSquare);
    FDwfAnalogOutNodeAmplitudeSet(hdwf, analogChannel, AnalogOutNodeCarrier, aCarrierAmp);
    FDwfAnalogOutNodeFrequencySet(hdwf, analogChannel, AnalogOutNodeCarrier, aCarrierFreq);
    FDwfAnalogOutNodeEnableSet(hdwf, analogChannel, AnalogOutNodeAM, true);
    FDwfAnalogOutNodeFunctionSet(hdwf, analogChannel, AnalogOutNodeAM, funcPulse);
    FDwfAnalogOutNodeAmplitudeSet(hdwf, analogChannel, AnalogOutNodeAM, aAMAmp);
    FDwfAnalogOutNodeFrequencySet(hdwf, analogChannel, AnalogOutNodeAM, aAMFreq);
    FDwfAnalogOutNodeOffsetSet(hdwf, analogChannel, AnalogOutNodeAM, aAMOffset);
    FDwfAnalogOutConfigure(hdwf, analogChannel, true);
    
    // configure SPI on AD2
    FDwfDigitalSpiReset(hdwf);
    FDwfDigitalSpiFrequencySet(hdwf, 9600);
    FDwfDigitalSpiClockSet(hdwf, dioSCK);
    FDwfDigitalSpiDataSet(hdwf, 0, dioSDIO);
    FDwfDigitalSpiModeSet(hdwf, 0);
    FDwfDigitalSpiOrderSet(hdwf, 1);
    FDwfDigitalSpiSelect(hdwf, dioCS, 0);
    Sleep(1.0);
    FDwfDigitalSpiWrite16(hdwf, 0, 16, zero, 1);
    
    // enable both analog channels
    FDwfAnalogInChannelEnableSet(hdwf, 0, true);
    FDwfAnalogInChannelEnableSet(hdwf, 1, true);
    
    // set 0V offset
    FDwfAnalogInChannelOffsetSet(hdwf, -1, 0);
    
    // set 10V pk2pk range
    FDwfAnalogInChannelRangeSet(hdwf, -1, SUPPLY_VOLTAGE * 2);
    
    ad2_enableMasterSwitches(false);
}


int dataGatheringSession(int x, int y)
{
	// clear screen
	cls();
	
	// init vars
	int i = 0;
	int breakFlag = 1;
	char choice;
	double vFlex_raw, vPot_raw, restingRatio, trialAverage;
	double tempVPot[500];
	double tempVFlat = 0.0, tempVBend = 5.0;
	double tempArr[9] = {};
	int potCount = 0, flexCount = 0;
	// ---
	
	// adjust y value to account for inverted matrix display
	y = ROWS - y - 1;
	
	// at this point, x and y values are accurate, similar to actual location in data arrays
	
	// draw UI
	xy(0,0); cout << "Cell " << char(65 + x) << y + 1 << " | Actual Array Values: " << x << "," << y;
	xy(0,2); cout << "Bended :";
	xy(0,3); cout << "Flat   :";
	xy(0,4); cout << "Resting:";
	xy(0,6); cout << "vFlex 1:";
	xy(0,7); cout << "vPot  1:";
	xy(0,9); cout << "vFlex 2:";
	xy(0,10);cout << "vPot  2:";
	xy(0,12);cout << "vFlex 3:";
	xy(0,13);cout << "vPot  3:";
	xy(0,15);cout << "Prob   :";
	// --- 
	// turn on master switches on pins
	ad2_enableMasterSwitches(true);
	Wait(0.2);
		
	while(breakFlag) {
		// update UI values while !kbhit()
		while(!kbhit()) {
			// read raw voltage value from flex sensor
			vFlex_raw = ad2_readAnalogIOVoltage(FLEX_CHANNEL);
			if(potCount < 500)
				tempVPot[potCount++]  = ad2_readAnalogIOVoltage(POT_CHANNEL);
			else
				potCount = 0;
			
			if(i < 2) {
						if(!i){
							tempVBend = (tempVBend < vFlex_raw)? tempVBend:vFlex_raw;
							tempArr[i] = tempVBend;
						}
						else if(i){
							tempVFlat = (tempVFlat > vFlex_raw)? tempVFlat:vFlex_raw;
							tempArr[i] = tempVFlat;
						}
			} else if(i < 3){
				if(ifsleep(0.1))
					tempArr[i] = vFlex_raw;
			} else if (i == 3 || i == 5 || i == 7) {
				if(ifsleep(0.1))
					tempArr[i] = vFlex_raw;
				if(ifsleep(0.1)){
					tempArr[i + 1] = *max_element(tempVPot , tempVPot + 499);
				}
			}
			
			// draw voltage values
			cout << fixed << setprecision(3);
			xy(9,2);  cout << tempArr[0] << " V";
			xy(9,3);  cout << tempArr[1] << " V";
			xy(9,4);  cout << tempArr[2] << " V";	
			xy(9,6);  cout << tempArr[3] << " V";
			xy(9,7);  cout << tempArr[4] << " V";	
			xy(9,9);  cout << tempArr[5] << " V";
			xy(9,10); cout << tempArr[6] << " V";
			xy(9,12); cout << tempArr[7] << " V";
			xy(9,13); cout << tempArr[8] << " V";
			
			
			// hello world
			// ---
		}
		
		// check keyboard input after kbhit()
		switch(choice = getch()) {
			case BACKSPACE:
				if(i == 0) {
					breakFlag = 0;
					return 0;
				} else if(i == 1 || i == 2) {
					tempArr[i] = 0;
					i--;
				} else if(i == 3 || i == 5 || i == 7 || i == 9) {
					tempArr[i] = 0;
					tempArr[i + 1] = 0;
					
					if(i == 3)
						i--;
					else
						i -= 2;
				}
				break;
 
			case ENTER:
				if(i == 0 || i == 1 || i == 2) {
					i++;
				} else if(i == 3 || i == 5 || i == 7) {
					i += 2;
				} else if(i == 9) {
					// turn off master switches on pins
					ad2_enableMasterSwitches(false);
					
					// store flat, bend, full flex sensor values
					flexFullMatrix[x][y] = tempArr[0];
					flexFlatMatrix[x][y] = tempArr[1];
					flexRestMatrix[x][y] = tempArr[2];
					
					// store flex sensor and potentiometer voltage values
					for(int i = 0; i < 3; i++) {
						vFlexMatrix[x][y][i] = tempArr[(i * 2) + 3];
						vPotMatrix[x][y][i] = tempArr[(i * 2) + 4];
					}
					
					trialAverage = (tempArr[3]+tempArr[5]+tempArr[7])/3;
					flexBendingPercentage[x][y] = (trialAverage<tempArr[2])?((tempArr[2]-trialAverage)/((tempArr[2]-tempArr[0])/100)):((tempArr[2]-trialAverage)/((tempArr[1]-tempArr[2])/100));
					xy(9,15); cout << flexBendingPercentage[x][y] << "\n";                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
					system("pause");
					if(flexBendingPercentage[x][y] > 100) {
						system("msg * Probability is greater than 100. You might want to retest.");
					} else if(flexBendingPercentage[x][y] < 0) {
						system("msg * Probability is lesser than 0. You might want to retest.");
					}
					
					isElectrodeRead[x][y] = 1;
					breakFlag = 0;
				}
				break;
				
			default:
				break;
		}
	}
	
	return 1;
}

void dataGathering()
{
	//	yStep = 3
	//yInit = 1
	//ROWS.inCode = 4
	//ROWS.inDisplay = ROWS.inCode + 1
	//yMin = yInit
	//ymax without offset = (ROWS.inCode * yStep) + yInit
	//ymax with offset = (((ROWS-electrodeOffset)-1) * yStep) + yInit
	
	int xInit = 1;
	int yInit = 1;
	int xStep = 6;
	int yStep = 3;
	int xMin = xInit;
	int yMin = yInit;	
	int xMax = ((COLUMNS - 1) * xStep) + xInit;
	int yMax = ((ROWS - electrodeOffset) * yStep) + yInit;
	int yPrev = yInit;
	int xPrev = xInit;
		
	while(1) {
		// draw electrode matrix UI
		dataGathering_UI(xStep, yStep);
		
		// draw isElectrodeFinished UI
		isElectrodeRead_UI(xStep, yStep);
		
		// navigator function, returns relative electrode location starting at 0
		choice = navigator(xPrev, yPrev, xStep, yStep, xMin, yMin, xMax, yMax, 1);
		
		// retain previous coordinate
		xPrev = choice.xAbs;
		yPrev = choice.yAbs;
		
		if(yPrev == yMax)
			return;
		else
			dataGatheringSession(choice.x, choice.y);
	}
}

void delData()
{
	// init vars
	int x, y;
	int xInit = 1;
	int yInit = 1;
	int xStep = 6;
	int yStep = 3;
	int xMin = xInit;
	int yMin = yInit;	
	int xMax = ((COLUMNS - 1) * xStep) + xInit;
	int yMax = ((ROWS - electrodeOffset) * yStep) + yInit;
	int yPrev = yInit;
	int xPrev = xInit;
		
	while(1) {
		// draw electrode matrix UI
		dataGathering_UI(xStep, yStep);
		
		// draw isElectrodeFinished UI
		isElectrodeRead_UI(xStep, yStep);
		
		// navigator function, returns relative electrode location starting at 0
		choice = navigator(xPrev, yPrev, xStep, yStep, xMin, yMin, xMax, yMax, 1);
		
		// retain previous coordinate
		xPrev = choice.xAbs;
		yPrev = choice.yAbs;
		
		if(yPrev == yMax)
			return;
		else {
			x = choice.x;
			y = ROWS - 1 - (choice.y);
			
			flexBendingPercentage[x][y] = 0;
			flexFlatMatrix[x][y] = 0;
			flexRestMatrix[x][y] = 0;
			flexFullMatrix[x][y] = 0;
			
			for(int i = 0; i < 3; i++) {
				vFlexMatrix[x][y][i] = 0;
				vPotMatrix[x][y][i] = 0;
			}
			
			isElectrodeRead[x][y] = false;
		}
	}
}

int main()
{
	// change cmd window title
	system("title FES Data Gathering Automation v1.49");
	
	// change color 
	system("color f0");
	
	// resize and constraint cmd window
	sprintf(temp, "mode con: cols=%d lines=%d", 75, (ROWS * 3) + 5);
	system(temp);
	
	// check if analog discovery is connected
	if(!isADConnected())
		exit(0);
	else 
		initAD();
				
	while(1) {
		// initialize some program parameters
		init();
		
		// init vars
		int breakFlag = 0;
		int yPrev = 3;
		
		// main loop
		while(!breakFlag) {
			// main menu
			mainMenu_UI();
			choice = navigator(1, yPrev, 0, 1, 1, 3, 1, 8, 2); 
			yPrev = choice.yAbs;
					
			switch(choice.y) {
				case 1:
					dataGathering();
					break;
				case 2:
					delData();
					break;
				case 3:
					getTime(endTime);
					
					if(e2Size == 2) {
						for(int c = 0; c <= (COLUMNS - 2); c += 2) {
							for(int r = ROWS - 1; r > electrodeOffset - 1; r -= 2) {
								if(isElectrodeRead[c][r]) {
									flexBendingPercentage[c + 1][r] 	= flexBendingPercentage[c][r];
									flexBendingPercentage[c][r - 1] 	= flexBendingPercentage[c][r];	
									flexBendingPercentage[c + 1][r - 1] = flexBendingPercentage[c][r];
									
									isElectrodeRead[c + 1][r] = true;
									isElectrodeRead[c][r - 1] = true;
									isElectrodeRead[c + 1][r - 1] = true;
								}
							}
						}
					}
					
					calcSFreq();
					generateHTMLFile();
					generateCSVFile();
					break;
				case 4:
					dataSeeder();
					break;
				case 5: // reset program
					breakFlag = 1;
					break;
				case 6:
					// close all opened devices
					FDwfDeviceCloseAll();
					exit(0);
					break;
				default:
					break;
			}
		}		
	}
}
