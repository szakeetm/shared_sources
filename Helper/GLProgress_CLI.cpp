#include "GLProgress_CLI.hpp"
#include <iostream>

void GLProgress_CLI::SetMessage(const std::string& msg) 
{
	status = msg;
	std::cout << std::endl << status;
}

void GLProgress_CLI::SetProgress(const double prg) 
{
	int newPrg = static_cast<int>(prg * 100.0);
	if (newPrg != progress) {
		progress = newPrg;
		std::cout << "\r" << status << " [" << progress << "%]";
	}
}
