// auxil.h

// --- ANALOG DISCOVER 2 --- 
void calcSFreq()
{
	int val, count = 0;
	
	// init sFreq values to zero
	for(int i = 0; i < 5; i++)
		sFreq[i] = 0;
	
	for(int c = 0; c < COLUMNS; c++) {
		for(int r = ROWS - 1; r > electrodeOffset - 1; r--) {
			
			val = round(flexBendingPercentage[c][r]);
			
			// ---
			if(val > 10){
				sFreq[0]++;
				if(val > 20){
					sFreq[1]++;
					if(val > 25){
						sFreq[2]++;
						if(val > 50){
							sFreq[3]++;
							if(val > 75){
								sFreq[4]++;
							}
						}
					}
				}
			}
			// ---
			
			count++;
		}
	}
	
	// if electrode size if big, only 1 value is counted per big location
	if(e2Size == 2) {
		for(int i = 0; i < 5; i++) {
			sFreq[i] /= 4;
		}
	}
}

double ad2_readAnalogIOVoltage(int channel)
{
	double temp;
	FDwfAnalogInStatus(hdwf, false, NULL);
	FDwfAnalogInStatusSample(hdwf, channel, &temp);
	
	return temp;
}

bool ifsleep(float seconds) {
	bool res = false;
    clock_t startClock = clock();
    float secondsAhead = seconds * CLOCKS_PER_SEC;
    // do nothing until the elapsed time has passed.
    if (clock() < startClock + secondsAhead)
    	res = true;
    return res;
}

string rgb2hex(int r, int g, int b, bool with_head)
{
	stringstream ss;
	
  	if (with_head)
    	ss << "#";
    
	ss << hex << (r << 16 | g << 8 | b );

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
selectionMatrix navigator(int xInit, int yInit, int xStep, int yStep, int xMin, int yMin, int xMax, int yMax, int mode)
{
	// init vars
	int x = xInit;
	int y = yInit;
	int choice = 0;
	selectionMatrix coord;
	// ---
	
	if(!xStep)
		xStep = 1;
	if(!yStep)
		yStep = 1;

	// draw initial 'x' mark
	xy(x,y);
	cout << "x";
		
	while(1) {
		if(choice = getch()) {
			
			// remove 'x' mark from current position
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
		        	
		        	coord.xAbs = x;
		        	coord.yAbs = y;
		        	
		        	if(!mode) {
		        		coord.x = x;
		        		coord.y = y;
					} else {
						coord.x = 0;
						coord.y = 0;
						
						for(int xCount = xMin; xCount <= x; xCount += xStep) {
							coord.x += 1;
						}
						for(int yCount = yMin; yCount <= y; yCount += yStep) {
							coord.y += 1;
						}
						
						if(mode == 1) {
							coord.x--;
							coord.y--;
						}			
					}
					
					return coord;
		        	break;
		        default:
		            break;
			}
			
			// write 'x' mark in new position
			xy(x,y);
	        cout << "x";
		}
	}
}

void generateHTMLFile()
{
	// init vars
	int color;
	char colChar = 'A';
	
	// create and open html file for writing
	char fileName[50];
	sprintf(fileName, "results/html/%s_%d_HTML.html", name, electrodeOffset);
	file.open(fileName);

	// HTML CODE START
	// to change HTML code below, please open input.txt in HTML folder
	// then remove all identions using Shift+Tab, then use HTMLtoCode.exe
	// to convert it. grab code from output.txt. then tweak as necessary
	
	file << "<!DOCTYPE html>" 	<< "\n";
	file << "<html>" 			<< "\n";
	file << "<head>" 			<< "\n";
	
	// css code
	file << "<style>" 						<< "\n";
	file << "body {" 						<< "\n";
	file << "font: 16px Arial, sans-serif;" << "\n";
	file << "}" 							<< "\n";
	file << "table {" 						<< "\n";
	file << "border-collapse: collapse;" 	<< "\n";
	file << "}" 							<< "\n";
	file << "table td {" 					<< "\n";
	file << "width: 50px;" 					<< "\n";
	file << "height: 50px;" 				<< "\n";
	file << "text-align: center;" 			<< "\n";
	file << "border: 1px solid black;" 		<< "\n";
	file << "}" 							<< "\n";
	file << "table tr td:first-child {" 	<< "\n";
	file << "border-top: 0;" 				<< "\n";
	file << "border-left: 0;" 				<< "\n";
	file << "border-bottom: 0;" 			<< "\n";
	file << "font-size: 75%;" 				<< "\n";
	file << "}" 							<< "\n";
	file << "table tr:last-child td {" 		<< "\n";
	file << "border-left: 0;" 				<< "\n";
	file << "border-bottom: 0;" 			<< "\n";
	file << "border-right: 0;" 				<< "\n";
	file << "font-size: 75%;" 				<< "\n";
	file << "}" 							<< "\n";
	file << "</style>" 						<< "\n";
	// ---

	file << "</head>" 						<< "\n";
	file << "<body>" 						<< "\n";
	file << "<center>" 						<< "\n";
	sprintf(temp, "Heat Map of %s", name);
	file << temp;
	file << "<br><br>" 						<< "\n";
	file << "Electrode size: ";
	if(e2Size == 1) {
		file << "Small (2cm x 2cm)";
	} else if ( e2Size == 2) {
		file << "Big (3.9cm x 3.9cm)";
	}
	file << "\n" << "<br>" << "\n";
	sprintf(temp, "Start time: %s", startTime);
	file << temp;
	file << "<br>"<< "\n";
	sprintf(temp, "End time: %s", endTime);
	file << temp;
	file << "<br><br>" 						<< "\n";
	file << "<table>" 						<< "\n";
	
	// loop to write values in table
	for(int r = ROWS - 1; r >= 0; r--) {
		
		file << "<tr>" << "\n";
		
		// write row label
		file << "<td>" << r + 1 << "</td>" << "\n";
		
		for(int c = 0; c < COLUMNS; c++) {
				
			// write code for cell color
			file << "<td bgcolor=\"";
			float tempVal = round(flexBendingPercentage[c][r]);
			
			if (r > (electrodeOffset - 1)) {
				if(tempVal < 0 && tempVal >= -100)
					file << rgb2hex(255, 255 - ((tempVal + 100) * 1.28), 255 - ((tempVal + 100) * 1.28), 1);	// shades of 'pastel' red for -1 to -100
				else if(tempVal >= 0 && tempVal <= 100)
					file << rgb2hex(255 - (tempVal * 1.28), 255, 255 - (tempVal * 1.28), 1);					// shades of 'pastel' green for 0 to 100
				else 
					file << rgb2hex(128, 128, 255, 1);															// pastel blue for all other
			} else if (r == (electrodeOffset - 1)) {
				file << rgb2hex(128, 128, 128, 1);	
			} else {
				file << rgb2hex(128, 128, 128, 1);	
			}
			file << "\">" << "\n";
			
			// write cell value
			if(r > (electrodeOffset - 1)){
				if(isElectrodeRead[c][r])
					file << round(flexBendingPercentage[c][r]);
				else
					file << "-";
			}
			else if(r == (electrodeOffset - 1))
				file << "<b>-</b>";
			else
				file << "X";
			file << "</td>" << "\n";			
		}
		file << "</tr>" << "\n";
	}
	
	// write label for columns
	file << "<tr>" << "\n";
	for(int c = 0; c < COLUMNS + 1; c++) {
		if(!c)
			file << "<td></td>" << "\n";
		else
			file << "<td>" << char(colChar + c - 1) << "</td>" << "\n";
	}
	file << "</tr>" << "\n";
	// ---
	
	file << "</table>" 		<< "\n";
	file << "<br>" 			<< "\n";
	file << "<br>" 			<< "\n";
	file << "</center>" 	<< "\n";
	file << "</body>" 		<< "\n";
	file << "</html>" 		<< "\n";
	
	// close file after writing data
	file.close();
	
	// open html file for viewing
	sprintf(fileName, "results\\%s_%d_HTML.html", name, electrodeOffset);
   	ShellExecute(NULL, "open", fileName, NULL, NULL, SW_SHOWNORMAL);
}

void generateCSVFile()
{
	// init var
	char fileName[50];
	char letter = 'A';
	
	// create file then open	
	sprintf(fileName, "results/csv/%s_%d_CSV.csv", name, electrodeOffset);
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
			file << flexBendingPercentage[c][r] << ",";
			
			file << "\n"; 
		}
		file << "\n";
		letter++;
	}
	
	file << "S.Freq 10%, S.Freq 20%, S.Freq 25%, S.Freq 50%, S.Freq 75%" << "\n";
	file << sFreq[0] << "," << sFreq[1] << "," << sFreq[2] << "," << sFreq[3] << "," << sFreq[4] << "\n\n";
	
	file << "Name:," << name << ",," << "Start time:" << "," << startTime;
	file << "Electrode 2 location:" << "," << electrodeOffset << ",," << "End time:" << "," << endTime;

	// close file
	file.close();
	
	// open created .csv file
//	sprintf(fileName, "results\\%s_%d_CSV.csv", name, electrodeOffset);
//	ShellExecute(NULL, "open", fileName, NULL, NULL, SW_SHOWNORMAL);
}

bool isADConnected()
{
	// close if something else opened AD previously
	FDwfDeviceCloseAll();
	
	char szError[512] = { 0 };
    char szDeviceName[32];
    int prevDuty = 0;
	
    if(!FDwfDeviceOpen(-1, &hdwf)) {
        FDwfGetLastErrorMsg(szError);

        printf("\nDevice open failed\n%s", szError);
        
        cout << "\n\n";
        cout << "Press any key to exit.";
        
        getch();
        return 0;
    }
    else {	
        return 1;
    }
}

// --- function to fill flexBendingPercentage matrix with random values
void dataSeeder()
{
	for(int c = 0; c < COLUMNS; c++) {
		for(int r = electrodeOffset - 1; r < ROWS; r++) {
			isElectrodeRead[c][r] = true;
			flexBendingPercentage[c][r] = rand() % 101;
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
	
	// switch for waveform generator output
	FDwfAnalogOutConfigure(hdwf, 0, state);
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
