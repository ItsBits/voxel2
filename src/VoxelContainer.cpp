#include "VoxelContainer.hpp"

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "mesher.hpp"
#include "Print.hpp"

VoxelContainer::VoxelContainer() {
    std::fill(std::begin(m_chunk_positions), std::end(m_chunk_positions), glm::tvec3<cfg::Coord>{ 0, 0, 0 });
    std::fill(std::begin(m_mesh_positions), std::end(m_mesh_positions), glm::tvec3<cfg::Coord>{ 0, 0, 0 });
    m_chunk_positions[0].x = 1;
    m_mesh_positions[0].x = 1;
    std::fill(std::begin(m_blocks), std::end(m_blocks), cfg::Block{ 0 });
    m_workers_running.store(true);
    m_iterator.store(0);
    m_center_chunk = { 0, 0, 0 };
    clearMeshReadines();
    std::for_each(std::begin(m_workers), std::end(m_workers), [this](std::thread & worker){
        worker = std::thread{ &VoxelContainer::worker, this };
    });
}

VoxelContainer::~VoxelContainer() {
    m_workers_running.store(false);
    std::for_each(std::begin(m_workers), std::end(m_workers), [this](std::thread & worker){
        worker.join();
    });
}

void VoxelContainer::worker() {
    while (m_workers_running) {
        // TODO: if ended or reseting => main worker thread reset state after all hit barrier

        const auto iter = m_iterator.fetch_add(1);
        // TODO: sleep instead of return
        if (iter >= m_voxel_indices.size())
            return;

        const auto chunk_position = m_voxel_indices[iter] + m_center_chunk;
        const auto chunk_index = Math::position_to_index(chunk_position, cfg::CHUNK_ARRAY_SIZE);
        if (!glm::all(glm::equal(chunk_position, m_chunk_positions[chunk_index]))) {
            m_chunk_positions[chunk_index] = chunk_position;
            generateChunk(m_blocks.data() + chunk_index * cfg::CHUNK_VOLUME, chunk_position);
        }

        std::array<glm::tvec3<cfg::Coord>, cfg::CHUNK_MESH_VOLUME> meshes_to_load;
        const auto meshes_to_load_count = markMeshes(chunk_position, meshes_to_load);
        for (std::size_t i = 0; i < meshes_to_load_count; ++i) {
            Mesh mesh;
            mesh.position = meshes_to_load[i];
            const auto mesh_index = Math::position_to_index(meshes_to_load[i], cfg::MESH_ARRAY_SIZE);
            generateMesh(meshes_to_load[i], mesh.mesh);
            m_mesh_positions[mesh_index] = meshes_to_load[i];
            m_mesh_queue.push(std::move(mesh));
        }
    }
}

void VoxelContainer::generateMesh(const glm::tvec3<cfg::Coord> & mesh_position, std::vector<cfg::Vertex> & mesh) {
    // check if mesh really not generated from before
    const auto index = Math::position_to_index(mesh_position, cfg::MESH_ARRAY_SIZE);
    if (glm::all(glm::equal(mesh_position, m_mesh_positions[index])))
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
                const auto index = Math::position_to_index(i, cfg::CHUNK_ARRAY_SIZE);
                chunks[j++] = m_blocks.data() + cfg::CHUNK_VOLUME * index;
                const auto correct = i;
                const auto actual = m_chunk_positions[index];
                if (!glm::all(glm::equal(correct, actual))) {
                    for (size_t l = 0; l < m_chunk_positions.size(); ++l)
                        Print("->", glm::to_string(m_chunk_positions[l]), ' ', l);
                    int dummy = 42;
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
                const auto index = Math::position_to_index(i, cfg::MESH_ARRAY_SIZE);
                const MeshReadinesType state = m_mesh_readines[index].fetch_or(mask) | mask;
                mask <<= 1;
                if (state == ALL_CHUNKS_READY)
                    meshes_to_load[meshes_to_load_count++] = i;
            }
    return meshes_to_load_count;
}

void VoxelContainer::generateChunk(cfg::Block * chunk, const glm::tvec3<cfg::Coord> & chunk_position) {
    glm::tvec3<cfg::Coord> i;
    const glm::tvec3<cfg::Coord> fr{ chunk_position * cfg::CHUNK_SIZE };
    const glm::tvec3<cfg::Coord> to{ fr + cfg::CHUNK_SIZE };
    for (i.z = fr.z; i.z < to.z; ++i.z)
        for (i.y = fr.y; i.y < to.y; ++i.y)
            for (i.x = fr.x; i.x < to.x; ++i.x) {
                const auto index = Math::position_to_index(i, cfg::CHUNK_SIZE);
                if (i.y < 0)
                    chunk[index] = std::rand() % 100 == 0;
                else
                    chunk[index] = 0;

/*
                const auto index = Math::position_to_index(i, cfg::CHUNK_SIZE);
                const auto f = Math::floor_mod(i, cfg::CHUNK_SIZE);
                if (glm::any(glm::equal(f, cfg::CHUNK_SIZE - 1)) || glm::any(glm::equal(f, glm::tvec3<cfg::Coord>{ 0, 0, 0 })))
                    chunk[index] = 0;
                else {
                    chunk[index] = std::rand();
                    if (chunk[index] == 0) ++chunk[index];
                }
*/

            
//                chunk[index] = i.y == -5;
                //chunk[index] = std::rand() % 10 == 0;
/*
                if (glm::all(glm::equal(chunk_position, glm::tvec3<cfg::Coord>{ 0, 0, 0 }))) {
                    chunk[index] = std::rand();
                    if (chunk[index] == 0) chunk[index] = 100;
                } else {
                    chunk[index] = 0;
                }
*/
            }
}
