#include<iostream>
#include<windows.h>
#include<cstdlib>
#include<stdio.h>
#include<stdlib.h>
#include<conio.h>

#include "sample.h"

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define ENTER 13

using namespace std;

class selectionMatrix {
	public:
		int x, y;
} choice;

class UI
{

};

// global variables
// ---for AD
char szError[512] = { 0 };
char szDeviceName[32];


void cls()
{
	system("cls");
}

void mainMenu_UI()
{
	cout << "[x] Gather Data" << "\n";
	cout << "[ ] Save to Text File" << "\n";
	cout << "[ ] Exit";
}

void dataGathering_UI()
{
	
}

void gotoxy(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// does navigation in any UI page
selectionMatrix navigator(int xInit, int yInit, int xStep, int yStep, int xMin, int yMin, int xMax, int yMax)
{
	int x = xInit;
	int y = yInit;
	int choice = 0;
	
	// check if xStep or yStep is zero, then change it to 1 to avoid error in return value of selectionMatrix 
	if(xStep == 0)
		xStep = 1;
	if(yStep == 0)
		yStep = 1;
	
	while(1) {
		if(choice = getch()) {
			gotoxy(x, y);
			cout << " ";
			
			switch(choice)
			{
			    case KEY_UP:
			    	if(y == yMin)
			    		y = yMax;
			    	else
			    		y -= yStep;
		            break;
		        case KEY_DOWN:
		            if(y == yMax)
		            	y = yMin;
		            else
		            	y += yStep;
		            break;
		        case KEY_LEFT:
			    	if(x == xMin)
			    		x = xMax;
			    	else
			    		x -= xStep;
		            break;
		        case KEY_RIGHT:
		            if(x == xMax)
		            	x = xMin;
		            else
		            	x += xStep;
		            break;
		        case ENTER:
		        	struct selectionMatrix choice;	        	
		        	choice.x = x/xStep;
		        	choice.y = y/yStep;
		        	return choice;
		        	break;
		        default:
		            break;
			}
			
			gotoxy(x,y);
	        cout << "x";
		}
	}
}

// draws electrode matrix UI
void drawUI()
{
	int c,r;
	char l;

	// columns
	for(c = 1; c <= 5; c++) {
		switch(c) {
			case 1:
				l = 'A';
				break;
			case 2:
				l = 'B';
				break;
			case 3:
				l = 'C';
				break;
			case 4:
				l = 'D';
				break;
			case 5:
				l = 'E';
				break;	
		}
		
		gotoxy(4*(c-1),0);
				
		for(r = 13; r > 0; r--) {
			gotoxy(4*(c-1), (-3*(r-13)));
			cout << l << r;
			gotoxy(4*(c-1), (-3*(r-13))+1);
			cout << "[ ]";
		}
	}
	
	gotoxy(1, 1);
	cout << "x";
}

void sp(int x)
{
	int c;
		
	for(c = 0; c < x; c++)
		cout << " ";
}

void updatePos()
{
	
}

void recordingSession()
{
	cout << "PRESS...";
	cout << "\n";
	cout << "A TO CAPTURE VALUES";
	cout << "B TO REMOVE PREV VALUES";	
}

void initAD()
{
	HDWF hdwf;
	double hzSys;
	char szError[512] = { 0 };
    char szDeviceName[32];
    int prevDuty = 0;
	
	printf("Opening Device");
    if (!FDwfDeviceOpen(-1, &hdwf)) {
        FDwfGetLastErrorMsg(szError);

        printf("\nDevice open failed\n%s", szError);
        
        cout << "\n\n";
        cout << "Press any key to exit.";
        getch();
    }
    else {
        FDwfEnumDeviceName(0, szDeviceName);
        printf("\nDevice Name: %s", szDeviceName);
        
        cout << "\n\n";
        cout << "Press any key to continue...";
        getch();
    }
}

int main()
{
	system("title FES");
	int x = 1, y = 1, c;
	selectionMatrix choice;
	
	// initialize analog discovery
	initAD();
	
	while(1) {
		// main menu
		mainMenu_UI();
		choice = navigator(1, 0, 0, 1, 1, 0, 1, 2); 
		
		cout << choice.x << choice.y;
		
	
//		drawUI();
//		c = navigator(1, 1, 4, 3, 1, 1, 17, 37);

		getch();
		
	}

}

