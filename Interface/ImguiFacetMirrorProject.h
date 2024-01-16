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
	int facetId;
	std::string facetIdInput, aInput, bInput, cInput, dInput;
	double a, b, c, d;
	bool enableUndo = false;
};