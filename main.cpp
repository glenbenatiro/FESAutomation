/*
 * This file is part of FES Automation program.
 * Developed by Louille Glen Benatiro. 2020.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
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

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define ENTER 13
#define BACKSPACE 8

using namespace std;

class selectionMatrix {
	public:
		int x, y;
};

#include "ENV_VARS.h"

// --- RUN-TIME GLOBAL VARIABLES ---
ofstream file;
char name[50]; 							// to store name of test subject, also used as title of resulting .txt file 
char temp[50]; 							// to temporarily store file of sprintf operations


double flexFlatMatrix[COLUMNS][ROWS];
double flexRestMatrix[COLUMNS][ROWS];
double flexFullMatrix[COLUMNS][ROWS];
double flexProbability[COLUMNS][ROWS];

double vFlexMatrix[COLUMNS][ROWS][TRIALS];
double vPotMatrix[COLUMNS][ROWS][TRIALS];

char startTime[30];
char endTime[30];

double arraySum;

int electrodeOffset;

// for AD
HDWF hdwf;
double hzSys;
char szError[512] = { 0 };
char szDeviceName[32];
int dgIO1 = 4; // set digital pin 1
int dgIO2 = 6; // set digital pin 2
int dFreq = 10000;
int dDutyPlus = 0;
int dSamples = 100;
int cSamples;

// class for getting x and y values in UI function
selectionMatrix choice;

#include "auxil.h"
#include "UI.h"

// -------------------------------------------------------------------------------


void init()
{
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
			
			flexFlatMatrix[c][d] 	= 0;
			flexRestMatrix[c][d]    = 0;
			flexFullMatrix[c][d] 	= 0;
			flexProbability[c][d] 	= 0;
		}
	}	
				
	// get name of subject
	cls();
	cout << "Subject name: ";
	cin >> name;
	cout << "Electrode 2 offset: ";
	cin >> electrodeOffset;
	
	// store session start time
	getTime(startTime);
}


void initAD()
{
	BYTE rgcustom[100] = { 0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE
                                    ,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE
                                    ,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE };
	
	FDwfDigitalOutInternalClockInfo(hdwf, &hzSys);
	
	// enable and set supply channel to 5V
	FDwfAnalogIOChannelNodeSet(hdwf, 0, 0, 1);
	FDwfAnalogIOChannelNodeSet(hdwf, 0, 1, SUPPLY_VOLTAGE);

	// configure 10kHz custom on IO pin 0
	FDwfDigitalOutEnableSet(hdwf, dgIO1, true);
    FDwfDigitalOutOutputSet(hdwf, dgIO1, DwfDigitalOutOutputThreeState);
    FDwfDigitalOutTypeSet(hdwf, dgIO1, DwfDigitalOutTypeCustom);
    FDwfDigitalOutIdleSet(hdwf, dgIO1, DwfDigitalOutIdleZet);
    FDwfDigitalOutDividerSet(hdwf, dgIO1, hzSys / dFreq / 2);
    FDwfDigitalOutDataSet(hdwf, dgIO1, rgcustom, 100 * 8);
    
    // configure 3.3V on IO pin 1
    FDwfDigitalOutEnableSet(hdwf, dgIO2, true);
    FDwfDigitalOutDividerSet(hdwf, dgIO2, hzSys / dFreq / dSamples);
    FDwfDigitalOutCounterSet(hdwf, dgIO2, 0, 100);
    
    // enable both analog channels
    FDwfAnalogInChannelEnableSet(hdwf, 0, true);
    FDwfAnalogInChannelEnableSet(hdwf, 1, true);
    
    // set 0V offset
    FDwfAnalogInChannelOffsetSet(hdwf, -1, 0);
    
    // set 10V pk2pk range
    FDwfAnalogInChannelRangeSet(hdwf, -1, SUPPLY_VOLTAGE * 2);
}


void dataGatheringSession(selectionMatrix choice, string name)
{
	// ---init vars
	int flag, count = 0;
	char menuChoice;
	
	// temp storage for flexFlat and flexFull values for percentage calculation
	double flexFlat, flexFull;
	
	// storage for raw vFlex and vPot values on realtime read
	double vFlex_raw, vPot_raw;
	
	// readjust choice struct values
	selectionMatrix arr;
	arr.x = choice.x - 1;
	arr.y = ((ROWS + 1) - choice.y) - 1;
	
	// turn on master switches on pins
	ad2_enableMasterSwitches(true);
	
	// get and store flex sensor values, both flat and fully bended
	if(!getFlexSensorValues(arr.x, arr.y))
		return;   	
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
	int xStep = 4;
	int yStep = 3;
	int xMin = xInit;
	int yMin = yInit;	
	int xMax = ((COLUMNS - 1) * xStep) + xInit;
	int yMax = ((ROWS - electrodeOffset) * yStep) + yInit;
		
	while(1) {
		dataGathering_UI(xStep, yStep);
		choice = navigator(xInit, yInit, xStep, yStep, xMin, yMin, xMax, yMax);
		
		if(choice.y == (ROWS - electrodeOffset + 1))
			return;
		else
			dataGatheringSession(choice, name);
	}
}

void saveToFile()
{
	cls();
	
	getTime(endTime);
	generateHTMLFile();
	sprintf(temp, "See %s_RESULTS.txt in file folder.", name);
	
	cout << "Data exported." << endl;
	cout << temp << endl << endl;
	cout << "Press any key to go back...";
	getch();
}

int main()
{
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
		
		// flag for program reset to change test subject and electrode 2
		int flag = 0;
		
		// main loop
		while(1) {
			// main menu
			mainMenu_UI();
			choice = navigator(1, 0, 0, 1, 1, 0, 1, 4); 
			
			switch(choice.y) {
				case 1:
					dataGathering();
					break;
				case 2:
					getTime(endTime);
					generateHTMLFile();
					generateCSVFile();
					break;
				case 3:
					dataSeeder();
					break;
				case 4:
					flag = 1;
					break;
				case 5:
					// close all opened devices
					FDwfDeviceCloseAll();
					exit(0);
					break;
				default:
					break;
			}
		}
		
		if(flag)
			break;		
	}

}



