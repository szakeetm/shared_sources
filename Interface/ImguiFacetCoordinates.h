#pragma once
#include "ImguiWindowBase.h"
#include <vector>

class ImFacetCoordinates : public ImWindow {
public:
	void Draw();
	void OnShow() override;
	void UpdateFromSelection(const std::vector<size_t>& selectedFacets);
	void UpdateFromSelection();
	void UpdateFromVertexSelection();
protected:
	void DrawTable();
	void ApplyButtonPress();
	void Apply();
	void Insert(int pos=-1);
	bool ValidateInputs(int idx);
	enum Axis : short {X,Y,Z};
	Axis axis = X;
	void SetAllTo(std::string val);
	std::string insertIdInput = "0";
	size_t insertID = 0;
	InterfaceFacet* selFacet = nullptr;
	long long selFacetId = 0;
	std::string name = "Facet coordinates###FCoords";
	int selRow = -1;

	typedef struct line {
		size_t vertexId=0;
		Vector3d coord;
		std::string coordInput[3];
	};
	std::vector<line> table;
};