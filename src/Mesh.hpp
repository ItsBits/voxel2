#pragma once

#include <vector>
#include "cfg.hpp"

struct Mesh {
    glm::tvec3<cfg::Coord> position;
    std::vector<cfg::Vertex> mesh;
    Mesh() = default;
    Mesh(const Mesh &) = delete;
    Mesh(Mesh &&) = default;
    Mesh & operator = (const Mesh &) = delete;
    Mesh & operator = (Mesh &&) = default;
};