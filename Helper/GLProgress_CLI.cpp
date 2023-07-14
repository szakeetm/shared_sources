#include "GLProgress_CLI.hpp"
#include <iostream>

GLProgress_CLI::GLProgress_CLI(const std::string& message)
{
	SetMessage(message,true);
}

void GLProgress_CLI::SetMessage(const std::string& msg, const bool newLine, const bool forceDraw)
{
	//forceDraw is ignored, only for GUI
	status = msg;
	if (newLine) {
		std::cout << std::endl;
	}
	else {
		std::cout << "\r";
	}
	std::cout << status;
}

void GLProgress_CLI::SetProgress(const double prg) 
{
	int newPrg = static_cast<int>(prg * 100.0);
	if (newPrg != progress) {
		progress = newPrg;
		std::cout << "\r" << status << " [" << progress << "%]";
	}
}
