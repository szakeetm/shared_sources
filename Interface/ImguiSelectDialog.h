#pragma once
#include <string>
#include <vector>
#include "ImguiWindowBase.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow* mApp;
#endif

#if defined(SYNRAD)
extern SynRad* mApp;
#include "../src/SynRad.h"
#endif

class ImSelectDialog : public ImWindow{
public:
	enum selMode : int {
		none,
		select,
		addSelect,
		rmvSelect
	};
	void Func(const int mode);
	void Draw();
	std::vector<size_t> facetIds;
protected:
	std::string numText;
	void Preprocess();
};