#include <ImguiWindowBase.h>

class ImFacetMirrorProject : public ImWindow {
public:
	void Draw();
	void Clear();
private:
	enum Action : int {
		Mirror, Project
	};
	void DoMirrorProject(Action action, bool copy);
	void UndoProjection();
	std::vector<UndoPoint> undoPoints;
	enum PlaneDefinition : int {
		xy, yz, xz, planeOfFacet, byVerts, byEqation, none
	};
	PlaneDefinition mode = none;
	size_t facetId = 0;
	std::string facetIdInput="", aInput = "", bInput = "", cInput = "", dInput = "";
	double a=0, b=0, c=0, d=0;
	bool enableUndo = false;
	friend class ImTest;
};