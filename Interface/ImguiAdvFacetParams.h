#pragma once
#include "ImguiWindowBase.h"

class ImAdvFacetParams : public ImWindow {
public:
	void Draw();
    void Update();
protected:
    void ApplyDrawSettings();
    size_t nbSelected;
    bool enableTexture;
    bool useSquareCells;
    float resolutionA, resolutionB;
    std::string resolutionInA, resolutionInB;
    int cellsX, cellY;
    std::string cellsXIn, cellsYIn;
    bool countDesorption;
    bool countReflection;
    bool countAbsorption;
    bool countTransparentPass;
    bool angularCoefficient;
    bool recordDirectionVectors;

    int memory;
    int cells;
    std::string memoryIn, cellsIn;

    float diffuse, specular, cosine, cosineN;
    std::string diffuseIn, specularIn, cosineIn, cosineNIn;
    float accomodation;
    std::string accommodationIn;
    int teleport;
    std::string teleportIn;
    int structure, link;
    std::string structureIn, linkIn;
    bool movingPart;
    bool wallSojourn;
    float freq, binding;
    std::string freqIn, bindingIn;

    short drawTexture;
    bool drawTextureAllowMixed;
    short drawVolume;
    bool drawVolumeAllowMixed;

    float avg1, avg2, avg3;
    std::string avg1In, avg2In, avg3In;

    bool record;
    float theta, max, nVals, phi;
    std::string thetaIn, maxIn, nValsIn, phiIn;
};