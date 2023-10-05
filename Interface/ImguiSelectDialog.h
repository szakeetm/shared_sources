#pragma once
#include <string>
#include <vector>

class ImSelectDialog {
public:
	ImSelectDialog();
	void Draw();
	void Show();
	std::vector<size_t> facetIds;
protected:
	void (*select)();
	void (*addSelect)();
	void (*rmvSelect)();
	bool drawn;
	std::string numText;
	void Preprocess();
};