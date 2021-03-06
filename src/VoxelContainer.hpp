#pragma once

#include <array>
#include <thread>
#include <atomic>
#include "cfg.hpp"
#include "VoxelIterator.hpp"
#include "LockedQueue.hpp"
#include "Mesh.hpp"
#include "ThreadBarrier.hpp"
#include "RegionContainer.hpp"

class VoxelContainer {
public:
    // public functions are not thread safe
    VoxelContainer();
    ~VoxelContainer();
    LockedQueue<Mesh, cfg::MESH_QUEUE_SIZE_LIMIT> & getQueue() { return m_mesh_queue; }
    // returns read only chunk data, returns nullptr if chunk not available at the moment
    // pointer is invalidated after next call to moveCenterChunk()
    const cfg::Block * getChunk(const glm::tvec3<cfg::Coord> & chunk_position);
    // TODO: set chunks dirty
    cfg::Block * getWritableChunk(const glm::tvec3<cfg::Coord> & chunk_position);
    // these two functions may reset the iterator
    // IMPORTANT: invalidating meshes outside of chunks received from calls to getWritableChunk() since last call to moveCenterChunk() is undefined behaviour
    void invalidateMeshWithBlockRange(Math::AABB3<cfg::Coord> range);
    void moveCenterChunk(const glm::tvec3<cfg::Coord> & new_center_chunk);

private:
    LockedQueue<Mesh, cfg::MESH_QUEUE_SIZE_LIMIT> m_mesh_queue;
    std::array<cfg::Block, cfg::CHUNK_VOLUME * cfg::CHUNK_ARRAY_VOLUME> m_blocks;
    // this atomic vec array makes me cry
    // without atomic -> undefined behaviour, but should work correctly on any common platforms anyway
    std::array<std::atomic<Math::DumbVec3>, cfg::CHUNK_ARRAY_VOLUME> m_chunk_positions;
    std::array<bool, cfg::CHUNK_ARRAY_VOLUME> m_chunk_dirty;
    VoxelIterator m_voxel_indices;
    std::array<std::thread, cfg::WORKER_THREAD_COUNT> m_workers;
    struct WorkerData {
        // non owning pointers (reference counting is managed by RegionContainer)
        std::array<Region *, cfg::WORKER_REGION_CACHE_VOLUME> regions;
        std::array<glm::tvec3<cfg::Coord>, cfg::WORKER_REGION_CACHE_VOLUME> positions;
    };
    std::array<WorkerData, cfg::WORKER_THREAD_COUNT> m_workers_data;
    std::atomic_bool m_workers_running;
    std::atomic_size_t m_iterator;
    glm::tvec3<cfg::Coord> m_loader_center_chunk;
    glm::tvec3<cfg::Coord> m_actual_center_chunk;
    Math::AABB3<cfg::Coord> m_center_chunk_overlap;
    std::mutex m_center_lock;
    using MeshReadinesType = uint8_t;
    std::array<std::atomic<MeshReadinesType>, cfg::MESH_ARRAY_VOLUME> m_mesh_readines;
    std::array<std::atomic<Math::DumbVec3>, cfg::MESH_ARRAY_VOLUME> m_mesh_positions;
    std::array<bool, cfg::MESH_ARRAY_VOLUME> m_mesh_empties;
    ThreadBarrier m_barrier;
    // used for more than what the name suggests
    std::atomic_bool m_center_dirty;
    std::atomic_size_t m_workers_finished;
    std::condition_variable m_condition;
    RegionContainer m_region_container;

    static_assert(cfg::MESH_CHUNK_VOLUME == 8);
    static constexpr MeshReadinesType ALL_CHUNKS_READY{ 0b11111111 };

    void worker(size_t thread_id);
    void clearMeshReadines();
    std::size_t markMeshes(const glm::tvec3<cfg::Coord> & chunk_position, std::array<glm::tvec3<cfg::Coord>, cfg::CHUNK_MESH_VOLUME> & meshes_to_load);
    bool checkMeshes(const glm::tvec3<cfg::Coord> & chunk_position);
    void generateChunk(cfg::Block * chunk, const glm::tvec3<cfg::Coord> & chunk_position);
    void generateMesh(const glm::tvec3<cfg::Coord> & mesh_position, std::vector<cfg::Vertex> & mesh);
    cfg::Block * getChunkNonConst(const glm::tvec3<cfg::Coord> & chunk_position);
    void saveChunk(const cfg::Block * chunk, const glm::tvec3<cfg::Coord> & chunk_position, Region * region);
    // return true if loading successful (aka. chunk found in storage)
    bool tryLoadChunk(cfg::Block * chunk, const glm::tvec3<cfg::Coord> & chunk_position, Region * region);
    Region * fetchRegionUseWorkerCache(const glm::tvec3<cfg::Coord> & region_position, WorkerData & worker_data);
};
