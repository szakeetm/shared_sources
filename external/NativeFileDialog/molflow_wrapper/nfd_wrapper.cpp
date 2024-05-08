#include "nfd_wrapper.h"
#include <nfd.hpp>

std::string NFD_OpenFile_Cpp(const std::string& fileFilters,const std::string& path) {
    // initialize NFD
    NFD::Guard nfdGuard;

    // auto-freeing memory
    NFD::UniquePath outPath;

    // prepare filters for the dialog
    nfdfilteritem_t filterItem[1] = { { "Supported types", fileFilters.c_str() } };

    // show the dialog
    const char* defaultPath = path.c_str(); if (path.empty()) defaultPath = nullptr;
    size_t filterCount = fileFilters.empty() ? 0 : 1;
    nfdresult_t result = NFD::OpenDialog(outPath, filterItem, filterCount, defaultPath);
    std::string resultStr;

	if (result == NFD_OKAY) {
		resultStr = outPath.get();
	}

	return resultStr;
}

std::string NFD_SaveFile_Cpp(const std::string& fileFilters,const std::string& path) {
    // initialize NFD
    NFD::Guard nfdGuard;

    // auto-freeing memory
    NFD::UniquePath outPath;

	//const char* filters = NULL; if (!fileFilters.empty()) filters = fileFilters.c_str();
    nfdfilteritem_t filterItem[1] = { { "Supported types", fileFilters.c_str() } };
    const char* defaultPath = path.c_str(); if (path.empty()) defaultPath = nullptr;



    nfdresult_t result = NFD::SaveDialog(outPath, filterItem, 1, defaultPath);
    std::string resultStr;
    if (result == NFD_OKAY) {
		resultStr = outPath.get();
	}
	return resultStr;
}

std::vector<std::string> NFD_OpenMultiple_Cpp(const std::string& fileFilters,const std::string& path){
    // initialize NFD
    NFD::Guard nfdGuard;

    // auto-freeing memory
    NFD::UniquePathSet outPaths;

    // prepare filters for the dialog
    nfdfilteritem_t filterItem[1] = { { "Supported types", fileFilters.c_str() } };
    const char* defaultPath = path.c_str(); if (path.empty()) defaultPath = nullptr;

    std::vector<std::string> paths;
    // show the dialog
    nfdresult_t result = NFD::OpenDialogMultiple(outPaths, filterItem, 1, defaultPath);
    if (result == NFD_OKAY) {
        nfdpathsetsize_t numPaths;
        NFD::PathSet::Count(outPaths, numPaths);

        nfdpathsetsize_t i;
        for (i = 0; i < numPaths; ++i) {
            NFD::UniquePathSetPath cpath;
            NFD::PathSet::GetPath(outPaths, i, cpath);
            paths.emplace_back(cpath.get());
        }
    }

	return paths;	
}