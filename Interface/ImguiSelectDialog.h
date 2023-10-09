#pragma once
#include <string>
#include <vector>

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow* mApp;
#endif

#if defined(SYNRAD)
extern SynRad* mApp;
#include "../src/SynRad.h"
#endif

class ImSelectDialog {
public:
	enum selMode : int {
		none,
		select,
		addSelect,
		rmvSelect
	};
	void Init();
	void Draw();
	void Show();
	std::vector<size_t> facetIds;
protected:
	bool drawn = false;
	std::string numText;
	void Preprocess();
	void (*function)(int);
};