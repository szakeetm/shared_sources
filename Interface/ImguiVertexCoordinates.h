#pragma once
#include "ImguiWindowBase.h"

class ImVertexCoordinates : public ImWindow {
public:
	void Draw();
	void UpdateFromSelection(const std::vector<size_t>& selectedVertices);
	void UpdateFromSelection();
protected:
	void DrawTable();
	void ApplyButtonPress();
	void Apply();
	struct vCoords {
		size_t vertexId;
		std::string xIn, yIn, zIn;
		double x, y, z;
	};
	enum Axis {
		X,
		Y,
		Z
	};
	void SetAllTo(Axis axis, std::string val);
	std::vector<vCoords> data;
};