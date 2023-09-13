#pragma once
#include <string>
#include "Geometry_shared.h"
#include "../Helper/StringHelper.h"
#include "Facet_shared.h"
#include "imgui.h"
#include "ImguiExtensions.h"
#include <imgui/imgui_internal.h>
#include "imgui_stdlib/imgui_stdlib.h"
#include "../../src/MolFlow.h"
#include <optional>


//forward declarations of classes used in the implementation
class InterfaceGeometry;
class InterfaceFacet;
class Worker; 
class MolFlow;

//Facet Move window implemented in ImGui

// Internal use variables
static std::string axis_X{"0"}, axis_Y{ "0" }, axis_Z{ "0" }, distance;
static bool base_selected;
static bool popup;
static int mode;
static Vector3d baseLocation;
static std::string prefix;
static std::string dirMessage;
static std::string message;
static std::string selection;
//enumeratior for movement modes
enum movementMode { absolute_offset, direction_and_distance };

//internal function declarations
void popupToggle();
void executeMove(MolFlow* mApp, InterfaceGeometry* interfGeom, bool copy);
void facetNormalButton(InterfaceGeometry* interfGeom);
void dirVertex(InterfaceGeometry* interfGeom);
void dirFacetCenter(InterfaceGeometry* interfGeom);
bool baseVertexSelect(InterfaceGeometry* interfGeom);
bool baseFacetSelect(InterfaceGeometry* interfGeom);

void ShowAppFacetMove(bool* p_open, MolFlow* mApp, InterfaceGeometry* interfGeom);
