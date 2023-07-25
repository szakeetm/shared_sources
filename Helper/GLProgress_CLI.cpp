#include "GLProgress_CLI.hpp"
#include <iostream>

GLProgress_CLI::~GLProgress_CLI() {
	std::cout << std::endl;
}

GLProgress_CLI::GLProgress_CLI(const std::string& message)
{
	SetMessage(message,false);
}

void GLProgress_CLI::SetMessage(const std::string& msg, const bool newLine, const bool forceDraw)
{
	//forceDraw is ignored, only for GUI
	if (newLine) {
		status = msg;
		std::cout << std::endl << status;
	}
	else {
		std::string suffix;
		int suffixLength = ((int)status.length()+10) - (int)msg.length(); //the old status, with the percent(approx 10 char) needs to be printed over
		if (suffixLength > 0) suffix = std::string(suffixLength, ' ');
		status = msg;
		std::cout << "\r" << status << suffix;
	}
}

void GLProgress_CLI::SetProgress(const double prg) 
{
	int newPrg = static_cast<int>(prg * 100.0);
	if (!noProgress && newPrg != progress) {
		progress = newPrg;
		std::cout << "\r" << status << " [" << progress << "%]";
	}
}
