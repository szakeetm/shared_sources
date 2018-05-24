/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <iostream>
#define NOMINMAX
#include <Windows.h> //Showwindow
#include <io.h>
#include <direct.h>
#include <errno.h>

std::string exec(std::string command);
std::string exec_str(const char* cmd);
bool Exist(std::string fileName);
bool Exist(const char *fileName);
std::string GetPath(const std::string& str);

int main(int argc,char* argv[]) {
	char key;
	std::string result;
	for (int i = 0; i < argc; i++) {
		std::cout << argv[i] << " ";
	}
	std::cout << "\n\n";
	if (argc<3) {
		std::cout<<"Usage: compress.exe FILE_TO_COMPRESS RENAMED_FILENAME {include_file1 include_file2 ...}";
		ShowWindow( GetConsoleWindow(), SW_RESTORE );
		std::cin>>key;
		return 0;
	}
	std::string command;
	std::string fileName;
	std::string fileNameWith7z;
	std::string fileNameGeometry;
	fileName = argv[1];
	std::cout<<"\nargv0: "<<argv[0];
	std::cout<<"\nargv1: "<<argv[1];
	std::cout<<"\nargv2: "<<argv[2]<<"\n";
	fileNameWith7z = fileName + "7z";
	if (!Exist("7za.exe")) {
		printf("\n7za.exe not found. Cannot compress.\n");
			std::cin>>key;
			return 0;
	}
	
	fileNameGeometry = GetPath(fileName) + argv[2];
	command = "move \"" + fileName + "\" \"" + fileNameGeometry + "\"";
	result=exec(command);
	char CWD [MAX_PATH];
	_getcwd( CWD, MAX_PATH );
	command = "del \""+fileNameWith7z+"\"";
	result = exec(command); 
	command = "cmd /C \"pushd \"";
	command+=CWD;
	command+="\"&&7za.exe u -t7z \"" + fileNameWith7z + "\" \"" + fileNameGeometry + "\"";
	for (int i=3;i<argc;i++) { //include files
		bool duplicate=false;
		for (int j=3;!duplicate && j<i;j++) { //check for duplicate include files
			if (strcmp(argv[i],argv[j])==0)
				duplicate=true;
		}
		if (!duplicate) {
			command += " \"";
			command += argv[i];
			command += "\""; //add as new input file
		}
	}
	command+="&&popd\"";
	std::cout<<"\nCommand: "<<command<<"\n\nStarting compression...\nYou can continue using Synrad while compressing.\n";
	result=exec(command);
	size_t found;
	found=result.find("Everything is Ok");
	if (found!=std::string::npos) {
		printf("\nCompression seems legit. Deleting GEO file.");
		remove(fileNameGeometry.c_str());
		return 0;
	}
	ShowWindow( GetConsoleWindow(), SW_RESTORE );
	command = "move \""+fileNameGeometry+"\" \""+fileName+"\"";
	result=exec(command);
	printf("\nSomething went wrong during the compression, read above. GEO file kept."
		"\nType any letter and press Enter to exit\n");
	std::cin>>key;
	return 0;
}

std::string exec(std::string command) {
	return exec_str(command.c_str());
}

std::string exec_str(const char* cmd) {
    FILE* pipe = _popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
                result += buffer;
		        printf(buffer);
    }
	result=result+'0';
    _pclose(pipe);
    return result;
}

bool Exist(std::string fileName) {
	return Exist(fileName.c_str());
}

bool Exist(const char *fileName) {
	FILE *filepoint;

	if ((fopen_s(&filepoint, fileName, "r")) != 0) {
		return false;
	}
	else {
		return true;
	}
}

std::string GetPath(const std::string& str)
{
	size_t found = str.find_last_of("/\\");
	if (found == std::string::npos) return ""; //not found, return empty string
	else return str.substr(0, found)+"\\";
}