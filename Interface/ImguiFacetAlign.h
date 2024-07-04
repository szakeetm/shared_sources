#include "ImguiWindowBase.h"

class ImFacetAlign : public ImWindow {
public:
	void Draw();
	void MemorizeSelection();
	void AlignButtonPress(bool copy);
	void UndoButtonPress();
protected:
	bool invertNormal = true, swapOnSource = false, swapOnDestination = false;
	std::vector<size_t> memorizedSelection;
	std::vector<std::vector<Vector3d>> oriPositions;
};