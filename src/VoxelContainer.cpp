#include "VoxelContainer.hpp"

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "mesher.hpp"
#include "Print.hpp"
#include "worldgen.hpp"

VoxelContainer::VoxelContainer() :
    m_barrier{ cfg::WORKER_THREAD_COUNT }
{
    std::for_each(std::begin(m_chunk_positions), std::end(m_chunk_positions), [this](std::atomic<Math::DumbVec3> & vector){
        vector.store(Math::toDumb3(glm::tvec3<cfg::Coord>{ 0, 0, 0 }));
    });
    m_chunk_positions[0].store(Math::toDumb3(glm::tvec3<cfg::Coord>{ 1, 0, 0 }));
    std::for_each(std::begin(m_mesh_positions), std::end(m_mesh_positions), [this](std::atomic<Math::DumbVec3> & vector){
        vector.store(Math::toDumb3(glm::tvec3<cfg::Coord>{ 0, 0, 0 }));
    });
    m_mesh_positions[0].store(Math::toDumb3(glm::tvec3<cfg::Coord>{ 1, 0, 0 }));
    //std::fill(std::begin(m_mesh_positions), std::end(m_mesh_positions), glm::tvec3<cfg::Coord>{ 0, 0, 0 });
    //m_mesh_positions[0].x = 1;
    std::fill(std::begin(m_blocks), std::end(m_blocks), cfg::Block{ 0 });
    m_workers_running.store(true);
    m_iterator.store(0);
    m_loader_center_chunk = { 0, 0, 0 };
    m_actual_center_chunk = { 0, 0, 0 };
    m_center_dirty.store(false);
    clearMeshReadines();
    std::for_each(std::begin(m_workers), std::end(m_workers), [this](std::thread & worker){
        worker = std::thread{ &VoxelContainer::worker, this };
    });
}

VoxelContainer::~VoxelContainer() {
    m_workers_running.store(false);
    // speed up shutdown of workers
    m_center_dirty.store(true);
    std::for_each(std::begin(m_workers), std::end(m_workers), [this](std::thread & worker){
        worker.join();
    });
}

void VoxelContainer::moveCenterChunk(const glm::tvec3<cfg::Coord> & new_center_chunk) {
    const auto changed = !glm::all(glm::equal(m_actual_center_chunk, new_center_chunk));
    std::lock_guard<std::mutex> lock{ m_center_lock };
    m_actual_center_chunk = new_center_chunk;
    m_center_chunk_overlap = Math::overlap(
        Math::toAABB3(m_actual_center_chunk, cfg::CHUNK_LOADING_RADIUS),
        Math::toAABB3(m_loader_center_chunk, cfg::CHUNK_LOADING_RADIUS)
    );
    if (changed)
        m_center_dirty.store(true);
}

const cfg::Block * VoxelContainer::getChunk(const glm::tvec3<cfg::Coord> & chunk_position) {
    return getChunkNonConst(chunk_position);
}

cfg::Block * VoxelContainer::getChunkNonConst(const glm::tvec3<cfg::Coord> & chunk_position) {
    const auto chunk_index = Math::position_to_index(chunk_position, cfg::CHUNK_ARRAY_SIZE);
    const auto loaded_chunk_position = Math::toVec3<cfg::Coord>(m_chunk_positions[chunk_index].load());
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
    if (checkMeshes(chunk_position) == true)
        return chunk;
    else
        return nullptr;
}

void VoxelContainer::invalidateMeshWithBlockRange(Math::AABB3<cfg::Coord> range) {
    range.min = Math::floor_div(range.min - Math::add(cfg::MESH_OFFSET, cfg::BLOCK_MESH_EFFECT_RADIUS), cfg::MESH_SIZE);
    range.max = Math::floor_div(range.max - Math::sub(cfg::MESH_OFFSET, cfg::BLOCK_MESH_EFFECT_RADIUS), cfg::MESH_SIZE);
    glm::tvec3<cfg::Coord> i;
    for (i.z = range.min.z; i.z <= range.max.z; ++i.z)
        for (i.y = range.min.y; i.y <= range.max.y; ++i.y)
            for (i.x = range.min.x; i.x <= range.max.x; ++i.x) {
                const auto mesh_index = Math::position_to_index(i, cfg::MESH_ARRAY_SIZE);
                m_mesh_positions[mesh_index].store(Math::toDumb3(i + glm::tvec3<cfg::Coord>{ 0, 0, 1 }));                
            }
    m_center_dirty.store(true);
}

void VoxelContainer::worker() {
    const auto indices_size = m_voxel_indices.size();
    while (true) {
        const auto center_dirty = m_center_dirty.exchange(false);
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
                {
                    std::lock_guard<std::mutex> lock{ m_center_lock };
                    m_loader_center_chunk = m_actual_center_chunk;
                }
                // TODO: do something better
                // TODO: and don't sleep if there is work to do (mesh updates)
                std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
            }
            m_barrier.wait();
            if (m_workers_running)
                continue;
            else
                break;
        }

        const auto chunk_position = m_voxel_indices[iterator_index] + m_loader_center_chunk;
        const auto chunk_index = Math::position_to_index(chunk_position, cfg::CHUNK_ARRAY_SIZE);
        if (!glm::all(glm::equal(chunk_position, Math::toVec3<cfg::Coord>(m_chunk_positions[chunk_index].load())))) {
            m_chunk_positions[chunk_index].store(Math::toDumb3(chunk_position));
            generateChunk(m_blocks.data() + chunk_index * cfg::CHUNK_VOLUME, chunk_position);
        }

        std::array<glm::tvec3<cfg::Coord>, cfg::CHUNK_MESH_VOLUME> meshes_to_load;
        const auto meshes_to_load_count = markMeshes(chunk_position, meshes_to_load);
        for (std::size_t i = 0; i < meshes_to_load_count; ++i) {
            Mesh mesh;
            mesh.position = meshes_to_load[i];
            const auto mesh_index = Math::position_to_index(meshes_to_load[i], cfg::MESH_ARRAY_SIZE);
            generateMesh(meshes_to_load[i], mesh.mesh);
            // must be set after generating mesh
            m_mesh_positions[mesh_index].store(Math::toDumb3(meshes_to_load[i]));
            if (mesh.mesh.size() > 0)
                m_mesh_queue.push(std::move(mesh));
        }
    }
}

void VoxelContainer::generateMesh(const glm::tvec3<cfg::Coord> & mesh_position, std::vector<cfg::Vertex> & mesh) {
    // check if mesh really not generated from before
    const auto mesh_index = Math::position_to_index(mesh_position, cfg::MESH_ARRAY_SIZE);
    if (glm::all(glm::equal(mesh_position, Math::toVec3<cfg::Coord>(m_mesh_positions[mesh_index].load()))))
        return;
    // collect needed chunks (not really necessary, original array could be directly adressed)
    std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> chunks;
    glm::tvec3<cfg::Coord> i;
    std::size_t j{ 0 };
    static std::mutex m;
    std::lock_guard<std::mutex> ll{ m };
    for (i.z = mesh_position.z + cfg::MESH_CHUNK_START.z; i.z < mesh_position.z + cfg::MESH_CHUNK_END.z; ++i.z)
        for (i.y = mesh_position.y + cfg::MESH_CHUNK_START.y; i.y < mesh_position.y + cfg::MESH_CHUNK_END.y; ++i.y)
            for (i.x = mesh_position.x + cfg::MESH_CHUNK_START.x; i.x < mesh_position.x + cfg::MESH_CHUNK_END.x; ++i.x) {
                // assuming chunks are loaded now
                const auto chunk_index = Math::position_to_index(i, cfg::CHUNK_ARRAY_SIZE);
                chunks[j++] = m_blocks.data() + cfg::CHUNK_VOLUME * chunk_index;
                const auto correct = i;
                const auto actual = Math::toVec3<cfg::Coord>(m_chunk_positions[chunk_index].load());
                // TODO: remove
                if (!glm::all(glm::equal(correct, actual))) {
                    for (size_t l = 0; l < m_chunk_positions.size(); ++l)
                        Print("->", glm::to_string(Math::toVec3<cfg::Coord>(m_chunk_positions[l].load())), ' ', l);
                    int dummy = 42;
                    throw 0;
                }
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
                if (
                    state == ALL_CHUNKS_READY &&
                    !glm::all(glm::equal(i, Math::toVec3<cfg::Coord>(m_mesh_positions[mesh_index].load())))
                    )
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
                if (!glm::all(glm::equal(i, Math::toVec3<cfg::Coord>(m_mesh_positions[mesh_index].load()))))
                    return false;
            }
    return true;
}

void VoxelContainer::generateChunk(cfg::Block * chunk, const glm::tvec3<cfg::Coord> & chunk_position) {
    // TODO: IDEA: second pass: if this is the last neighbour of any chunk that neighbour chunk can do a second loading pass (for more advanced and "non deterministic" world generators)
    worldgen::generate<worldgen::WorldGenType::STANDARD>(chunk, chunk_position);
}
