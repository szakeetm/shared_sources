#include "ImguiWindowBase.h"

class ImFacetRotate : public ImWindow {
public:
	void Draw();
protected:
	enum Mode : int {
		axisX, axisY, axisZ, facetU, facetV, facetN, verticies, equation, none
	};
	Mode mode = none;
	int facetId;
	std::string facetIdInput;
	double a=0, b=0, c=0, u=0, v=0, w=0, deg=0, rad=0;
	std::string aIn="0", bIn = "0", cIn = "0", uIn = "0", vIn = "0", wIn = "0", degIn = "0", radIn = "0";
	void RotateFacetButtonPress(bool copy);
	void DoRotate(bool copy);
};