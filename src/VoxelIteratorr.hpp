#pragma once

class VoxelIteratorr {
public:
    static constexpr int8_t LOAD_MESH{ 0 };
    static constexpr int8_t LOAD_CHUNK{ 1 };

    VoxelIteratorr() {
        static constexpr int32_t LRX{ cfg::MESH_LOADING_RADIUS_X };
        static constexpr int32_t LRY{ cfg::MESH_LOADING_RADIUS_Y };
        static constexpr int32_t LRZ{ cfg::MESH_LOADING_RADIUS_Z };
        static_assert(
            cfg::MESH_SIZE_X % cfg::CHUNK_SIZE_X == 0 &&
            cfg::MESH_SIZE_Y % cfg::CHUNK_SIZE_Y == 0 &&
            cfg::MESH_SIZE_Z % cfg::CHUNK_SIZE_Z == 0
        );
        static constexpr int32_t BLOCK_START_X{ cfg::MESH_OFFSET_X - 1 };
        static constexpr int32_t BLOCK_START_Y{ cfg::MESH_OFFSET_Y - 1 };
        static constexpr int32_t BLOCK_START_Z{ cfg::MESH_OFFSET_Z - 1 };
        static constexpr int32_t BLOCK_END_X{ cfg::MESH_OFFSET_X + cfg::MESH_SIZE_X + 1 };
        static constexpr int32_t BLOCK_END_Y{ cfg::MESH_OFFSET_Y + cfg::MESH_SIZE_Y + 1 };
        static constexpr int32_t BLOCK_END_Z{ cfg::MESH_OFFSET_Z + cfg::MESH_SIZE_Z + 1 };
        // (x + (x < 0)) / y - (x < 0)
        static constexpr int32_t CHUNK_START_X{ (BLOCK_START_X + (BLOCK_START_X < 0)) / cfg::CHUNK_SIZE_X - (BLOCK_START_X < 0) };
        static constexpr int32_t CHUNK_START_Y{ (BLOCK_START_Y + (BLOCK_START_Y < 0)) / cfg::CHUNK_SIZE_Y - (BLOCK_START_Y < 0) };
        static constexpr int32_t CHUNK_START_Z{ (BLOCK_START_Z + (BLOCK_START_Z < 0)) / cfg::CHUNK_SIZE_Z - (BLOCK_START_Z < 0) };
        static constexpr int32_t CHUNK_END_X{ (BLOCK_END_X + (BLOCK_END_X < 0)) / cfg::CHUNK_SIZE_X - (BLOCK_END_X < 0) };
        static constexpr int32_t CHUNK_END_Y{ (BLOCK_END_Y + (BLOCK_END_Y < 0)) / cfg::CHUNK_SIZE_Y - (BLOCK_END_Y < 0) };
        static constexpr int32_t CHUNK_END_Z{ (BLOCK_END_Z + (BLOCK_END_Z < 0)) / cfg::CHUNK_SIZE_Z - (BLOCK_END_Z < 0) };
        struct Hash { size_t operator () (const Vec3<int8_t> & k) const {
            return 73856093 * k.x + 19349663 * k.y + 83492791 * k.z;
        }};
        struct Equal { bool operator () (const Vec3<int8_t> & l, const Vec3<int8_t> & r) const {
            return l.x == r.x && l.y == r.y && l.z == r.z;
        }};

        std::vector<Vec3<int8_t>> mesh_indices;
        Vec3<int8_t> i;

        // meshes to load
        for (i.z = -LRZ; i.z <= LRZ; ++i.z)
            for (i.y = -LRY; i.y <= LRY; ++i.y)
                for (i.x = -LRX; i.x <= LRX; ++i.x)
                    mesh_indices.push_back(i);
        // sort meshes by distance from center
        std::sort(mesh_indices.begin(), mesh_indices.end(),
            [](const Vec3<int8_t> & a, const Vec3<int8_t> & b) {
//                static_assert(0, "TODO: cast to larger type before multiplying");
                return a.x*a.x+a.y*a.y+a.z*a.z < b.x*b.x+b.y*b.y+b.z*b.z;
        });
        // chunks to load
        std::unordered_set<Vec3<int8_t>, Hash, Equal> chunk_indices_map;
        for (const auto & j : mesh_indices) {
            for (i.x = j.x + CHUNK_START_X; i.x <= j.x + CHUNK_END_X; ++i.x)
                for (i.y = j.y + CHUNK_START_Y; i.y <= j.y + CHUNK_END_Y; ++i.y)
                    for (i.z = j.z + CHUNK_START_Z; i.z <= j.z + CHUNK_END_Z; ++i.z) {
                        const bool is_new = chunk_indices_map.insert(i).second;
                        if (is_new) {
                            m_indices.push_back({ i.x, i.y, i.z, LOAD_CHUNK });
                        }
                    }
            m_indices.push_back({ j.x, j.y, j.z, LOAD_MESH });
        }
    }

    const std::vector<Vec4<int8_t>> & indices() { return m_indices; }

    Vec3<int8_t> getRadius() const {
        return {
            cfg::MESH_LOADING_RADIUS_X,
            cfg::MESH_LOADING_RADIUS_Y,
            cfg::MESH_LOADING_RADIUS_Z
        };
    }

private:
    // LOAD_MESH nodes can be moved back by any amount
    std::vector<Vec4<int8_t>> m_indices;
    
};