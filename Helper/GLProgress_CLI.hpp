#pragma once
#include <Helper/GLProgress_abstract.hpp>

//Can also print to cout
class GLProgress_CLI : public GLProgress_Abstract {
public:
	GLProgress_CLI(const std::string& message);
	~GLProgress_CLI();
	void SetMessage(const std::string& msg, const bool newLine=false, const bool forceDraw=false) override;
	void SetProgress(const double prg) override;
};