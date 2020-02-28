#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main()
{
	string line;
	char temp[50];
	char strTemp[50];
	
	ifstream inFile ("input.html");
	ofstream outFile ("output.txt");
	
	if(inFile.is_open()) {
		while(getline(inFile,line)) {
			for(int c = 0; c < sizeof(temp); c++) {
				temp[c] = line[c];
			}
			
			sprintf(strTemp, "file << \"%s\" << \"\\n\";", temp);
			outFile << strTemp;
			outFile << "\n";
		}
		
		inFile.close();
    	outFile.close();
	} else {
		cout << "Cannot open file.";
	}
	
	return 0;
}
