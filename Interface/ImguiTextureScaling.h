#include "ImguiWindowBase.h"
#include "../../src/MolflowGeometry.h"

class ImTextureScaling : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp_);
	void Load();
	void UpdateSize();
protected:
	void Update();

	void SetCurrentButtonPress();
	void ApplyButtonPress();
	bool WorkerUpdate();
	void DrawGradient();
	void GetCurrentRange();

	double logScaleInterpolate(double x, double leftTick, double rightTick);

	std::vector<ImU32> GenerateColorMap();

	std::vector<ImU32> colorMap;

	MolflowGeometry* molflowGeom;
	bool photoMode = false;
	std::string minInput[3] = { "0", "0", "0" }, maxInput[3] = { "1","1","1" };
	double minScale[3] = { 0,0,0 }, maxScale[3] = { 1,1,1 };
	double cMinScale = 0, cMaxScale = 1;
	bool autoscale = true, colors = true, logScale = true;
	AutoScaleMode includeComboVal = AutoscaleMomentsAndConstFlow;
	short showComboVal=0;
	std::string includeComboLabels[3] = {"Only moments", "Moments+const. flow", "Only constant flow"};
	std::string showComboLabels[3] = {"Pressure [mbar]", u8"Impingement rate [1/sec/m\u00b2]", u8"Particle density [1/m\u00b3]"};
	std::string swapText = "0 bytes";
};