#include "VoxelContainer.hpp"

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "mesher.hpp"
#include "worldgen.hpp"
#include "Print.hpp"


VoxelContainer::VoxelContainer() :
    m_barrier{ cfg::WORKER_THREAD_COUNT }
{
    std::for_each(std::begin(m_workers_data), std::end(m_workers_data), [] (WorkerData & worker_data) {
        std::fill(std::begin(worker_data.regions), std::end(worker_data.regions), nullptr);
        std::fill(std::begin(worker_data.positions), std::end(worker_data.positions), glm::tvec3<cfg::Coord>{ 0, 0, 0 });
    });
    std::for_each(std::begin(m_chunk_positions), std::end(m_chunk_positions), [] (std::atomic<Math::DumbVec3> & vector) {
        vector.store(Math::toDumb3(glm::tvec3<cfg::Coord>{ 0, 0, 0 }, false));
    });
    m_chunk_positions[0].store(Math::toDumb3(glm::tvec3<cfg::Coord>{ 1, 0, 0 }, false));
    std::for_each(std::begin(m_mesh_positions), std::end(m_mesh_positions), [] (std::atomic<Math::DumbVec3> & vector) {
        vector.store(Math::toDumb3(glm::tvec3<cfg::Coord>{ 0, 0, 0 }, false));
    });
    m_mesh_positions[0].store(Math::toDumb3(glm::tvec3<cfg::Coord>{ 1, 0, 0 }, false));
    std::fill(std::begin(m_mesh_empties), std::end(m_mesh_empties), true);
    std::fill(std::begin(m_blocks), std::end(m_blocks), cfg::Block{ 0 });
    std::fill(std::begin(m_chunk_dirty), std::end(m_chunk_dirty), false);
    m_workers_running.store(true);
    m_iterator.store(0);
    m_loader_center_chunk = { 0, 0, 0 };
    m_actual_center_chunk = { 0, 0, 0 };
    m_center_dirty.store(false);
    clearMeshReadines();
    m_workers_finished.store(0);
    for (size_t i = 0; i < cfg::WORKER_THREAD_COUNT; ++i)
        m_workers[i] = std::thread{ &VoxelContainer::worker, this, i };
}

VoxelContainer::~VoxelContainer() {
    m_workers_running.store(false);
    // speed up shutdown of workers
    m_center_dirty.store(true);
    m_condition.notify_one();
    // empty mesh queue after set m_center_dirty 
    // no need to fully empty queue
    // making cfg::WORKER_THREAD_COUNT slots in queue is enough
    Mesh m;
    while (m_mesh_queue.pop(std::move(m)));
    std::for_each(std::begin(m_workers), std::end(m_workers), [this](std::thread & worker){
        worker.join();
    });

    // TODO: !!!!!! release all Regions held by chunks here !!!!!!
    //       it is good practice, but reference count is not enforced
    //       so RAII will take care of correct cleanup when ~RegionContainer() is called

    for (size_t i = 0; i < cfg::CHUNK_ARRAY_VOLUME; ++i) {
        if (m_chunk_dirty[i]) {
            bool dummy;
            const auto chunk_position = Math::toVec3<cfg::Coord>(m_chunk_positions[i], dummy);
            const auto region_position = Math::floor_div(chunk_position, cfg::REGION_SIZE);
            Region & region = m_region_container.get(region_position);
            saveChunk(m_blocks.data() + i * cfg::CHUNK_VOLUME, chunk_position, &region);
            m_region_container.release(region_position);
            // TODO: check if this is true: m_chunk_dirty does not need to be atomic, because there will be no concurrent access
            //       and it is implicitly synchronized between threads by other atomic variables
            m_chunk_dirty[i] = false;
            Print("Saving ", glm::to_string(chunk_position));
        }
    }
}

void VoxelContainer::moveCenterChunk(const glm::tvec3<cfg::Coord> & new_center_chunk) {
    const auto changed = !glm::all(glm::equal(m_actual_center_chunk, new_center_chunk));
    std::lock_guard<std::mutex> lock{ m_center_lock };
    m_actual_center_chunk = new_center_chunk;
    m_center_chunk_overlap = Math::overlap(
        Math::toAABB3(m_actual_center_chunk, cfg::CHUNK_LOADING_RADIUS),
        Math::toAABB3(m_loader_center_chunk, cfg::CHUNK_LOADING_RADIUS)
    );
    if (changed) {
        m_center_dirty.store(true);
        m_condition.notify_one();
    }
}

const cfg::Block * VoxelContainer::getChunk(const glm::tvec3<cfg::Coord> & chunk_position) {
    return getChunkNonConst(chunk_position);
}

cfg::Block * VoxelContainer::getChunkNonConst(const glm::tvec3<cfg::Coord> & chunk_position) {
    const auto chunk_index = Math::position_to_index(chunk_position, cfg::CHUNK_ARRAY_SIZE);
    bool valid_chunk;
    const auto loaded_chunk_position = Math::toVec3<cfg::Coord>(m_chunk_positions[chunk_index].load(), valid_chunk);
    if (!valid_chunk)
        return nullptr;
    if (!glm::all(glm::equal(loaded_chunk_position, chunk_position)))
        return nullptr;
    if (!Math::inside(m_center_chunk_overlap, chunk_position))
        return nullptr;
    return m_blocks.data() + chunk_index * cfg::CHUNK_VOLUME;
}


cfg::Block * VoxelContainer::getWritableChunk(const glm::tvec3<cfg::Coord> & chunk_position) {
    cfg::Block * chunk = getChunkNonConst(chunk_position);
    if (chunk == nullptr) return nullptr;
    // check if meshes are dirty
    if (checkMeshes(chunk_position) == false)
        return nullptr;
    // set dirty if successful acquire (assuming chunk will be modified, but its not a huge deal if its not going to be modified)
    const auto chunk_index = Math::position_to_index(chunk_position, cfg::CHUNK_ARRAY_SIZE);
    m_chunk_dirty[chunk_index] = true;
    return chunk;
}

void VoxelContainer::invalidateMeshWithBlockRange(Math::AABB3<cfg::Coord> range) {
    range.min = Math::floor_div(range.min - Math::add(cfg::MESH_OFFSET, cfg::BLOCK_MESH_EFFECT_RADIUS), cfg::MESH_SIZE);
    range.max = Math::floor_div(range.max - Math::sub(cfg::MESH_OFFSET, cfg::BLOCK_MESH_EFFECT_RADIUS), cfg::MESH_SIZE);
    glm::tvec3<cfg::Coord> i;
    for (i.z = range.min.z; i.z <= range.max.z; ++i.z)
        for (i.y = range.min.y; i.y <= range.max.y; ++i.y)
            for (i.x = range.min.x; i.x <= range.max.x; ++i.x) {
                // assert correct mesh loaded, if not undefined beaviour
                const auto mesh_index = Math::position_to_index(i, cfg::MESH_ARRAY_SIZE);
                m_mesh_positions[mesh_index].store(Math::toDumb3(i, false)); // alternatively atomic_fetch_and(valid_flag)
            }
    m_center_dirty.store(true);
    m_condition.notify_one();
}

void VoxelContainer::worker(size_t thread_id) {
    WorkerData & worker_data = *(m_workers_data.data() + thread_id);
    const auto indices_size = m_voxel_indices.size();
    while (true) {
        // don't rely on variable center_dirty later, because you don't know which threads registered it as true
        const auto center_dirty = m_center_dirty.load(); // well whatever, more hacks
        if (center_dirty) {
            // move iterator to end
            const auto iterator_index_swap = m_iterator.exchange(indices_size);
            if (iterator_index_swap > indices_size)
                m_iterator.fetch_add(iterator_index_swap - indices_size);
        }

        const auto iterator_index = m_iterator.fetch_add(1);
        if (iterator_index >= indices_size) {
            if (iterator_index == indices_size + cfg::WORKER_THREAD_COUNT - 1) {
                // you are last
                clearMeshReadines();
                m_iterator.store(0);
                
                std::unique_lock<std::mutex> lock{ m_center_lock };
                m_loader_center_chunk = m_actual_center_chunk;
                while (m_center_dirty.exchange(false) == false)
                    m_condition.wait(lock);

                // TODO: consider potential issue: if this if block takes longer than next call
                //       to invalidateMeshWithBlockRange() or moveCenterChunk()
                //       the iterator will reset again and again causing no chunks to be loaded
                //       simulate with: std::this_thread::sleep_for(std::chrono::milliseconds{ 1000 });
            }

            m_barrier.wait();

            if (m_workers_running) {
                continue;
            } else {
                // trap workers to prevent some getting stuck in other barriers
                m_workers_finished.fetch_add(1);
                do {
                    m_barrier.wait();
                } while (m_workers_finished.load() < cfg::WORKER_THREAD_COUNT);
                break;
            }
        }

        const auto chunk_position = m_voxel_indices[iterator_index] + m_loader_center_chunk;
        const auto chunk_index = Math::position_to_index(chunk_position, cfg::CHUNK_ARRAY_SIZE);
        bool chunk_valid;
        const auto old_chunk_position = Math::toVec3<cfg::Coord>(m_chunk_positions[chunk_index].load(), chunk_valid);
        if (!glm::all(glm::equal(chunk_position, old_chunk_position))) {
            if (m_chunk_dirty[chunk_index]) {
                const auto old_region_position = Math::floor_div(old_chunk_position, cfg::REGION_SIZE);
                auto old_region = fetchRegionUseWorkerCache(old_region_position, worker_data);
                saveChunk(m_blocks.data() + chunk_index * cfg::CHUNK_VOLUME, old_chunk_position, old_region);
                m_chunk_dirty[chunk_index] = false;
            }
            const auto region_position = Math::floor_div(chunk_position, cfg::REGION_SIZE);
            auto region = fetchRegionUseWorkerCache(region_position, worker_data);
            m_chunk_positions[chunk_index].store(Math::toDumb3(chunk_position, false));
            if (false == tryLoadChunk(m_blocks.data() + chunk_index * cfg::CHUNK_VOLUME, chunk_position, region)) {
                generateChunk(m_blocks.data() + chunk_index * cfg::CHUNK_VOLUME, chunk_position);
                m_chunk_dirty[chunk_index] = cfg::SAVE_NEWLY_GENERATED_CHUNKS;
            } else {
                m_chunk_dirty[chunk_index] = false;
            }
            m_chunk_positions[chunk_index].store(Math::toDumb3(chunk_position, true));
        }

        std::array<glm::tvec3<cfg::Coord>, cfg::CHUNK_MESH_VOLUME> meshes_to_load;
        const auto meshes_to_load_count = markMeshes(chunk_position, meshes_to_load);
        for (std::size_t i = 0; i < meshes_to_load_count; ++i) {
            Mesh mesh;
            mesh.position = meshes_to_load[i];
            const auto mesh_index = Math::position_to_index(meshes_to_load[i], cfg::MESH_ARRAY_SIZE);
            generateMesh(meshes_to_load[i], mesh.mesh);
            // must be set after generating mesh
            bool old_mesh_valid;
            const auto old_mesh_position = Math::toVec3<cfg::Coord>(m_mesh_positions[mesh_index].load(), old_mesh_valid);
            // if same position, just replace (already implemented in VoxelScene), don't erase + insert
            if (m_mesh_empties[mesh_index] == false) {
                // empty vector indicates remove that mesh
                Mesh mm;
                mm.position = old_mesh_position;
                m_mesh_queue.push(std::move(mm));
            }
            m_mesh_positions[mesh_index].store(Math::toDumb3(meshes_to_load[i], true));
            if (mesh.mesh.size() > 0) {
                m_mesh_empties[mesh_index] = false;
                m_mesh_queue.push(std::move(mesh));
            } else {
                m_mesh_empties[mesh_index] = true;
            }
        }
    }
}

void VoxelContainer::generateMesh(const glm::tvec3<cfg::Coord> & mesh_position, std::vector<cfg::Vertex> & mesh) {
    // check if mesh really not generated from before
    const auto mesh_index = Math::position_to_index(mesh_position, cfg::MESH_ARRAY_SIZE);

    // collect needed chunks (not really necessary, original array could be directly adressed)
    std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> chunks;
    glm::tvec3<cfg::Coord> i;
    std::size_t j{ 0 };
    for (i.z = mesh_position.z + cfg::MESH_CHUNK_START.z; i.z < mesh_position.z + cfg::MESH_CHUNK_END.z; ++i.z)
        for (i.y = mesh_position.y + cfg::MESH_CHUNK_START.y; i.y < mesh_position.y + cfg::MESH_CHUNK_END.y; ++i.y)
            for (i.x = mesh_position.x + cfg::MESH_CHUNK_START.x; i.x < mesh_position.x + cfg::MESH_CHUNK_END.x; ++i.x) {
                // assuming chunks are loaded now
                const auto chunk_index = Math::position_to_index(i, cfg::CHUNK_ARRAY_SIZE);
                chunks[j++] = m_blocks.data() + cfg::CHUNK_VOLUME * chunk_index;
            }
    // generate mesh
    return mesher::mesh<mesher::MesherType::STANDARD>(mesh, chunks);
}

void VoxelContainer::clearMeshReadines() {
    std::for_each(std::begin(m_mesh_readines), std::end(m_mesh_readines), [this](std::atomic<MeshReadinesType> & value){
        value.store(0);
    });
}

std::size_t VoxelContainer::markMeshes(const glm::tvec3<cfg::Coord> & chunk_position, std::array<glm::tvec3<cfg::Coord>, cfg::CHUNK_MESH_VOLUME> & meshes_to_load) {
    glm::tvec3<cfg::Coord> i;
    MeshReadinesType mask{ 1 };
    std::size_t meshes_to_load_count{ 0 };
    for (i.z = chunk_position.z + cfg::CHUNK_MESH_START.z; i.z < chunk_position.z + cfg::CHUNK_MESH_END.z; ++i.z)
        for (i.y = chunk_position.y + cfg::CHUNK_MESH_START.y; i.y < chunk_position.y + cfg::CHUNK_MESH_END.y; ++i.y)
            for (i.x = chunk_position.x + cfg::CHUNK_MESH_START.x; i.x < chunk_position.x + cfg::CHUNK_MESH_END.x; ++i.x) {
                const auto mesh_index = Math::position_to_index(i, cfg::MESH_ARRAY_SIZE);
                const MeshReadinesType state = m_mesh_readines[mesh_index].fetch_or(mask) | mask;
                mask <<= 1;
                bool mesh_valid;
                if (
                    state == ALL_CHUNKS_READY &&
                    !glm::all(glm::equal(i, Math::toVec3<cfg::Coord>(m_mesh_positions[mesh_index].load(), mesh_valid)))
                    )
                    meshes_to_load[meshes_to_load_count++] = i;
                else if (state == ALL_CHUNKS_READY && !mesh_valid)
                    meshes_to_load[meshes_to_load_count++] = i;
            }
    return meshes_to_load_count;
}

bool VoxelContainer::checkMeshes(const glm::tvec3<cfg::Coord> & chunk_position) {
    glm::tvec3<cfg::Coord> i;
    for (i.z = chunk_position.z + cfg::CHUNK_MESH_START.z; i.z < chunk_position.z + cfg::CHUNK_MESH_END.z; ++i.z)
        for (i.y = chunk_position.y + cfg::CHUNK_MESH_START.y; i.y < chunk_position.y + cfg::CHUNK_MESH_END.y; ++i.y)
            for (i.x = chunk_position.x + cfg::CHUNK_MESH_START.x; i.x < chunk_position.x + cfg::CHUNK_MESH_END.x; ++i.x) {
                const auto mesh_index = Math::position_to_index(i, cfg::MESH_ARRAY_SIZE);
                bool mesh_valid;
                if (!glm::all(glm::equal(i, Math::toVec3<cfg::Coord>(m_mesh_positions[mesh_index].load(), mesh_valid))))
                    return false;
                if (!mesh_valid)
                    return false;
            }
    return true;
}

void VoxelContainer::generateChunk(cfg::Block * chunk, const glm::tvec3<cfg::Coord> & chunk_position) {
    // TODO: IDEA: second pass: if this is the last neighbour of any chunk that neighbour chunk can do a second loading pass (for more advanced and "non deterministic" world generators)
    worldgen::generate<worldgen::WorldGenType::STANDARD>(chunk, chunk_position);
}

void VoxelContainer::saveChunk(const cfg::Block * chunk, const glm::tvec3<cfg::Coord> & chunk_position, Region * region) {
    assert(chunk != nullptr && region != nullptr);
    Print("Save ", glm::to_string(chunk_position));
    region->saveChunk(Math::position_to_index(chunk_position, cfg::REGION_SIZE), chunk);
}

bool VoxelContainer::tryLoadChunk(cfg::Block * chunk, const glm::tvec3<cfg::Coord> & chunk_position, Region * region) {
    assert(chunk != nullptr && region != nullptr);
    return region->loadChunk(Math::position_to_index(chunk_position, cfg::REGION_SIZE), chunk);
}

Region * VoxelContainer::fetchRegionUseWorkerCache(const glm::tvec3<cfg::Coord> & region_position, WorkerData & worker_data) {
    const auto region_index = Math::position_to_index(region_position, cfg::WORKER_REGION_CACHE_SIZE);
    Region * * region = worker_data.regions.data() + region_index;
    glm::tvec3<cfg::Coord> * position = worker_data.positions.data() + region_index;
    if (*region != nullptr) {
        if (glm::all(glm::equal(*position, region_position)))
            return *region;
        else
            m_region_container.release(*position);
    }
    *position = region_position;
    // don't worry about a pointer being overwritten, its all being taken care of :)
    *region = &m_region_container.get(region_position);
    return *region;
}
