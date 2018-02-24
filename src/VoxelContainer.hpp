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
    // public functions are not thread safe
    VoxelContainer();
    ~VoxelContainer();
    LockedQueue<Mesh, cfg::MESH_QUEUE_SIZE_LIMIT> & getQueue() { return m_mesh_queue; }
    // returns read only chunk data, returns nullptr if chunk not available at the moment
    // pointer is invalidated after next call to moveCenterChunk()
    const cfg::Block * getChunk(const glm::tvec3<cfg::Coord> & chunk_position);
    cfg::Block * getWritableChunk(const glm::tvec3<cfg::Coord> & chunk_position);
    // these two functions may reset the iterator
    void invalidateMeshWithBlockRange(Math::AABB3<cfg::Coord> range);
    void moveCenterChunk(const glm::tvec3<cfg::Coord> & new_center_chunk);

private:
    LockedQueue<Mesh, cfg::MESH_QUEUE_SIZE_LIMIT> m_mesh_queue;
    std::array<cfg::Block, cfg::CHUNK_VOLUME * cfg::CHUNK_ARRAY_VOLUME> m_blocks;
    // this atomic vec array makes me cry
    // without atomic -> undefined behaviour, but should work correctly on any common platforms anyway
    std::array<std::atomic<Math::DumbVec3>, cfg::CHUNK_ARRAY_VOLUME> m_chunk_positions;
    VoxelIterator m_voxel_indices;
    std::array<std::thread, cfg::WORKER_THREAD_COUNT> m_workers;
    std::atomic_bool m_workers_running;
    std::atomic_size_t m_iterator;
    glm::tvec3<cfg::Coord> m_loader_center_chunk;
    glm::tvec3<cfg::Coord> m_actual_center_chunk;
    Math::AABB3<cfg::Coord> m_center_chunk_overlap;
    std::mutex m_center_lock;
    using MeshReadinesType = uint8_t;
    std::array<std::atomic<MeshReadinesType>, cfg::MESH_ARRAY_VOLUME> m_mesh_readines;
    std::array<std::atomic<Math::DumbVec3>, cfg::MESH_ARRAY_VOLUME> m_mesh_positions;
    ThreadBarrier m_barrier;
    // used for more than what the name suggests
    std::atomic_bool m_center_dirty;
    std::atomic_size_t m_workers_finished;
    std::condition_variable m_condition;

    static_assert(cfg::MESH_CHUNK_VOLUME == 8);
    static constexpr MeshReadinesType ALL_CHUNKS_READY{ 0b11111111 };

    void worker();
    void clearMeshReadines();
    std::size_t markMeshes(const glm::tvec3<cfg::Coord> & chunk_position, std::array<glm::tvec3<cfg::Coord>, cfg::CHUNK_MESH_VOLUME> & meshes_to_load);
    bool checkMeshes(const glm::tvec3<cfg::Coord> & chunk_position);
    void generateChunk(cfg::Block * chunk, const glm::tvec3<cfg::Coord> & chunk_position);
    void generateMesh(const glm::tvec3<cfg::Coord> & mesh_position, std::vector<cfg::Vertex> & mesh);
    cfg::Block * getChunkNonConst(const glm::tvec3<cfg::Coord> & chunk_position);

};
