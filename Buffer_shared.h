#pragma once
#include "Vector.h"
#include "GLApp/GLTypes.h"

#ifdef MOLFLOW
#include "MolflowTypes.h" //Texture Min Max of GlobalHitBuffer
#endif

#define PROFILE_SIZE  (size_t)100 // Size of profile
#define LEAKCACHESIZE     (size_t)2048  // Leak history max length
#define HITCACHESIZE      (size_t)2048  // Max. displayed number of lines and hits.

class FacetProperties { //Formerly SHFACET
public:
	//For sync between interface and subprocess
	double sticking;       // Sticking (0=>reflection  , 1=>absorption)   - can be overridden by time-dependent parameter
	double opacity;        // opacity  (0=>transparent , 1=>opaque)
	double area;           // Facet area (m^2)

	int    profileType;    // Profile type
	size_t    superIdx;       // Super structure index (Indexed from 0)
	size_t    superDest;      // Super structure destination index (Indexed from 1, 0=>current)
	int	 teleportDest;   // Teleport destination facet id (for periodic boundary condition) (Indexed from 1, 0=>none, -1=>teleport to where it came from)

	bool   countAbs;       // Count absoprtion (MC texture)
	bool   countRefl;      // Count reflection (MC texture)
	bool   countTrans;     // Count transparent (MC texture)
	bool   countDirection;

	// Flags
	bool   is2sided;     // 2 sided
	bool   isProfile;    // Profile facet
	bool   isTextured;   // texture
	bool   isVolatile;   // Volatile facet (absorbtion facet which does not affect particule trajectory)

						 // Geometry
	size_t    nbIndex;   // Number of index/vertex
	double sign;      // Facet vertex rotation (see Facet::DetectOrientation())

					  // Plane basis (O,U,V) (See Geometry::InitializeGeometry() for info)
	Vector3d   O;  // Origin
	Vector3d   U;  // U vector
	Vector3d   V;  // V vector
	Vector3d   nU; // Normalized U
	Vector3d   nV; // Normalized V

				   // Normal vector
	Vector3d    N;    // normalized
	Vector3d    Nuv;  // normal to (u,v) not normlized

					  // Axis Aligned Bounding Box (AABB)
	AABB       bb;
	Vector3d   center;

	// Hit/Abs/Des/Density recording on 2D texture map
	size_t    texWidth;    // Rounded texture resolution (U)
	size_t    texHeight;   // Rounded texture resolution (V)
	double texWidthD;   // Actual texture resolution (U)
	double texHeightD;  // Actual texture resolution (V)

	size_t   hitOffset;      // Hit address offset for this facet

#ifdef MOLFLOW
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
	// FacetHitBuffer counter; - removed as now it's time-dependent and part of the hits buffer

	// Moving facets
	bool isMoving;

	//Outgassing map
	bool   useOutgassingFile;   //has desorption file for cell elements
	double outgassingFileRatio; //desorption file's sample/unit ratio
	size_t   outgassingMapWidth; //rounded up outgassing file map width
	size_t   outgassingMapHeight; //rounded up outgassing file map height

	double totalOutgassing; //total outgassing for the given facet

	AnglemapParams anglemapParams;//Incident angle map
#endif

#ifdef SYNRAD
	int    doScattering;   // Do rough surface scattering
	double rmsRoughness;   // RMS height roughness, in meters
	double autoCorrLength; // Autocorrelation length, in meters
	int    reflectType;    // Reflection type. 0=Diffuse, 1=Mirror, 10,11,12... : Material 0, Material 1, Material 2...., 9:invalid 
	bool   hasSpectrum;    // Calculate energy spectrum (histogram)
#endif

};

class GeomProperties {  //Formerly SHGEOM
public:
	size_t     nbFacet;   // Number of facets (total)
	size_t     nbVertex;  // Number of 3D vertices
	size_t     nbSuper;   // Number of superstructures
	char       name[64];  // (Short file name)

#ifdef MOLFLOW
	size_t nbMoments; //To pass in advance for memory reservation
	double latestMoment;
	double totalDesorbedMolecules; //Number of molecules desorbed between t=0 and latest_moment
	double finalOutgassingRate; //Number of outgassing molecules / second at latest_moment (constant flow)
	double gasMass;
	bool enableDecay;
	double halfLife;
	double timeWindowSize;
	bool useMaxwellDistribution; //true: Maxwell-Boltzmann distribution, false: All molecules have the same (V_avg) speed
	bool calcConstantFlow;

	int motionType;
	Vector3d motionVector1; //base point for rotation
	Vector3d motionVector2; //rotation vector or velocity vector
#endif

#ifdef SYNRAD
	size_t        nbRegion;  //number of magnetic regions
	size_t        nbTrajPoints; //total number of trajectory points (calculated at CopyGeometryBuffer)
	bool       newReflectionModel;
#endif
};

typedef struct {

	Vector3d pos;
	int    type;
#ifdef SYNRAD
	double dF;
	double dP;
#endif
} HIT;

// Velocity field
typedef struct {
	Vector3d dir;
	size_t count;
} VHIT;

typedef struct {
	Vector3d pos;
	Vector3d dir;
} LEAK;

#ifdef MOLFLOW
typedef union {

	struct {
		// Counts
		llong nbDesorbed;          // Number of desorbed molec
		llong nbHit;               // Number of hits
		llong nbAbsorbed;          // Number of absorbed molec
		double sum_1_per_ort_velocity;    // sum of reciprocials of orthogonal velocity components, used to determine the density, regardless of facet orientation
		double sum_v_ort;          // sum of orthogonal speeds of incident velocities, used to determine the pressure
	} hit;

	struct {
		// density
		double desorbed;
		double value;
		double absorbed;
	} density;
} FacetHitBuffer;
#endif

#ifdef SYNRAD
	typedef struct {
		// Counts
		double fluxAbs, powerAbs;
		llong nbHit;           // Number of hits
		llong nbAbsorbed;      // Number of absorbed molec
		llong nbDesorbed;
	} FacetHitBuffer;
#endif


class GlobalHitBuffer {
public:
	FacetHitBuffer total;               // Global counts (as if the whole geometry was one extra facet)
	size_t hitCacheSize;              // Number of valid hits in cache
	size_t lastHitIndex;					//Index of last recorded hit in gHits (turns over when reaches HITCACHESIZE)
	HIT    hitCache[HITCACHESIZE];       // Hit history

	size_t  lastLeakIndex;		  //Index of last recorded leak in gHits (turns over when reaches LEAKCACHESIZE)
	size_t  leakCacheSize;        //Number of valid leaks in the cache
	size_t  nbLeakTotal;         // Total leaks
	LEAK   leakCache[LEAKCACHESIZE];      // Leak history

#ifdef MOLFLOW
	int    mode;                // Simu mode (MC_MODE or AC_MODE)
	TEXTURE_MIN_MAX texture_limits[3]; //Min-max on texture
	double distTraveledTotal_total;
	double distTraveledTotal_fullHitsOnly;
#endif

#ifdef SYNRAD
	llong  minHit_MC, maxHit_MC;
	double   minHit_flux;              // Minimum on texture
	double   maxHit_flux;              // Maximum on texture
	double   minHit_power;              // Minimum on texture
	double   maxHit_power;              // Maximum on texture
	double distTraveledTotal;
#endif
};



// Master control shared memory block  (name: MFLWCTRL[masterPID])
// 

#define PROCESS_STARTING 0   // Loading state
#define PROCESS_RUN      1   // Running state
#define PROCESS_READY    2   // Waiting state
#define PROCESS_KILLED   3   // Process killed
#define PROCESS_ERROR    4   // Process in error
#define PROCESS_DONE     5   // Simulation ended
#define PROCESS_RUNAC    6   // Computing AC matrix

#define COMMAND_NONE     10  // No change
#define COMMAND_LOAD     11  // Load geometry
#define COMMAND_START    12  // Start simu
#define COMMAND_PAUSE    13  // Pause simu
#define COMMAND_RESET    14  // Reset simu
#define COMMAND_EXIT     15  // Exit
#define COMMAND_CLOSE    16  // Release handles
#define COMMAND_UPDATEPARAMS 17 //Update simulation mode (low flux, fluxwise/powerwise, displayed regions)
#define COMMAND_LOADAC   18  // Load mesh and compute AC matrix
#define COMMAND_STEPAC   19  // Perform single iteration step (AC)

static const char *prStates[] = {

	"Not started",
	"Running",
	"Waiting",
	"Killed",
	"Error",
	"Done",
	"Computing AC matrix", //Molflow only
	"",
	"",
	"",
	"No command",
	"Loading",
	"Starting",
	"Stopping",
	"Resetting",
	"Exiting",
	"Closing",
	"Update params",  //Synrad only
	"Load AC matrix", //Molflow only
	"AC iteration step" //Molflow only
};

#define MAX_PROCESS (size_t)32    // Maximum number of process

typedef struct {
	// Process control
	int		states[MAX_PROCESS];        // Process states/commands
	size_t    cmdParam[MAX_PROCESS];      // Command param 1
	size_t		cmdParam2[MAX_PROCESS];     // Command param 2
	char		statusStr[MAX_PROCESS][128]; // Status message
} SHCONTROL;