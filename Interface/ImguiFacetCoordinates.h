#pragma once
#include "ImguiWindowBase.h"
#include <vector>

class ImFacetCoordinates : public ImWindow {
public:
	void Draw();
protected:
	void DrawTable();
	void ApplyButtonPress();
	void Apply();
	void Insert(int pos=-1);
	void UpdateFromSelection();
	bool ValidateInputs(int idx);
	std::string insertIdInput = "0";
	size_t insertID = 0;
	InterfaceFacet* selFacet = nullptr;
	size_t selFacetId = 0;
	std::string name = "Facet coordinates###FCoords";
	int selRow = -1;

	typedef struct line {
		size_t vertexId=0;
		Vector3d coord;
		std::string coordInput[3];
	};
	std::vector<line> table;
};