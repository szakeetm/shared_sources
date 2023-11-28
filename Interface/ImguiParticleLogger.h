#include "ImguiWindowBase.h"

class ImParticleLogger : public ImWindow {
public:
	void Draw();
private:
	bool enableLogging;
	std::string facetNumInput = "";
	int facetNum = -1;
	std::string maxRecInput = "10000";
	size_t maxRec = 10000;
};