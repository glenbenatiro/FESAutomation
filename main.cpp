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
		int x, y, xAbs, yAbs;
};

#include "ENV_VARS.h"

// --- RUN-TIME GLOBAL VARIABLES ---
ofstream file;
char name[50]; 							// to store name of test subject, also used as title of resulting .txt file 
char temp[50]; 							// to temporarily store file of sprintf operations

bool isElectrodeRead[COLUMNS][ROWS] = {0};
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
			isElectrodeRead[c][d]	= 0;
		}
	}	
				
	// get name of subject
	cls();
	cout << "Subject  : ";
	cin >> name;
	cout << "E2 Offset: ";
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


int dataGatheringSession(int x, int y)
{
	// clear screen
	cls();
	
	// init vars
	int i = 0;
	int breakFlag = 1;
	char choice;
	double vFlex_raw, vPot_raw;
	double tempArr[9] = {};
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
	// --- 
	
	// turn on master switches on pins
	ad2_enableMasterSwitches(true);
		
	while(breakFlag) {
		
		// update UI values while !kbhit()
		while(!kbhit()) {
			// read raw voltage value from flex sensor
			vFlex_raw = ad2_readAnalogIOVoltage(FLEX_CHANNEL);
			vPot_raw  = ad2_readAnalogIOVoltage(POT_CHANNEL);
			
			if(i <= 2) {
				if(vFlex_raw >= 0) {
					tempArr[i] = vFlex_raw;
				}
			} else if (i == 3 || i == 5 || i == 7) {
				if(vFlex_raw >= 0)
					tempArr[i] = vFlex_raw;
				if(vPot_raw >= 0.1)
					tempArr[i + 1] = vPot_raw;
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
					
					flexFullMatrix[x][y] = tempArr[0];
					flexFlatMatrix[x][y] = tempArr[1];
					flexRestMatrix[x][y] = tempArr[2];
					
					for(int i = 0; i < 3; i++) {
						vFlexMatrix[x][y][i] = tempArr[(i * 2) + 3];
						vPotMatrix[x][y][i] = tempArr[(i * 2) + 4];
					}
					
					flexProbability[x][y] = (((tempArr[2]-((tempArr[3]+tempArr[5]+tempArr[7])/3))/(tempArr[2]-tempArr[0]))*100);
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
	// change cmd window title
	system("title FES Data Gathering Automation v1.4");
	
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
			choice = navigator(1, yPrev, 0, 1, 1, 3, 1, 7, 2); 
			yPrev = choice.yAbs;
					
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
				case 4: // reset program
					breakFlag = 1;
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
	}

}



