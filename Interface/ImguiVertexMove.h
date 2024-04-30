#pragma once
#include "ImguiWindowBase.h"

class ImVertexMove : public ImWindow {
public:
	void Draw();
protected:

	void FacetNormalButtonPress();
	void BaseSelVertButtonPress();
	void BaseFacCentButtonPress();
	void DirSelVertButtonPress();
	void DirFacCentButtonPress();
	void ApplyButtonPress(bool copy);

	enum MovementMode {
		absOffset,
		directionDist
	};
	MovementMode mode = absOffset;
	std::string xIn, yIn, zIn, dIn;
	double x, y, z, d;
	std::string baseMsg="Nothing chosen", dirMsg = "Choose base first";
};