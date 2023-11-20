#include "ImguiWindowBase.h"
#include "../../src/MolflowGeometry.h"

class ImTextureScailing : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp_);
protected:
	void UpdateSize();
	void Update();

	void SetCurrent();
	void Apply();
	bool WorkerUpdate();
	void DrawGradient();
	std::vector<ImU32> GenerateColorMap();

	std::vector<ImU32> colorMap;

	MolflowGeometry* molflowGeom;

	std::string minInput = "0", maxInput = "1";
	double minScale = 0, maxScale = 1;
	bool autoscale = true, colors = true, logScale = true;
	short includeComboVal = 1, showComboVal=0;
	std::string includeComboLabels[3] = {"Only moments", "Include constant flow", "Only constant flow"};
	std::string showComboLabels[3] = {"Pressure [mbar]", u8"Impingement rate [1/sec/m\u00b2]", u8"Particle density [1/m\u00b3]"};
	std::string swapText = "0 bytes";
};