//
// Created by Pascal Baehr on 14.06.22.
//

#include "SimulationFacet.h"
#include <Polygon.h>
#include <Helper/MathTools.h>

bool SimulationFacet::InitializeLinkAndVolatile(const size_t & id)
{
    if (sh.superDest || sh.isVolatile) {
        // Link or volatile facet, overides facet settings
        // Must be full opaque and 0 sticking
        // (see SimulationMC.c::PerformBounce)
        //sh.isOpaque = true;
        sh.opacity = 1.0;
        sh.sticking = 0.0;
    }
    return true;
}

size_t SimulationFacet::InitializeProfile() {
    size_t profileSize = 0;

    //Profiles
    if (sh.isProfile) {
        profileSize = PROFILE_SIZE * sizeof(ProfileSlice);
        try {
            //profile = std::vector<std::vector<ProfileSlice>>(1 + nbMoments, std::vector<ProfileSlice>(PROFILE_SIZE));
        }
        catch (...) {
            throw std::runtime_error("Not enough memory to load profiles");
            return false;
        }
    }
    else
        profileSize = 0;
    return profileSize;
}

std::vector<double> SimulationFacet::InitTextureMesh()
{
    GLAppPolygon P1, P2;
    double sx, sy;
    double iw = 1.0 / (double)sh.texWidth_precise;
    double ih = 1.0 / (double)sh.texHeight_precise;
    double rw = sh.U.Norme() * iw;
    double rh = sh.V.Norme() * ih;
    double fullCellArea = iw*ih;

    std::vector<Vector2d>(4).swap(P1.pts);
    //P1.sign = 1;
    P2.pts = vertices2;
    //P2.sign = -sign;

    std::vector<double> interCellArea;
    interCellArea.resize(sh.texWidth * sh.texHeight, -1.0); //will shrink at the end

    for (size_t j = 0;j < sh.texHeight;j++) {
        sy = (double)j;
        for (size_t i = 0;i < sh.texWidth;i++) {
            sx = (double)i;

            bool allInside = false;
            double u0 = sx * iw;
            double v0 = sy * ih;
            double u1 = (sx + 1.0) * iw;
            double v1 = (sy + 1.0) * ih;
            //mesh[i + j*wp.texWidth].elemId = -1;

            if (sh.nbIndex <= 4) {

                // Optimization for quad and triangle
                allInside = IsInPoly(Vector2d(u0,v0), vertices2)
                            && IsInPoly(Vector2d(u0, v1), vertices2)
                            && IsInPoly(Vector2d(u1, v0), vertices2)
                            && IsInPoly(Vector2d(u1, v1), vertices2);

            }

            if (!allInside) {
                double area{};

                // Intersect element with the facet (facet boundaries)
                P1.pts[0] = {u0, v0};
                P1.pts[1] = {u1, v0};
                P1.pts[2] = {u1, v1};
                P1.pts[3] = {u0, v1};

                std::vector<bool>visible(P2.pts.size());
                std::fill(visible.begin(), visible.end(), true);
                auto [A,center,vList] = GetInterArea(P1, P2, visible);
                if (!IsZero(A)) {

                    if (A > (fullCellArea + 1e-10)) {

                        // Polyon intersection error !
                        // Switch back to brute force
                        auto [bfArea, center_loc] = GetInterAreaBF(P2, Vector2d(u0, v0), Vector2d(u1, v1));
                        bool fullElem = IsZero(fullCellArea - bfArea);
                        if (!fullElem) {
                            interCellArea[i + j*sh.texWidth] = (bfArea*(rw*rh) / (iw*ih));
                        }
                        else {
                            interCellArea[i + j*sh.texWidth] = -1.0;
                        }

                        //cellprop.full = IsZero(fullCellArea - A);

                    }
                    else {

                        bool fullElem = IsZero(fullCellArea - A);
                        if (!fullElem) {
                            // !! P1 and P2 are in u,v coordinates !!
                            interCellArea[i + j*sh.texWidth] = (A*(rw*rh) / (iw*ih));
                        }
                        else {
                            interCellArea[i + j*sh.texWidth] = -1.0;
                        }

                    }

                }
                else interCellArea[i + j*sh.texWidth] = -2.0; //zero element

            }
            else {  //All indide and triangle or quad
                interCellArea[i + j*sh.texWidth] = -1.0;
            }
        }
    }

    return interCellArea;
}

size_t SimulationFacet::InitializeTexture(){
    size_t textureSize = 0;
    //Textures
    if (sh.isTextured) {
        size_t nbE = sh.texWidth*sh.texHeight;
        largeEnough.resize(nbE);
        textureSize = nbE * sizeof(TextureCell);
        /*try {
            texture = std::vector<std::vector<TextureCell>>(1 + nbMoments, std::vector<TextureCell>(nbE));
        }
        catch (...) {
            throw std::runtime_error("Not enough memory to load textures");
            return false;
        }*/
        // Texture increment of a full texture element
        double fullSizeInc = (sh.texWidth_precise * sh.texHeight_precise) / (sh.U.Norme() * sh.V.Norme());
        for (size_t j = 0; j < nbE; j++) { //second pass, filter out very small cells
            largeEnough[j] = textureCellIncrements[j] < (5.0*fullSizeInc);
        }

        //double iw = 1.0 / (double)sh.texWidth_precise;
        //double ih = 1.0 / (double)sh.texHeight_precise;
        //double rw = sh.U.Norme() * iw;
        //double rh = sh.V.Norme() * ih;
    }
    else
        textureSize = 0;
    return textureSize;
}

size_t SimulationFacet::InitializeDirectionTexture(){
    size_t directionSize = 0;

    //Direction
    if (sh.countDirection) {
        directionSize = sh.texWidth*sh.texHeight * sizeof(DirectionCell);
        try {
            //direction = std::vector<std::vector<DirectionCell>>(1 + nbMoments, std::vector<DirectionCell>(sh.texWidth*sh.texHeight));
        }
        catch (...) {
            throw std::runtime_error("Not enough memory to load direction textures");
        }
    }
    else
        directionSize = 0;
    return directionSize;
}

/**
* \brief Constructor for cereal initialization
*/
SimulationFacet::SimulationFacet() : Facet() {
    isReady = false;
    globalId = 0;
    isHit = false;
}

/**
* \brief Constructor with initialisation based on the number of indices/facets
* \param nbIndex number of indices/facets
*/
SimulationFacet::SimulationFacet(size_t nbIndex) : Facet(nbIndex) {
    isReady = false;
    globalId = 0;
    isHit = false;
    indices.resize(nbIndex);                    // Ref to Geometry Vector3d
    vertices2.resize(nbIndex);
}

/**
* \brief Calculates the hits size for a single facet which is necessary for hits dataport
* \param nbMoments amount of moments
* \return calculated size of the facet hits
*/
size_t SimulationFacet::GetHitsSize(size_t nbMoments) const { //for hits dataport
    return   (1 + nbMoments)*(
            sizeof(FacetHitBuffer) +
            +(sh.isTextured ? (sh.texWidth*sh.texHeight * sizeof(TextureCell)) : 0)
            + (sh.isProfile ? (PROFILE_SIZE * sizeof(ProfileSlice)) : 0)
            + (sh.countDirection ? (sh.texWidth*sh.texHeight * sizeof(DirectionCell)) : 0)
            + sh.facetHistogramParams.GetDataSize()
    );
}

size_t SimulationFacet::GetMemSize() const {
    size_t sum = 0;
    sum += sizeof (SimulationFacet);
    sum += sizeof (size_t) * indices.capacity();
    sum += sizeof (Vector2d) * vertices2.capacity();
    sum += sizeof (double) * textureCellIncrements.capacity();
    sum += sizeof (bool) * largeEnough.capacity();
    return sum;
}