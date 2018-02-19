#include "VoxelIterator.hpp"

#include <cassert>
#include <algorithm>
#include <unordered_set>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include "Math.hpp"

VoxelIterator::VoxelIterator() {
    glm::tvec3<cfg::Coord> i;
    std::vector<glm::tvec3<cfg::Coord>> mesh_indices;
    mesh_indices.reserve(cfg::MESH_LOADING_VOLUME);
    for (i.z = -cfg::MESH_LOADING_RADIUS.z; i.z <= cfg::MESH_LOADING_RADIUS.z; ++i.z)
        for (i.y = -cfg::MESH_LOADING_RADIUS.y; i.y <= cfg::MESH_LOADING_RADIUS.y; ++i.y)
            for (i.x = -cfg::MESH_LOADING_RADIUS.x; i.x <= cfg::MESH_LOADING_RADIUS.x; ++i.x)
                mesh_indices.push_back(i);

    std::sort(
        std::begin(mesh_indices), std::end(mesh_indices),
        [](const glm::tvec3<cfg::Coord> & a, const glm::tvec3<cfg::Coord> & b) {
            // sort by distance from (0,0,0)
            return Math::dot(a, a) < Math::dot(b, b);
        }
    );

    std::unordered_set<glm::tvec3<cfg::Coord>> chunk_indices_map;

    for (const auto & mi : mesh_indices)
        for (i.z = mi.z + cfg::MESH_CHUNK_START.z; i.z < mi.z + cfg::MESH_CHUNK_END.z; ++i.z)
            for (i.y = mi.y + cfg::MESH_CHUNK_START.y; i.y < mi.y + cfg::MESH_CHUNK_END.y; ++i.y)
                for (i.x = mi.x + cfg::MESH_CHUNK_START.x; i.x < mi.x + cfg::MESH_CHUNK_END.x; ++i.x)
                    if (chunk_indices_map.insert(i).second)
                        m_indices.push_back(i);
}

const glm::tvec3<cfg::Coord> & VoxelIterator::operator [] (size_t i) const {
    assert(i < m_indices.size());
    return m_indices[i];
}
