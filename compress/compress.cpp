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
#ifdef _WIN32
#include <Windows.h> //Showwindow
#endif

#include <errno.h>
#include <filesystem>
#include <fmt/core.h>

#define ERR_INC_ARG 1
#define ERR_NO_CMPR 2
#define ERR_UNRSLVD 42

int main(int argc, char* argv[]) {
	std::cout << "MolFlow / SynRad wrapper for 7-zip executable\n";
	std::cout << "Renames a file, compresses it and on success it deletes the original.\n\n";
	char key;
	bool autoclose = false;
	std::string result;
	std::vector<std::string> args;
	for (int i = 0; i < argc; i++) {
		std::cout << argv[i] << " ";
		args.push_back(argv[i]);
	}
	std::cout << "\n\n";
	if (argc < 3 || argc>5) {
		std::cerr<<"Incorrect number of arguments\nUsage: compress FILE_TO_COMPRESS NEW_NAME_NAME_IN ARCHIVE  [@include_file_list.txt] [autoclose]\n";
		return ERR_INC_ARG;
	}
	else {
		if (args.back() == "autoclose") {
			autoclose = true;
			args.pop_back(); //autoclose arg processed, treat the rest in common code
		}
		if (args.size() == 4 && args[3][0] != '@') {
			std::cerr << "Incorrect arguments\nUsage: compress FILE_TO_COMPRESS NEW_NAME_NAME_IN ARCHIVE  [@include_file_list.txt] [autoclose]\n";
#ifdef _WIN32
			if (!autoclose) {
				std::cout << "Type any letter and press ENTER to quit\n";
				ShowWindow(GetConsoleWindow(), SW_RESTORE);
				std::cin >> key;
			}
#endif
			return ERR_INC_ARG;
		}
	}
	std::string command;
	std::string fileName;
	std::string fileNameWith7z;
	std::string fileNameGeometry;
	fileName = args[1];
	std::cout << "\nFile to compress: " << args[1];
	std::cout << "\nNew name in archive: " << args[2] << "\n";
	if (args.size() == 4) std::cout << "Additional file list: " << args[3] << "\n";

	fileNameWith7z = fileName + "7z";
	std::string sevenZipName;
#ifdef _WIN32
	sevenZipName += "7za.exe";
#else //Linux, MacOS
	sevenZipName = "7za"; //so that Exist() check fails and we get an error message on the next command
	std::string possibleLocations[] = { "./7za", //use 7za binary shipped with Molflow
									   "/usr/bin/7za", //use p7zip installed system-wide
									   "/usr/local/bin/7za", //use p7zip installed for user
									   "/opt/homebrew/bin/7za" }; //homebrew on Apple chip mac
	for (auto& path : possibleLocations) {
		if (FileUtils::Exist(path)) {
			sevenZipName = path;
		}
	}
#endif
	if (!FileUtils::Exist(sevenZipName)) {
		std::cerr << "\n{} not found. Cannot compress.\n", sevenZipName;
#ifdef _WIN32
		if (!autoclose) {
			std::cout << "Type any letter and press ENTER to quit\n";
			std::cin >> key;
		}
#endif
		return ERR_NO_CMPR;
	}

	fileNameGeometry = FileUtils::GetPath(fileName) + args[2];
	std::filesystem::rename(fileName, fileNameGeometry);
	std::filesystem::remove(fileNameWith7z);
	command = "";

#ifdef _WIN32
	//Trick so Windows command line supports UNC (network) paths
	std::string cwd = FileUtils::get_working_path();
	command += "cmd /C \"pushd \"" + cwd + "\"&&";
#endif

	command += sevenZipName + " u -t7z \"" + fileNameWith7z + "\" \"" + fileNameGeometry + "\"";
	if (args.size() == 4) { //include file list
		command = command + " " + args[3];
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
#ifdef _WIN32
	command += "&&popd\"";
#endif

	std::cout << "\nCommand:\n" << command << "\n\nStarting compression...";
#ifdef _WIN32
	std::cout << "\nYou can continue using Molflow/Synrad while compressing.\n"; //On Windows, compress.exe is launched as a background process
#endif
	result = FileUtils::exec(command);

	if (args.size() == 4) { //Delete list file
		std::string listFile = args[3];
		listFile = listFile.substr(1, std::string::npos);
		std::filesystem::remove(listFile);
	}

	int found;
	found = result.find("Everything is Ok");
	if (found != std::string::npos) {
		std::cout << "\nCompression seems legit. Deleting original {} file.\n", std::filesystem::path(fileNameGeometry).extension().string().c_str();
		std::filesystem::remove(fileNameGeometry);
		return 0;
	}

	//Handle errors:
#ifdef _WIN32
	ShowWindow(GetConsoleWindow(), SW_RESTORE); //Make window visible on error
#endif
	std::filesystem::rename(fileNameGeometry, fileName);
	std::cerr << fmt::format("\nSomething went wrong during the compression, read above. {} file kept.\n", std::filesystem::path(fileNameGeometry).extension().string().c_str());
#ifdef _WIN32
	if (!autoclose) {
		std::cout<<"Type any letter and press Enter to exit\n";
		std::cin >> key;
	}
#endif
	return ERR_UNRSLVD;
}
