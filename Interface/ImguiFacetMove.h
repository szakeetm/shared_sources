#pragma once
#include <string>
#include "Geometry_shared.h"
#include "../Helper/StringHelper.h"
#include "Facet_shared.h"
#include "imgui.h"
#include "ImguiWindow.h"
#include "ImguiExtensions.h"
#include <imgui/imgui_internal.h>
#include "imgui_stdlib/imgui_stdlib.h"
#include "../../src/MolFlow.h"

//forward declarations of classes used in the implementation
class InterfaceGeometry;
class InterfaceFacet;
class Worker; 
class Interface;

//Facet Move window implemented in ImGui
namespace ImFacetMove {
	// Internal use variables
	static std::string axis_X{"0"}, axis_Y{ "0" }, axis_Z{ "0" }, distance;
	static bool base_selected;
	static std::string title;
	static int mode;
	static Vector3d baseLocation;
	static std::string prefix;
	static std::string dirMessage;
	static std::string selection;
	//enumeratior for movement modes
	enum movementMode { absolute_offset, direction_and_distance };

	void ShowAppFacetMove(bool* p_open, Interface* mApp, InterfaceGeometry* interfGeom);

	static struct {
		float posX, posY, sizeX, sizeY;
	} window;
}