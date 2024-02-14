#include "GLProgress_abstract.hpp"

double GLProgress_Abstract::GetProgress()
{
	return static_cast<double>(progress) * .01;
}

std::string GLProgress_Abstract::GetMsg()
{
	return status;
}