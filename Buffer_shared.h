/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/
#pragma once
#include "Vector.h"
#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <array>

#if defined(MOLFLOW)
#include "../src/MolflowTypes.h" //Texture Min Max of GlobalHitBuffer, anglemapparams
#endif

#if defined(SYNRAD)
#include "../src/SynradTypes.h" //Texture Min Max of GlobalHitBuffer
#endif

#define PROFILE_SIZE  (size_t)100 // Size of profile
#define LEAKCACHESIZE     (size_t)2048  // Leak history max length
#define HITCACHESIZE      (size_t)2048  // Max. displayed number of lines and hits.
//#define MAX_STRUCT 64

class HistogramParams {
public:
	bool recordBounce=false;
	size_t nbBounceMax=10000;
	size_t nbBounceBinsize=1;
	bool recordDistance = false;
	double distanceMax=10.0;
	double distanceBinsize=0.001;
#if defined(MOLFLOW)
	bool recordTime = false;
	double timeMax=0.1;
	double timeBinsize=1E-5;
#endif

	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(
			CEREAL_NVP(recordBounce),
			CEREAL_NVP(nbBounceMax),
			CEREAL_NVP(nbBounceBinsize),
			CEREAL_NVP(recordDistance),
			CEREAL_NVP(distanceMax),
			CEREAL_NVP(distanceBinsize)
#if defined(MOLFLOW)
			, CEREAL_NVP(recordTime)
			, CEREAL_NVP(timeMax)
			, CEREAL_NVP(timeBinsize)
#endif
		);
	}

	size_t GetBounceHistogramSize() const {
		return nbBounceMax / nbBounceBinsize + 1 + 1; //+1: overrun
	}
	size_t GetDistanceHistogramSize() const {
		return (size_t)(distanceMax / distanceBinsize) + 1; //+1: overrun
	}
#if defined(MOLFLOW)
	size_t GetTimeHistogramSize() const {
		return (size_t)(timeMax / timeBinsize) + 1; //+1: overrun
	}
#endif
	size_t GetDataSize() const {
		size_t size = 0;
		if (recordBounce) size += sizeof(double) * GetBounceHistogramSize();
		if (recordDistance) size += sizeof(double) * GetDistanceHistogramSize();
#if defined(MOLFLOW)
		if (recordTime) size += sizeof(double) * GetTimeHistogramSize();
#endif
		return size;
		
	}
	size_t GetBouncesDataSize() const {
		if (!recordBounce) return 0;
		else return sizeof(double)*GetBounceHistogramSize();
	}
	size_t GetDistanceDataSize() const {
		if (!recordDistance) return 0;
		else return sizeof(double)*GetDistanceHistogramSize();
	}
#if defined(MOLFLOW)
	size_t GetTimeDataSize() const {
		if (!recordTime) return 0;
		else return sizeof(double)*GetTimeHistogramSize();
	}
#endif
};

class FacetProperties { //Formerly SHFACET
public:
	//For sync between interface and subprocess
	double sticking;       // Sticking (0=>reflection  , 1=>absorption)   - can be overridden by time-dependent parameter
	double opacity;        // opacity  (0=>transparent , 1=>opaque)
	double area;           // Facet area (m^2)

	int    profileType;    // Profile type
	int    superIdx;       // Super structure index (Indexed from 0) -1: facet belongs to all structures (typically counter facets)
	size_t    superDest;      // Super structure destination index (Indexed from 1, 0=>current)
	int	 teleportDest;   // Teleport destination facet id (for periodic boundary condition) (Indexed from 1, 0=>none, -1=>teleport to where it came from)

	bool   countAbs;       // Count absorption (MC texture)
	bool   countRefl;      // Count reflection (MC texture)
	bool   countTrans;     // Count transparent (MC texture)
	bool   countDirection;

	HistogramParams facetHistogramParams;

	// Flags
	bool   is2sided;     // 2 sided
	bool   isProfile;    // Profile facet
	bool   isTextured;   // texture
	bool   isVolatile;   // Volatile facet (absorbtion facet which does not affect particule trajectory)

						 // Geometry
	size_t nbIndex;   // Number of index/vertex
	//double sign;      // Facet vertex rotation (see Facet::DetectOrientation())

					  // Plane basis (O,U,V) (See Geometry::InitializeGeometry() for info)
	Vector3d   O;  // Origin
	Vector3d   U;  // U vector
	Vector3d   V;  // V vector
	Vector3d   nU; // Normalized U
	Vector3d   nV; // Normalized V

				   // Normal vector
	Vector3d    N;    // normalized
	Vector3d    Nuv;  // normal to (u,v) not normlized

					  // Axis Aligned Bounding Box (AxisAlignedBoundingBox)
	AxisAlignedBoundingBox       bb;
	Vector3d   center;

	// Hit/Abs/Des/Density recording on 2D texture map
	size_t    texWidth;    // Rounded texture resolution (U)
	size_t    texHeight;   // Rounded texture resolution (V)
	double texWidth_precise;   // Actual texture resolution (U)
	double texHeight_precise;  // Actual texture resolution (V)

	size_t   hitOffset;      // Hit address offset for this facet

#if defined(MOLFLOW)
							 // Molflow-specific facet parameters
	double temperature;    // Facet temperature (Kelvin)                  - can be overridden by time-dependent parameter
	double outgassing;           // (in unit *m^3/s)                      - can be overridden by time-dependent parameter

	int sticking_paramId;    // -1 if use constant value, 0 or more if referencing time-dependent parameter
	int opacity_paramId;     // -1 if use constant value, 0 or more if referencing time-dependent parameter
	int outgassing_paramId;  // -1 if use constant value, 0 or more if referencing time-dependent parameter

	int CDFid; //Which probability distribution it belongs to (one CDF per temperature)
	int IDid;  //If time-dependent desorption, which is its ID

	int    desorbType;     // Desorption type
	double desorbTypeN;    // Exponent in Cos^N desorption type
	Reflection reflection;

	bool   countDes;       // Count desoprtion (MC texture)

	bool   countACD;       // Angular coefficient (AC texture)
	double maxSpeed;       // Max expected particle velocity (for velocity histogram)
	double accomodationFactor; // Thermal accomodation factor [0..1]
	bool   enableSojournTime;
	double sojournFreq, sojournE;

	// Facet hit counters
	// FacetHitBuffer tmpCounter; - removed as now it's time-dependent and part of the hits buffer

	// Moving facets
	bool isMoving;

	//Outgassing map
	bool   useOutgassingFile;   //has desorption file for cell elements
	double outgassingFileRatioU,outgassingFileRatioV; //desorption file's sample/unit ratio
	size_t   outgassingMapWidth; //rounded up outgassing file map width
	size_t   outgassingMapHeight; //rounded up outgassing file map height

	double totalOutgassing; //total outgassing for the given facet

	AnglemapParams anglemapParams;//Incident angle map
#endif

#if defined(SYNRAD)
	int    doScattering;   // Do rough surface scattering
	double rmsRoughness;   // RMS height roughness, in meters
	double autoCorrLength; // Autocorrelation length, in meters
	int    reflectType;    // Reflection type. 0=Diffuse, 1=Mirror, 10,11,12... : Material 0, Material 1, Material 2...., 9:invalid 
	bool   recordSpectrum;    // Calculate energy spectrum (histogram)
#endif


	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(
			CEREAL_NVP(sticking),       // Sticking (0=>reflection  , 1=>absorption)   - can be overridden by time-dependent parameter
			CEREAL_NVP(opacity),        // opacity  (0=>transparent , 1=>opaque)
			CEREAL_NVP(area),          // Facet area (m^2)

			CEREAL_NVP(profileType),    // Profile type
			CEREAL_NVP(superIdx),       // Super structure index (Indexed from 0)
			CEREAL_NVP(superDest),      // Super structure destination index (Indexed from 1, 0=>current)
			CEREAL_NVP(teleportDest),   // Teleport destination facet id (for periodic boundary condition) (Indexed from 1, 0=>none, -1=>teleport to where it came from)

			CEREAL_NVP(countAbs),       // Count absoprtion (MC texture)
			CEREAL_NVP(countRefl),      // Count reflection (MC texture)
			CEREAL_NVP(countTrans),     // Count transparent (MC texture)
			CEREAL_NVP(countDirection),

			CEREAL_NVP(facetHistogramParams),

			// Flags
			CEREAL_NVP(is2sided),     // 2 sided
			CEREAL_NVP(isProfile),    // Profile facet
			CEREAL_NVP(isTextured),   // texture
			CEREAL_NVP(isVolatile),   // Volatile facet (absorbtion facet which does not affect particule trajectory)

							  // Geometry
			CEREAL_NVP(nbIndex),   // Number of index/vertex
			//CEREAL_NVP(sign),      // Facet vertex rotation (see Facet::DetectOrientation())

							 // Plane basis (O,U,V) (See Geometry::InitializeGeometry() for info)
			CEREAL_NVP(O),  // Origin
			CEREAL_NVP(U),  // U vector
			CEREAL_NVP(V),  // V vector
			CEREAL_NVP(nU), // Normalized U
			CEREAL_NVP(nV), // Normalized V

						// Normal vector
			CEREAL_NVP(N),    // normalized
			CEREAL_NVP(Nuv),  // normal to (u,v) not normlized

						  // Axis Aligned Bounding Box (AxisAlignedBoundingBox)
			CEREAL_NVP(bb),
			CEREAL_NVP(center),

			// Hit/Abs/Des/Density recording on 2D texture map
			CEREAL_NVP(texWidth),    // Rounded texture resolution (U)
			CEREAL_NVP(texHeight),   // Rounded texture resolution (V)
			CEREAL_NVP(texWidth_precise),   // Actual texture resolution (U)
			CEREAL_NVP(texHeight_precise),  // Actual texture resolution (V)

			CEREAL_NVP(hitOffset)      // Hit address offset for this facet

#if defined(MOLFLOW)
								 // Molflow-specific facet parameters
			, CEREAL_NVP(temperature),    // Facet temperature (Kelvin)                  - can be overridden by time-dependent parameter
			CEREAL_NVP(outgassing),           // (in unit *m^3/s)                      - can be overridden by time-dependent parameter

			CEREAL_NVP(sticking_paramId),    // -1 if use constant value, 0 or more if referencing time-dependent parameter
			CEREAL_NVP(opacity_paramId),     // -1 if use constant value, 0 or more if referencing time-dependent parameter
			CEREAL_NVP(outgassing_paramId),  // -1 if use constant value, 0 or more if referencing time-dependent parameter

			CEREAL_NVP(CDFid), //Which probability distribution it belongs to (one CDF per temperature)
			CEREAL_NVP(IDid),  //If time-dependent desorption, which is its ID

			CEREAL_NVP(desorbType),     // Desorption type
			CEREAL_NVP(desorbTypeN),    // Exponent in Cos^N desorption type
			CEREAL_NVP(reflection),

			CEREAL_NVP(countDes),       // Count desoprtion (MC texture)

			CEREAL_NVP(countACD),       // Angular coefficient (AC texture)
			CEREAL_NVP(maxSpeed),       // Max expected particle velocity (for velocity histogram)
			CEREAL_NVP(accomodationFactor), // Thermal accomodation factor [0..1]
			CEREAL_NVP(enableSojournTime),
			CEREAL_NVP(sojournFreq),
			CEREAL_NVP(sojournE),

			// Facet hit counters
			// FacetHitBuffer tmpCounter, - removed as now it's time-dependent and part of the hits buffer

			// Moving facets
			CEREAL_NVP(isMoving),

			//Outgassing map
			CEREAL_NVP(useOutgassingFile),   //has desorption file for cell elements
			CEREAL_NVP(outgassingFileRatioU), //desorption file's sample/unit ratio U
			CEREAL_NVP(outgassingFileRatioV), //desorption file's sample/unit ratio V
			CEREAL_NVP(outgassingMapWidth), //rounded up outgassing file map width
			CEREAL_NVP(outgassingMapHeight), //rounded up outgassing file map height

			CEREAL_NVP(totalOutgassing), //total outgassing for the given facet

			CEREAL_NVP(anglemapParams)//Incident angle map
#endif
			

#if defined(SYNRAD)
			,
			CEREAL_NVP(doScattering),   // Do rough surface scattering
			CEREAL_NVP(rmsRoughness),   // RMS height roughness, in meters
			CEREAL_NVP(autoCorrLength), // Autocorrelation length, in meters
			CEREAL_NVP(reflectType),    // Reflection type. 0=Diffuse, 1=Mirror, 10,11,12... : Material 0, Material 1, Material 2...., 9:invalid 
			CEREAL_NVP(recordSpectrum)    // Calculate energy spectrum (histogram)
#endif
		);
	}
};

struct WorkerParams { //Plain old data
	HistogramParams globalHistogramParams;
#if defined(MOLFLOW)
	double latestMoment;
	double totalDesorbedMolecules; //Number of molecules desorbed between t=0 and latest_moment
	double finalOutgassingRate; //Number of outgassing molecules / second at latest_moment (constant flow)
	double finalOutgassingRate_Pa_m3_sec;
	double gasMass;
	bool enableDecay;
	double halfLife;
	double timeWindowSize;
	bool useMaxwellDistribution; //true: Maxwell-Boltzmann distribution, false: All molecules have the same (V_avg) speed
	bool calcConstantFlow;

	int motionType;
	Vector3d motionVector1; //base point for rotation
	Vector3d motionVector2; //rotation vector or velocity vector
	size_t    sMode;                // Simu mode (MC_MODE or AC_MODE)
#endif
#if defined(SYNRAD)
	size_t        nbRegion;  //number of magnetic regions
	size_t        nbTrajPoints; //total number of trajectory points (calculated at CopyGeometryBuffer)
	bool       newReflectionModel;
#endif

	template <class Archive> void serialize(Archive & archive) {
		archive(
#if defined(MOLFLOW)
			CEREAL_NVP(globalHistogramParams)


			, CEREAL_NVP(latestMoment)
			, CEREAL_NVP(totalDesorbedMolecules)
			, CEREAL_NVP(finalOutgassingRate)
			, CEREAL_NVP(gasMass)
			, CEREAL_NVP(enableDecay)
			, CEREAL_NVP(halfLife)
			, CEREAL_NVP(timeWindowSize)
			, CEREAL_NVP(useMaxwellDistribution)
			, CEREAL_NVP(calcConstantFlow)

			, CEREAL_NVP(motionType)
			, CEREAL_NVP(motionVector1)
			, CEREAL_NVP(motionVector2)
			, CEREAL_NVP(sMode)
#endif

#if defined(SYNRAD)
			CEREAL_NVP(nbRegion)  //number of magnetic regions
			, CEREAL_NVP(nbTrajPoints) //total number of trajectory points (calculated at CopyGeometryBuffer)
			, CEREAL_NVP(newReflectionModel)
#endif
		);
	}
};

class GeomProperties {  //Formerly SHGEOM
public:
	GeomProperties () : nbFacet(0),nbVertex(0),nbSuper(0),name(""){};
	size_t     nbFacet;   // Number of facets (total)
	size_t     nbVertex;  // Number of 3D vertices
	size_t     nbSuper;   // Number of superstructures
	std::string name;  // (Short file name)

	template <class Archive> void serialize(Archive & archive) {
		archive(
			CEREAL_NVP(nbFacet),   // Number of facets (total)
			CEREAL_NVP(nbVertex),  // Number of 3D vertices
			CEREAL_NVP(nbSuper),   // Number of superstructures
			CEREAL_NVP(name)  // (Short file name)
		);
	}
};

class OntheflySimulationParams {
public:
#if defined(SYNRAD)
	int      generation_mode; // Fluxwise/powerwise
#endif
	bool	 lowFluxMode;
	double	 lowFluxCutoff;

	bool enableLogging;
	size_t logFacetId, logLimit;

	size_t desorptionLimit;
	size_t nbProcess; //For desorption limit / log size calculation

	template<class Archive> void serialize(Archive& archive) {
		archive(
#if defined(SYNRAD)
			CEREAL_NVP(generation_mode),
#endif
			CEREAL_NVP(lowFluxMode),
			CEREAL_NVP(lowFluxCutoff),
			CEREAL_NVP(enableLogging),
			CEREAL_NVP(logFacetId),
			CEREAL_NVP(logLimit),
			CEREAL_NVP(desorptionLimit),
			CEREAL_NVP(nbProcess)
		);
	}

}; //parameters that can be changed without restarting the simulation

class HIT {
public:
	Vector3d pos;
	int    type;
#if defined(SYNRAD)
	double dF;
	double dP;
#endif
	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(
			CEREAL_NVP(pos),
			CEREAL_NVP(type)
#if defined(SYNRAD)
			,CEREAL_NVP(dF),
			CEREAL_NVP(dP)
#endif
		);
	}
};

// Velocity field
class DirectionCell {
public:
	DirectionCell& operator+=(const DirectionCell& rhs) {
		this->dir += rhs.dir;
		this->count += rhs.count;
		return *this;
	}
	DirectionCell& operator+(const DirectionCell& rhs) {
		*this += rhs;
		return *this;
	}
	Vector3d dir = Vector3d(0.0,0.0,0.0);
	size_t count=0;
	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(
			CEREAL_NVP(dir), 
			CEREAL_NVP(count)
		);
	}
};

class LEAK {
public:
	Vector3d pos;
	Vector3d dir;
	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(
			CEREAL_NVP(pos),
			CEREAL_NVP(dir)
		);
	}
};

class FacetHistogramBuffer { //raw data containing histogram result
public:
	void Resize(const HistogramParams& params);
    void Reset();

    FacetHistogramBuffer& operator+=(const FacetHistogramBuffer& rhs);
	std::vector<double> nbHitsHistogram;
	std::vector<double> distanceHistogram;
	std::vector<double> timeHistogram;
	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(
			CEREAL_NVP(nbHitsHistogram), 
			CEREAL_NVP(distanceHistogram), 
			CEREAL_NVP(timeHistogram)
		);
	}
};

#if defined(MOLFLOW)
struct FacetHitBuffer {

    FacetHitBuffer() {
        hit.nbDesorbed = 0;
        hit.nbMCHit = 0;
        hit.nbHitEquiv = 0.0;
        hit.nbAbsEquiv = 0.0;
        hit.sum_1_per_ort_velocity = 0.0;
        hit.sum_1_per_velocity = 0.0;
        hit.sum_v_ort = 0.0;
    }
	struct {
		// Counts
        size_t nbDesorbed;          // Number of desorbed molec
        size_t nbMCHit;               // Number of hits
		double nbHitEquiv;			//Equivalent number of hits, used for low-flux impingement rate and density calculation
		double nbAbsEquiv;          // Equivalent number of absorbed molecules
		double sum_1_per_ort_velocity;    // sum of reciprocials of orthogonal velocity components, used to determine the density, regardless of facet orientation
		double sum_1_per_velocity;          //For average molecule speed calculation
		double sum_v_ort;          // sum of orthogonal speeds of incident velocities, used to determine the pressure
	} hit;

	/*struct {
		// density
		double desorbed;
		double value;
		double absorbed;
	} density;*/

	//TODO: Check for correct serialization
	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(
			CEREAL_NVP(hit.nbDesorbed),
			CEREAL_NVP(hit.nbMCHit),
			CEREAL_NVP(hit.nbHitEquiv),
			CEREAL_NVP(hit.nbAbsEquiv),
			CEREAL_NVP(hit.sum_1_per_ort_velocity),
			CEREAL_NVP(hit.sum_1_per_velocity),
			CEREAL_NVP(hit.sum_v_ort)
			);
	}
};
#endif

#if defined(SYNRAD)
class FacetHitBuffer {
public:
    FacetHitBuffer();
    void ResetBuffer();

    struct {
        // Counts
        size_t nbMCHit;               // Number of hits
        size_t nbDesorbed;          // Number of desorbed molec
        double nbHitEquiv;			//Equivalent number of hits, used for low-flux impingement rate and density calculation
        double nbAbsEquiv;          // Equivalent number of absorbed molecules
        double fluxAbs;         // Total desorbed Flux
        double powerAbs;        // Total desorbed power
    } hit;

	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(
			CEREAL_NVP(hit.fluxAbs),
			CEREAL_NVP(hit.powerAbs),
			CEREAL_NVP(hit.nbMCHit),           // Number of hits
			CEREAL_NVP(hit.nbHitEquiv),			//Equivalent number of hits, used for low-flux impingement rate and density calculation
			CEREAL_NVP(hit.nbAbsEquiv),      // Number of absorbed molec
			CEREAL_NVP(hit.nbDesorbed)
			);
	}

	FacetHitBuffer& operator+=(const FacetHitBuffer& rhs);
};
#endif


class GlobalHitBuffer { //Should be plain old data, memset applied
public:
	FacetHitBuffer globalHits;               // Global counts (as if the whole geometry was one extra facet)
	size_t hitCacheSize;              // Number of valid hits in cache
	size_t lastHitIndex;					//Index of last recorded hit in gHits (turns over when reaches HITCACHESIZE)
	HIT hitCache[HITCACHESIZE];       // Hit history
	LEAK leakCache[LEAKCACHESIZE];      // Leak history
	size_t  lastLeakIndex;		  //Index of last recorded leak in gHits (turns over when reaches LEAKCACHESIZE)
	size_t  leakCacheSize;        //Number of valid leaks in the cache
	size_t  nbLeakTotal;         // Total leaks
	

#if defined(MOLFLOW)
	TEXTURE_MIN_MAX texture_limits[3]; //Min-max on texture
	double distTraveled_total;
	double distTraveledTotal_fullHitsOnly;
#endif

#if defined(SYNRAD)
	TextureCell hitMin, hitMax;
	double distTraveledTotal;
#endif

	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(
			CEREAL_NVP(globalHits),               // Global counts (as if the whole geometry was one extra facet)
			CEREAL_NVP(hitCacheSize),              // Number of valid hits in cache
			CEREAL_NVP(lastHitIndex),					//Index of last recorded hit in gHits (turns over when reaches HITCACHESIZE)
			CEREAL_NVP(hitCache),       // Hit history

			CEREAL_NVP(lastLeakIndex),		  //Index of last recorded leak in gHits (turns over when reaches LEAKCACHESIZE)
			CEREAL_NVP(leakCacheSize),        //Number of valid leaks in the cache
			CEREAL_NVP(nbLeakTotal),         // Total leaks
			CEREAL_NVP(leakCache)      // Leak history

#if defined(MOLFLOW)
			,CEREAL_NVP(texture_limits), //Min-max on texture
			CEREAL_NVP(distTraveled_total),
			CEREAL_NVP(distTraveledTotal_fullHitsOnly)
#endif

#if defined(SYNRAD)
			,CEREAL_NVP(hitMin),
				CEREAL_NVP(hitMax),
				CEREAL_NVP(distTraveledTotal)
#endif
		);
	}
};

class ParticleLoggerItem {
public:
	Vector2d facetHitPosition;
	double hitTheta, hitPhi;
	double oriRatio;
#if defined(MOLFLOW)
	double time, particleDecayMoment, velocity;
#endif
#if defined(SYNRAD)
	double energy, dF, dP;
#endif
	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(
			CEREAL_NVP(facetHitPosition),
			CEREAL_NVP(hitTheta), CEREAL_NVP(hitPhi),
			CEREAL_NVP(oriRatio)
#if defined(MOLFLOW)
			,CEREAL_NVP(time), CEREAL_NVP(particleDecayMoment), CEREAL_NVP(velocity)
#endif
#if defined(SYNRAD)
			,CEREAL_NVP(energy), CEREAL_NVP(dF), CEREAL_NVP(dP)
#endif
		);
	}
};