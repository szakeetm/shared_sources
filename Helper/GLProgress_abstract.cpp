#include "GLProgress_abstract.hpp"

double GLProgress_Abstract::GetProgress()
{
	return static_cast<double>(progress) * .01;
}

void GLProgress_Abstract::SetMessage(const std::string& msg, const bool newLine)
{
	status = msg;
}

std::string GLProgress_Abstract::GetMsg()
{
	return status;
}

void GLProgress_Abstract::SetProgress(const double prg)
{
	int newPrg = static_cast<int>(prg * 100.0);
	if (newPrg != progress) {
		progress = newPrg;
	}
}