#pragma once
#include "ImguiWindowBase.h"

class ImAdvFacetParams : public ImWindow {
public:
	void Draw();
    void Update();
protected:
    void ApplyDrawSettings();
    void ExportToCSVButtonPress();
    void CopyButtonPress();
    void ReleaseRecordedButtonPress();
    void ImportCVSButtonPress();
    void ForceRemeshButtonPress();
    void ApplyTexture(bool force = false);
    void UpdateMemoryEstimate();
    void UpdateCellCount();
    void EditCellCount();
    void CalcSojournTime();
    void Apply();
    std::pair<double, double> GetRatioForNbCell(size_t nbCellsU, size_t nbCellsV);
    size_t nbSelected;
    short enableTexture;
    bool enableTextureAllowMixed;
    short useSquareCells;
    bool useSquareCellsAllowMixed;
    float resolutionU, resolutionV;
    std::string resolutionUIn, resolutionVIn;
    float cellSizeU, cellSizeV;
    std::string cellSizeUIn, cellSizeVIn;
    int cellsU, cellsV;
    std::string cellsUIn, cellsVIn;
    short countDesorption;
    bool countDesorptionAllowMixed;
    short countReflection;
    bool countReflectionAllowMixed;
    short countAbsorption;
    bool countAbsorptionAllowMixed;
    short countTransparentPass;
    bool countTransparentPassAllowMixed;
    short angularCoefficient;
    bool angularCoefficientAllowMixed;
    short recordDirectionVectors;
    bool recordDirectionVectorsAllowMixed;

    std::string memory, cells;

    float diffuse, specular, cosine, cosineN;
    std::string diffuseIn, specularIn, cosineIn, cosineNIn;
    float accomodation;
    std::string accommodationIn;
    int teleport;
    std::string teleportIn;
    int structure, link;
    std::string structureIn, linkIn;
    bool movingPart;

    short wallSojourn;
    bool wallSojournAllowMixed;
    double freq, binding;
    std::string freqIn, bindingIn, sojournText = "Wall sojourn time";

    short drawTexture;
    bool drawTextureAllowMixed;
    short drawVolume;
    bool drawVolumeAllowMixed;

    float avg1, avg2, avg3;
    std::string avg1In, avg2In, avg3In;

    bool record;
    float theta, max, nVals, phi;
    std::string thetaIn, maxIn, nValsIn, phiIn;

    friend class ImSidebar;
};