#pragma once
#include <string>
#include "../Vector.h"
#include "ImguiWindowBase.h"

//forward declarations of classes used in the implementation
class InterfaceGeometry;
class Interface;

//Facet Move window implemented in ImGui
class ImFacetMove : public ImWindow{
public:
	void Draw();
	void Init(Interface* mApp_, InterfaceGeometry* interfGeom_);
protected:
	void ExecuteFacetMove(bool copy);
	void FacetNormalButtonPress();
	void VertexDirectionButtonPress();
	void FacetCenterButtonPress();
	bool BaseVertexSelectButtonPress();
	bool BaseFacetSelectButtonPress();
	// Internal use variables
	std::string axis_X = "0", axis_Y = "0", axis_Z = "0", distance;
	bool base_selected;
	std::string title;
	int mode;
	Vector3d baseLocation;
	std::string prefix;
	std::string dirMessage;
	std::string selection;
	//enumeratior for movement modes
	enum movementMode { absolute_offset, direction_and_distance };
	Interface* mApp;
	InterfaceGeometry* interfGeom;
};