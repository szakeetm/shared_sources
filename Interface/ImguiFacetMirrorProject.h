#include <ImguiWindowBase.h>

class ImFacetMirrorProject : public ImWindow {
public:
	void Draw();
private:
	enum PlaneDefinition : int {
		xy, yz, xz, planeOfFacet, byVerts, byEqation, none
	};
	PlaneDefinition mode = none;
	int facetId;
	std::string facetIdInput, xFactorInput, yFactorInput, zFactorInput, offsetInput;
	double xFactor, yFactor, zFactor, offset;
	bool enableUndo = false;
};