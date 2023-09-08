#pragma once
#include <string>
#include "Geometry_shared.h"
static std::string axis_X, axis_Y, axis_Z, distance;
static bool base_selected{ false };

class InterfaceGeometry;
class Worker;

class MolFlow;
void ShowAppFacetMove(bool* p_open, MolFlow* mApp, InterfaceGeometry* interfGeom);

// Internal use variables
enum movementMode { absolute_offset, direction_and_distance };
static int mode;
static std::string prefix{"d"};
static std::string selection{""};
static Vector3d baseLocation;

	