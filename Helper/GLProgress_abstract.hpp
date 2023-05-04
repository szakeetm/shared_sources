//Either implemented as console or GUI progressbar (GLProgress_CLI or GLProgress_GUI)
#pragma once
#include <string>

class GLProgress_Abstract {
protected:
	int progress = 0;
	std::string status;
public:
	virtual void SetMessage(const std::string& msg);
	virtual void SetProgress(const double prg);
	virtual double GetProgress();
	virtual std::string GetMsg();
	GLProgress_Abstract() = default;
	GLProgress_Abstract(const std::string msg) : status(msg) {}
};
