#pragma once
#include <Helper/GLProgress_abstract.hpp>

//Can also print to cout
class GLProgress_CLI : public GLProgress_Abstract {
public:
	void SetMessage(const std::string& msg, const bool newLine=true) override;
	void SetProgress(const double prg) override;
};