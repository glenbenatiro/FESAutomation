// UI.h


void mainMenu_UI()
{
	cls();
	
	cout << "Subject  : " << name 					<< "\n";
	cout << "E2 Offset: " << electrodeOffset 		<< "\n\n";	
	
	cout << "[ ] Gather data" 						<< "\n";
	cout << "[ ] Export and show data" 				<< "\n";
	cout << "[ ] DEBUG: Populate with random data"	<< "\n";
	cout << "[ ] Reset program" 					<< "\n";
	cout << "[ ] Exit"								<< "\n";
}


// draws electrode matrix UI
void dataGathering_UI(int xStep, int yStep)
{
	cls();

	// init var
	char l;
	int c = 0, r = 0;

	for(c = 0; c < COLUMNS; c++) {
		// update column letter
		l = 65 + c;
		
		for(r = 0; r < ROWS - electrodeOffset; r++) {
			// write cell address and [ ] 
			xy((c * xStep), (r * yStep));
			cout << l << ROWS - r;
			
			xy((c * xStep), (r * yStep) + 1);
			cout << "[ ]";
		}
	}
	
	// write back
	c = 0;
	
	xy((c * xStep), (r * yStep));
	cout << "Back";
			
	xy((c * xStep), (r * yStep) + 1);
	cout << "[ ]";
}

void isElectrodeRead_UI(int xStep, int yStep)
{
	for(int c = 0; c < COLUMNS; c++) {
		for(int r = 0; r < ROWS - electrodeOffset; r++) {
			xy((c * xStep) + 3, (r * yStep));
			if(isElectrodeRead[c][ROWS - 1 - r]) {
				cout << "+";
			}
		}
	}
}


void dataGatheringSession_UI(double *Vpot, double *Vflex)
{
	cls();
		
	for(int c = 0; c < 3; c++) {
		cout << "Vflex " << c + 1 << ": " << endl;
		cout << "Vpot  " << c + 1 << ": " << endl << endl;
	}
}

void updateDataGatheringSession(double *Vpot, double *Vflex)
{
	int y = 0;
	
	for(int c = 0; c < 3; c++)
	{
		xy(9, y);
		cout << fixed << setprecision(3) << Vflex[c] << " V";
		xy(9, y + 1);
		cout << fixed << setprecision(3) << Vpot[c] << " V";
		
		y += 3;
	}
}
