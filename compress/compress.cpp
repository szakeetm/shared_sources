/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
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
#include "File.h"
#include <cstring>
#define NOMINMAX
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <Windows.h> //Showwindow
#endif

#include <errno.h>
#include <filesystem>
#include "Helper/ConsoleLogger.h"

#define ERR_INC_ARG 1
#define ERR_NO_CMPR 2
#define ERR_UNRSLVD 42

std::string exec(const std::string& command);
std::string exec(const char* cmd);

int main(int argc,char* argv[]) {
	std::cout << "MolFlow / SynRad wrapper for 7-zip executable\n";
	std::cout << "Renames a file, compresses it and on success it deletes the original.\n\n";
	char key;
	std::string result;
	for (int i = 0; i < argc; i++) {
		std::cout << argv[i] << " ";
	}
	std::cout << "\n\n";
	if (argc < 3 || argc>4 || (argc == 4 && argv[3][0] != '@')) {
        Log::console_error("Incorrect arguments\nUsage: compress FILE_TO_COMPRESS NEW_NAME_NAME_IN ARCHIVE  [@include_file_list.txt]\n");
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
        Log::console_error("Type any letter and press ENTER to quit\n");
		ShowWindow( GetConsoleWindow(), SW_RESTORE );
        std::cin>>key;
#endif
		return ERR_INC_ARG;
	}
	std::string command;
	std::string fileName;
	std::string fileNameWith7z;
	std::string fileNameGeometry;
	fileName = argv[1];
	std::cout << "\nFile to compress: " << argv[1];
	std::cout << "\nNew name in archive: " << argv[2] << "\n";
	if (argc == 4) std::cout << "Additional file list: " << argv[3] << "\n";

	fileNameWith7z = fileName + "7z";
	std::string sevenZipName;
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    sevenZipName += "7za.exe";
#else //Linux, MacOS
    sevenZipName = "7za"; //so that Exist() check fails and we get an error message on the next command
    std::string possibleLocations[] = {"./7za", //use 7za binary shipped with Molflow
                                       "/usr/bin/7za", //use p7zip installed system-wide
                                       "/usr/local/bin/7za", //use p7zip installed for user
                                       "/opt/homebrew/bin/7za"}; //homebrew on M1 mac
    for(auto& path : possibleLocations){
        if (FileUtils::Exist(path)) {
            sevenZipName = path;
        }
    }
#endif
	if (!FileUtils::Exist(sevenZipName)) {
        Log::console_error("\n{} not found. Cannot compress.\n", sevenZipName);
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
        Log::console_error("Type any letter and press ENTER to quit\n");
        std::cin>>key;
#endif
        return ERR_NO_CMPR;
	}
	
	fileNameGeometry = FileUtils::GetPath(fileName) + argv[2];
	/*
	command = "move \"" + fileName + "\" \"" + fileNameGeometry + "\"";
	result=exec(command);
	*/
	std::filesystem::rename(fileName, fileNameGeometry);
	std::filesystem::remove(fileNameWith7z);
	command = "";

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
	//Trick so Windows command line supports UNC (network) paths
	std::string cwd = FileUtils::get_working_path();
	command += "cmd /C \"pushd \"" + cwd + "\"&&";
#endif

	command+=sevenZipName + " u -t7z \"" + fileNameWith7z + "\" \"" + fileNameGeometry + "\"";
	if (argc==4) { //include file list
		command = command + " " + argv[3];
		/*
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
		*/
	}
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
	command+="&&popd\"";
#endif

	std::cout << "\nCommand:\n" << command << "\n\nStarting compression...";
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
	std::cout << "\nYou can continue using Molflow/Synrad while compressing.\n"; //On Windows, compress.exe is launched as a background process
#endif
	result=exec(command);

	if (argc == 4) { //Delete list file
		std::string listFile = argv[3];
		listFile = listFile.substr(1, std::string::npos);
		std::filesystem::remove(listFile);
	}

	size_t found;
	found=result.find("Everything is Ok");
	if (found!=std::string::npos) {
        Log::console_error("\nCompression seems legit. Deleting original {} file.\n",std::filesystem::path(fileNameGeometry).extension().string().c_str());
		std::filesystem::remove(fileNameGeometry);
		return 0;
	}

	//Handle errors:
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
	ShowWindow( GetConsoleWindow(), SW_RESTORE ); //Make window visible on error
#endif
	std::filesystem::rename(fileNameGeometry, fileName);
	Log::console_error("\nSomething went wrong during the compression, read above. {} file kept.\n" ,std::filesystem::path(fileNameGeometry).extension().string().c_str());
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    Log::console_error("Type any letter and press Enter to exit\n");
    std::cin>>key;
#endif
	return ERR_UNRSLVD;
}

std::string exec(const std::string& command) {
	return exec(command.c_str());
}

std::string exec(const char* cmd) { //Execute a command and return what it prints to the command line / terinal
    FILE* pipe = 
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
		_popen
#else
		popen
#endif
		(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
                result += buffer;
		        printf("%s",buffer);
    }
	result=result+'0';
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
	_pclose
#else
	pclose
#endif
	(pipe);
    return result;
}
