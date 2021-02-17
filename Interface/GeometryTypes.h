//
// Created by Pascal Baehr on 17.02.21.
//

#ifndef MOLFLOW_PROJ_GEOMETRYTYPES_H
#define MOLFLOW_PROJ_GEOMETRYTYPES_H

#include <string>
#include <vector>

typedef struct {
    std::string    name;       // Selection name
    std::vector<size_t> selection; // List of facets
} SelectionGroup;

#endif //MOLFLOW_PROJ_GEOMETRYTYPES_H
