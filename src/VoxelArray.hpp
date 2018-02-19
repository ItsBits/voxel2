#pragma once

#include "Array3D.hpp"
#include "cfg.hpp"
#include "SparseMap.hpp"
#include "Algebra.hpp"
#include "QuadEBO.hpp"

#include <glm/glm.hpp>

#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>

#include "../gl3w/gl3w.h"

#include "VoxelIteratorr.hpp"
using VoxelArrayIterator = VoxelIteratorr;

//namespace cfg = configuration;
using Block = uint8_t;

class VoxelArray {
private:
    struct ChunkMeta {
        Vec3<int32_t> position;
        bool dirty;
    };
public:
    VoxelArray();
    ~VoxelArray();

    void update(const glm::ivec3 & center);

       /* Vec3<int32_t> position;
        GLint VAO, VBO;
        GLsizei size;*/
    void draw(GLint offset_uniform) {
        static constexpr glm::ivec3 CHUNK_SIZES{ cfg::CHUNK_SIZE_X, cfg::CHUNK_SIZE_Y, cfg::CHUNK_SIZE_Z };
        //auto s = m_meshes.size();
        for (const auto & m : m_meshes) {
            if (m.size > 0) {
                const glm::ivec3 pos{ m.position.x, m.position.y, m.position.z };
                const auto offset = pos * CHUNK_SIZES;
                glUniform3f(offset_uniform, offset.x, offset.y, offset.z);
                glBindVertexArray(m.VAO);
                glDrawElements(GL_TRIANGLES, m.size, m_quad_ebo.type(), 0);
                glBindVertexArray(0);
            }
        }
    }

private:
    QuadEBO m_quad_ebo;

    Array3D<
        cfg::CHUNK_ARRAY_SIZE_X, cfg::CHUNK_ARRAY_SIZE_Y, cfg::CHUNK_ARRAY_SIZE_Z,
        Block, cfg::CHUNK_VOLUME
    > m_chunks;

    Array3D<
        cfg::CHUNK_ARRAY_SIZE_X, cfg::CHUNK_ARRAY_SIZE_Y, cfg::CHUNK_ARRAY_SIZE_Z,
        ChunkMeta, 1
    > m_positions;

    VoxelArrayIterator m_iterator;
    Vec3<int32_t> m_center{ 0, 0, 0 };

    std::thread m_workers[2];
    std::atomic_bool m_running;

    struct MeshInfo {
        Vec3<int32_t> position;
        GLuint VAO, VBO;
        GLsizei size;
    };

    struct ChunkMesh {
        GLuint VAO, VBO;
        GLsizei element_count;
    };
    struct KeyHash {
    std::size_t operator () (const glm::ivec3 & k) const {
        return k.x * k.y * k.z; // TODO: better hash function
    }};
    struct KeyEqual {
    bool operator()(const glm::ivec3 & lhs, const glm::ivec3 & rhs) const {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
    }};

    std::unordered_map<glm::ivec3, ChunkMesh, KeyHash, KeyEqual> m_meshes_;
    SparseMap<MeshInfo, uint32_t, cfg::MESH_ARRAY_VOLUME> m_meshes;

    template <typename T>
    struct LockedQueue {
    public:
        void push(const T & value) {
            std::lock_guard<std::mutex> l{ lock };
            tasks.push(value);
        }

        bool pop(T & result) {
            std::lock_guard<std::mutex> l{ lock };
            if (tasks.empty()) return false;
            result = tasks.front();
            tasks.pop();
            return true;
        }
    private:
        std::queue<T> tasks;
        std::mutex lock;

    };

    struct MeshFromWorker {
        Vec3<int32_t> position;
        int8_t type;
        std::vector<uint8_t> mesh;
    };

    LockedQueue<Vec4<int8_t>> m_tasks;
    LockedQueue<MeshFromWorker> m_results;

    //==============================================================================
    static inline std::uint8_t vertexAO(const bool side_a, const bool side_b, const bool corner)
    {
    #if 0
        // is this branch free version correct?
        corner = corner || side_a && side_b;
        return (uint8_t)corner + (uint8_t)side_a + (uint8_t)size_b;
    #endif

        if (side_a && side_b) return 3;

        return
            static_cast<std::uint8_t>(side_a) +
            static_cast<std::uint8_t>(side_b) +
            static_cast<std::uint8_t>(corner);
    }

    void worker() {
        while (m_running) {
            Vec4<int8_t> task;
            Vec3<int32_t> center = m_center;
            const bool valid = m_tasks.pop(task);
            if (valid) {
                switch (task.w) {
                case VoxelArrayIterator::LOAD_MESH:
                    loadChunk(center.x + task.x, center.y + task.y, center.z + task.z);
                    break;
                case VoxelArrayIterator::LOAD_CHUNK:
                    loadMesh(center.x + task.x, center.y + task.y, center.z + task.z);
                    break;
                default:
                    break;
                }
            }
        }
    }

    void loadChunk(int32_t x, int32_t y, int32_t z) {
        const int32_t xxx = floor_mod_i(x, cfg::CHUNK_ARRAY_SIZE_X);
        const int32_t yyy = floor_mod_i(y, cfg::CHUNK_ARRAY_SIZE_Y);
        const int32_t zzz = floor_mod_i(z, cfg::CHUNK_ARRAY_SIZE_Z);
        ChunkMeta * chunk_meta = m_positions.get(xxx, yyy, zzz);
        if (all_equal(Vec3<int32_t>{ x, y, z }, chunk_meta->position)) return;

        Block * blocks = m_chunks.get(xxx, yyy, zzz);

        for (size_t i = 0; i < cfg::CHUNK_VOLUME; ++i)
            blocks[i] = std::rand() % 300 == 0;
        
        chunk_meta->position = Vec3<int32_t>{ x, y, z };
        chunk_meta->dirty = false;
    }

    void loadMesh(int32_t x, int32_t y, int32_t z) {

    }

    std::vector<uint8_t> generateChunkMesh(const Vec3<int32_t> & mesh_positionn);

};
