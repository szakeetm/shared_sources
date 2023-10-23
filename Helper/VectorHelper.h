#pragma once
#include <vector>
namespace Util {
	template <typename T>
	bool inVec(T val, std::vector<T> vec);
}
template <typename T>
inline bool Util::inVec(T val, std::vector<T> vec)
{
	return std::find(vec.begin(), vec.end(), val) != vec.end();
}