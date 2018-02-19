#pragma once

#include <array>
#include <thread>
#include <atomic>

#include "cfg.hpp"
#include "VoxelIterator.hpp"
#include "LockedQueue.hpp"
#include "Mesh.hpp"
#include "ThreadBarrier.hpp"

class VoxelContainer {
public:
    VoxelContainer();
    ~VoxelContainer();

    void moveCenterChunk(const glm::tvec3<cfg::Coord> & new_center_chunk);
    LockedQueue<Mesh> & getQueue() { return m_mesh_queue; }

private:
    LockedQueue<Mesh> m_mesh_queue;
    std::array<cfg::Block, cfg::CHUNK_VOLUME * cfg::CHUNK_ARRAY_VOLUME> m_blocks;
    std::array<glm::tvec3<cfg::Coord>, cfg::CHUNK_ARRAY_VOLUME> m_chunk_positions;
    VoxelIterator m_voxel_indices;
    std::array<std::thread, cfg::WORKER_THREAD_COUNT> m_workers;
    std::atomic_bool m_workers_running;
    std::atomic_size_t m_iterator;
    glm::tvec3<cfg::Coord> m_loader_center_chunk;
    std::mutex m_actual_center_lock;
    glm::tvec3<cfg::Coord> m_actual_center_chunk;
    using MeshReadinesType = uint8_t;
    std::array<std::atomic<MeshReadinesType>, cfg::MESH_ARRAY_VOLUME> m_mesh_readines;
    std::array<glm::tvec3<cfg::Coord>, cfg::MESH_ARRAY_VOLUME> m_mesh_positions;
    //std::array<std::vector<cfg::Vertex>, cfg::MESH_ARRAY_VOLUME> m_meshes;
    ThreadBarrier m_barrier;
    std::atomic_bool m_center_dirty;

    static_assert(cfg::MESH_CHUNK_VOLUME == 8);
    static constexpr MeshReadinesType ALL_CHUNKS_READY{ 0b11111111 };

    void worker();
    void clearMeshReadines();
    std::size_t markMeshes(const glm::tvec3<cfg::Coord> & chunk_position, std::array<glm::tvec3<cfg::Coord>, cfg::CHUNK_MESH_VOLUME> & meshes_to_load);
    void generateChunk(cfg::Block * chunk, const glm::tvec3<cfg::Coord> & chunk_position);
    void generateMesh(const glm::tvec3<cfg::Coord> & mesh_position, std::vector<cfg::Vertex> & mesh);

};
