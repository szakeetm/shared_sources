#pragma once
#include <string>
#include "../Vector.h"

//forward declarations of classes used in the implementation
class InterfaceGeometry;
class Interface;

//Facet Move window implemented in ImGui
class ImFacetMove {
public:
	void Show();
	void Draw(Interface* mApp, InterfaceGeometry* interfGeom);
protected:
	bool drawn = false;

	void ExecuteFacetMove(Interface* mApp, InterfaceGeometry* interfGeom, bool copy);
	void FacetNormalButtonPress(Interface* mApp, InterfaceGeometry* interfGeom);
	void VertexDirectionButtonPress(Interface* mApp, InterfaceGeometry* interfGeom);
	void FacetCenterButtonPress(Interface* mApp, InterfaceGeometry* interfGeom);
	bool BaseVertexSelectButtonPress(Interface* mApp, InterfaceGeometry* interfGeom);
	bool BaseFacetSelectButtonPress(Interface* mApp, InterfaceGeometry* interfGeom);
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
};