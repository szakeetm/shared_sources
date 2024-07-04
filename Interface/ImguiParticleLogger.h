#pragma once
#include "ImguiWindowBase.h"
#if defined(MOLFLOW)
#include "../../src/Simulation/MolflowSimGeom.h"
#endif

class ImParticleLogger : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp_);
	void UpdateStatus();
	void OnShow() override;
private:
	void ApplyButtonPress();
	void UpdateMemoryEstimate();
	std::string LogToText(const std::string& separator = "\t", FILE* file = nullptr);
	bool enableLogging = false;
	std::string facetNumInput = "";
	size_t facetNum = -1;
	std::string maxRecInput = "10000";
	size_t maxRec = 10000;
	bool isRunning = false;
	std::string memUse = "No estimate";
	std::string statusLabel = "No recording.";
	std::shared_ptr<ParticleLog> log;
};