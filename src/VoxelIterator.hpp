#pragma once

#include "cfg.hpp"
#include <vector>

class VoxelIterator {
public:
    VoxelIterator();
    const glm::tvec3<cfg::Coord> & operator [] (size_t i) const;
    std::size_t size() const { return m_indices.size(); }

private:
    std::vector<glm::tvec3<cfg::Coord>> m_indices;

};
