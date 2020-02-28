// auxil.h

// --- ANALOG DISCOVER 2 --- 
double ad2_readAnalogIOVoltage(int channel)
{
	double temp;
	
	FDwfAnalogInStatus(hdwf, false, NULL);
	FDwfAnalogInStatusSample(hdwf, channel, &temp);
	return temp;
}

string rgb2hex(int r, int g, int b, bool with_head)
{
  std::stringstream ss;
  if (with_head)
    ss << "#";
  ss << std::hex << (r << 16 | g << 8 | b );
  return ss.str();
}


void cls()
{
	system("cls");
}


void xy(int x, int y)
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
			xy(x, y);
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
		        	choice.x = (x/xStep) + 1;
		        	choice.y = (y/yStep) + 1;
		        	return choice;
		        	break;
		        default:
		            break;
			}
			
			xy(x,y);
	        cout << "x";
		}
	}
}


void generateHTMLFile()
{
	char fileName[50];
	sprintf(fileName, "results/%s_%d_HTML.html", name, electrodeOffset);
	file.open(fileName);
	int color;
	
	// HTML CODE START
	// to change HTML code below, please open input.txt in HTML folder
	// then remove all identions using Shift+Tab, then use HTMLtoCode.exe
	// to convert it. grab code from output.txt. then tweak as necessary
	
	file << "<!DOCTYPE html>" << "\n";
	file << "<html>" << "\n";
	file << "<head>" << "\n";
	
	
	
	file << "<style>" << "\n";
	file << "body {" << "\n";
	file << "font: 16px Arial, sans-serif;" << "\n";
	file << "}" << "\n";
	file << "table {" << "\n";
	file << "border-collapse: collapse;" << "\n";
	file << "}" << "\n";
	file << "table td {" << "\n";
	file << "width: 50px;" << "\n";
	file << "height: 50px;" << "\n";
	file << "text-align: center;" << "\n";
	file << "border: 1px solid black;" << "\n";
	file << "}" << "\n";
	file << "table tr td:first-child {" << "\n";
	file << "border-top: 0;" << "\n";
	file << "border-left: 0;" << "\n";
	file << "border-bottom: 0;" << "\n";
	file << "font-size: 75%;" << "\n";
	file << "}" << "\n";
	file << "table tr:last-child td {" << "\n";
	file << "border-left: 0;" << "\n";
	file << "border-bottom: 0;" << "\n";
	file << "border-right: 0;" << "\n";
	file << "font-size: 75%;" << "\n";
	file << "}" << "\n";
	file << "</style>" << "\n";

	
	
	file << "</head>" << "\n";
	file << "" << "\n";
	file << "<body>" << "\n";
	file << "<center>" << "\n";
	
	sprintf(temp, "Heat Map of %s", name);
	file << temp;
	
	file << "<br><br>" << "\n";
	
	sprintf(temp, "Start time: %s", startTime);
	file << temp;
	
	file << "<br>" << "\n";
	
	sprintf(temp, "End time: %s", endTime);
	file << temp;
	file << "<br><br>" << "\n";
	file << "<table>" << "\n";
	
	for(int r = ROWS - 1; r >= 0; r--) {
		file << "<tr>" << "\n";
		
		// write row label
		file << "<td>" << r << "</td>" << "\n";
		
		for(int c = 0; c < COLUMNS; c++) {
			
			// calculate color value
			color = 255 - round(flexProbability[c][r]*2.57);
			
			char buffer[10];
			itoa(color, buffer, 16);
			
			// GREEN NI NGA COLOR HEHE xd kapoy code tarong
			file << "<td bgcolor=\"#";	
			if(r > (electrodeOffset - 1)) {
				file << buffer;
				file << "ff";
				file << buffer;			
			} else if (r == (electrodeOffset - 1)) {
				file << "c80000";
			} else {
				file << "c8c8c8";
			}
			file << "\">" << "\n";
			if(r > (electrodeOffset - 1))
				file << round(flexProbability[c][r]);
			else if(r == (electrodeOffset - 1))
				file << "<b>-</b>";
			else
				file << "X";
			file << "</td>" << "\n";			
		}
		file << "</tr>" << "\n";
	}
	
	char colChar = 'A';
	
	// write label for columns
	file << "<tr>" << "\n";
	for(int c = 0; c < COLUMNS + 1; c++) {
		if(!c)
			file << "<td></td>" << "\n";
		else
			file << "<td>" << char(colChar + c - 1) << "</td>" << "\n";
	}
	file << "</tr>" << "\n";
	
	file << "</table>" << "\n";
	file << "" << "\n";
	file << "<br>" << "\n";
	file << "<br>" << "\n";
//	file << "second electrode offset: 2" << "\n";
//	file << "<br>" << "\n";
//	file << "flex flat value: 1.23" << "\n";
//	file << "<br>" << "\n";
//	file << "flex full value: 1.23" << "\n";
//	file << "" << "\n";
	file << "</center>" << "\n";
	file << "</body>" << "\n";
	file << "" << "\n";
	file << "</html>" << "\n";

	// HTML CODE END
	sprintf(fileName, "results\\%s_%d_HTML.html", name, electrodeOffset);
   	ShellExecute(NULL, "open", fileName, NULL, NULL, SW_SHOWNORMAL);
                
	file.close();

}


void generateCSVFile()
{
	// init var
	char fileName[50];
	char letter = 'A';
	
	// create file then open	
	sprintf(fileName, "results/%s_%d_CSV.csv", name, electrodeOffset);
	file.open(fileName);
	
	// write column headings to file
	file << "Electrode 1,Electrode 2,,,vFlex,,,,vPot,,,Flat,Resting,Fully Bended,,Prob" << "\n";
	
	// main loop to write to file
	for(int c = 0; c < COLUMNS; c++) {
		for(int r = ROWS - 1; r >= electrodeOffset; r--) {
			file << letter << r + 1 << ",";
			file << letter << electrodeOffset << ",,";
			
			for(int t = 0; t < TRIALS; t++) {
				file << vFlexMatrix[c][r][t] << ",";
			}
			
			file << ",";
			
			for(int t = 0; t < TRIALS; t++) {
				file << vPotMatrix[c][r][t] << ",";
			}
			
			file << "," << flexFlatMatrix[c][r] << ",";
			file << flexRestMatrix[c][r] << ",";
			file << flexFullMatrix[c][r] << ",,";
			file << flexProbability[c][r] << ",";
			
			file << "\n"; 
		}
		file << "\n";
		letter++;
	}
	
	file << "\n";
	file << "Subject:" << "," << name << ",,";
	file << "Start time:" << "," << startTime << "\n";
	file << "Electrode 2 location:" << "," << electrodeOffset << ",,";
	file << "End time:" << "," << endTime << "\n";

	// close file
	file.close();
	
	// open created .csv file
	sprintf(fileName, "results\\%s_%d_CSV.csv", name, electrodeOffset);
   	ShellExecute(NULL, "open", fileName, NULL, NULL, SW_SHOWNORMAL);
}


bool isADConnected()
{
	system("title FES Data Gathering Automation");
	
	// close if something else opened AD previously
	FDwfDeviceCloseAll();
	
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
        return 0;
    }
    else {
        FDwfEnumDeviceName(0, szDeviceName);
        printf("\nDevice Name: %s", szDeviceName);
        
        cout << "\n\n";
        cout << "Press any key to continue...";
        
        while(!getch()) {
        	
		}
		
        return 1;
    }
}


int getFlexSensorValues(int x, int y)
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
	
	// draw UI
	xy(0,0); cout << "Cell " << char(65 + x) << y + 1; 
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
					flexFullMatrix[x][y] = tempArr[0];
					flexFlatMatrix[x][y] = tempArr[1];
					flexRestMatrix[x][y] = tempArr[2];
					
					for(int i = 0; i < 3; i++) {
						vFlexMatrix[x][y][i] = tempArr[(i * 2) + 3];
						vPotMatrix[x][y][i] = tempArr[(i * 2) + 4];
					}
					
					flexProbability[x][y] = (((tempArr[2]-((tempArr[3]+tempArr[5]+tempArr[7])/3))/(tempArr[2]-tempArr[0]))*100);
					breakFlag = 0;
				}
				break;
				
			default:
				break;
		}
	}
	
	return 1;
}


// --- function to fill flexProbability matrix with random values
void dataSeeder()
{
	for(int c = 0; c < COLUMNS; c++) {
		for(int r = 0; r < ROWS; r++) {
			flexProbability[c][r] = rand() % 100;
		}
	}
}


void ad2_enableMasterSwitches(bool state)
{
	// start/stop acquisition
	FDwfAnalogInConfigure(hdwf, 0, state);
	
	// master switch for 5V supply
	FDwfAnalogIOEnableSet(hdwf, state);
	
	// master switch for digital outputs
	FDwfDigitalOutConfigure(hdwf, state);
}


void getTime(char *timeVar)
{
	// declaring argument for time() 
    time_t tt; 
  
    // declaring variable to store return value of localtime() 
    struct tm * ti; 
  
    // applying time() 
    time (&tt); 
  
    // using localtime() 
    ti = localtime(&tt); 
  
  	// store human readable time format in specified string
  	sprintf(timeVar, "%s", asctime(ti));
}
