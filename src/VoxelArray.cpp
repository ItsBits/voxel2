#include "VoxelArray.hpp"

VoxelArray::VoxelArray() {
    m_running = true;
    m_workers[0] = std::thread{ &VoxelArray::worker, this };
}

VoxelArray::~VoxelArray() {
    m_running = false;
    m_workers[0].join();
}

void VoxelArray::update(const glm::ivec3 & center) {
    /*const Vec3<int8_t> r = m_iterator.getRadius();
    for(auto m = m_meshes.begin(); m != m_meshes.end();) {
        const glm::ivec3 pp{ m->position.x, m->position.y, m->position.z };
        const glm::ivec3 distance = glm::abs(center - pp);
        if (distance.x > r.x || distance.y > r.y || distance.z > r.z) {
            glDeleteBuffers(1, &m->VBO);
            glDeleteVertexArrays(1, &m->VAO);
            m = m_meshes.erase(m);
        } else {
            ++m;
        }
    }*/

    size_t this_loop = 0;
    const auto & iter = m_iterator.indices();
    int64_t index = -1;
    //for (size_t i = 0, size = it.size(); i < size; ++i) {
    for (const auto & i : iter) {
        ++index;
        int8_t type = i.w;
        glm::ivec3 p{ i.x, i.y, i.z };
        p += center;

        if (type == VoxelArrayIterator::LOAD_CHUNK) {
            const glm::ivec3 pp{
                floor_mod_i(p.x, cfg::CHUNK_ARRAY_SIZE_X),
                floor_mod_i(p.y, cfg::CHUNK_ARRAY_SIZE_Y),
                floor_mod_i(p.z, cfg::CHUNK_ARRAY_SIZE_Z)
                };

            ChunkMeta * chunk_meta = m_positions.get(pp.x, pp.y, pp.z);
            if (!all_equal(chunk_meta->position, {p.x, p.y, p.z})) {
                // TODO: fix, don't cast to uint8_t
                m_tasks.push({ (int8_t)p.x, (int8_t)p.y, (int8_t)p.z, VoxelArrayIterator::LOAD_CHUNK });
                ++this_loop;
            }
        } else if (type == VoxelArrayIterator::LOAD_MESH) {

        } else {
            assert(0);
        }

        if (this_loop++ > 10) break;

/*
        const auto mesh_position = p + center;
        const auto mesh_iterator = m_meshes.find(mesh_position);
        if (mesh_iterator == m_meshes.end()) {
            const auto chunk_mesh = generateAndUploadChunkMesh(vs, mesh_position);
            static size_t j = 0;
            std::cout << j++ << std::endl;
            m_meshes.insert({ mesh_position, chunk_mesh });

            // TODO: improve
            if (this_loop++ > 10) break;
        }
*/
    }
}

std::vector<uint8_t> VoxelArray::generateChunkMesh(const Vec3<int32_t> & mesh_positionn) {
    const glm::ivec3 mesh_position{ mesh_positionn.x, mesh_positionn.y, mesh_positionn.z };
    constexpr glm::ivec3 MESH_SIZES{ cfg::MESH_SIZE_X, cfg::MESH_SIZE_Y, cfg::MESH_SIZE_Z };
    constexpr glm::ivec3 MESH_OFFSETS{ cfg::MESH_OFFSET_X, cfg::MESH_OFFSET_Y, cfg::MESH_OFFSET_Z };
    const glm::ivec3 from_block{ mesh_position * MESH_SIZES + MESH_OFFSETS };
    const glm::ivec3 to_block{ from_block + MESH_SIZES };
    const uint8_t * blockss[8];
    static_assert(MESH_SIZES.x == cfg::CHUNK_SIZE_X);
    static_assert(MESH_SIZES.y == cfg::CHUNK_SIZE_Y);
    static_assert(MESH_SIZES.z == cfg::CHUNK_SIZE_Z);
    static_assert(cfg::MESH_OFFSET_X > 0);
    static_assert(cfg::MESH_OFFSET_Y > 0);
    static_assert(cfg::MESH_OFFSET_Z > 0);
    static_assert(cfg::MESH_OFFSET_X < MESH_SIZES.x);
    static_assert(cfg::MESH_OFFSET_Y < MESH_SIZES.y);
    static_assert(cfg::MESH_OFFSET_Z < MESH_SIZES.z);
    size_t k = 0;
    for (int z = mesh_positionn.z; z <= mesh_positionn.z + 1; ++z)
        for (int y = mesh_positionn.y; y <= mesh_positionn.y + 1; ++y)
            for (int x = mesh_positionn.x; x <= mesh_positionn.x + 1; ++x) {
                const int32_t xxx = floor_mod_i(x, cfg::CHUNK_ARRAY_SIZE_X);
                const int32_t yyy = floor_mod_i(y, cfg::CHUNK_ARRAY_SIZE_Y);
                const int32_t zzz = floor_mod_i(z, cfg::CHUNK_ARRAY_SIZE_Z);
                ChunkMeta * chunk_meta = m_positions.get(xxx, yyy, zzz);
                assert(all_equal(Vec3<int32_t>{ x, y, z }, chunk_meta->position));
                Block * blocks = m_chunks.get(xxx, yyy, zzz);
                blockss[k++] = blocks;
            }
//    const uint8_t * blockss = vs.get({ chunk_position.x, chunk_position.y, chunk_position.z }, true, false);
    const auto block_get = [this, blockss, mesh_position] (glm::ivec3 p) -> uint8_t {
        glm::ivec3 pp = p;
        // floor division
        // r[i] = (x[i] + (x[i] < 0)) / y[i] - (x[i] < 0)
        pp.x = (pp.x + (pp.x < 0)) / cfg::CHUNK_SIZE_X - (pp.x < 0);
        pp.y = (pp.y + (pp.y < 0)) / cfg::CHUNK_SIZE_Y - (pp.y < 0);
        pp.z = (pp.z + (pp.z < 0)) / cfg::CHUNK_SIZE_Z - (pp.z < 0);
        // TODO: handle out of ranges (aka. decouble mesh from chunk)
//        if (pp.x != chunk_position.x || pp.y != chunk_position.y || pp.z != chunk_position.z)
//            return 0;
        pp -= mesh_position - 1;
        assert(pp.x >= 0 && pp.y >= 0 && pp.z >= 0 && pp.x < 3 && pp.y < 3 && pp.z < 3);


        // floor modulus
        // r[i] = (x[i] % y[i] + y[i]) % y[i]
        p.x = (p.x % cfg::CHUNK_SIZE_X + cfg::CHUNK_SIZE_X) % cfg::CHUNK_SIZE_X;
        p.y = (p.y % cfg::CHUNK_SIZE_Y + cfg::CHUNK_SIZE_Y) % cfg::CHUNK_SIZE_Y;
        p.z = (p.z % cfg::CHUNK_SIZE_Z + cfg::CHUNK_SIZE_Z) % cfg::CHUNK_SIZE_Z;
        // TODO: don't hardcode lookup array dimensions (but calculate from mesh dimensions)
        return blockss[pp.z * 9 + pp.y * 3 + pp.x][p.z * cfg::CHUNK_SIZE_Y * cfg::CHUNK_SIZE_X + p.y * cfg::CHUNK_SIZE_X + p.x];
    };

    struct Vertex {
        uint8_t x, y, z, c, a0, a1, a2, a3;
    };

    constexpr std::uint8_t SHADOW_STRENGTH{ 60 };

    std::vector<Vertex> mesh;
    mesh.reserve(1024*1024); // whatever

    glm::ivec3 i;

    const glm::ivec3 offset{ from_block };
    for (i[2] = from_block[2]; i[2] < to_block[2]; ++i[2])
        for (i[1] = from_block[1]; i[1] < to_block[1]; ++i[1])
            for (i[0] = from_block[0]; i[0] < to_block[0]; ++i[0])
            {
                auto block = block_get(i); // TODO: this can in future probably be set to reference
                if (block == 0) continue;

                block = std::rand() % 254 + 1;

                // X + 1
                if (block_get({ i[0] + 1, i[1], i[2] }) == 0)
                {
                    const bool aos[8]{
                        block_get({ i[0] + 1, i[1] - 1, i[2]     }) != 0,
                        block_get({ i[0] + 1, i[1]    , i[2] - 1 }) != 0,
                        block_get({ i[0] + 1, i[1] + 1, i[2]     }) != 0,
                        block_get({ i[0] + 1, i[1]    , i[2] + 1 }) != 0,

                        block_get({ i[0] + 1, i[1] - 1, i[2] - 1 }) != 0,
                        block_get({ i[0] + 1, i[1] - 1, i[2] + 1 }) != 0,
                        block_get({ i[0] + 1, i[1] + 1, i[2] + 1 }) != 0,
                        block_get({ i[0] + 1, i[1] + 1, i[2] - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        vertexAO(aos[0], aos[3], aos[5]),
                        vertexAO(aos[2], aos[3], aos[6]),
                        vertexAO(aos[0], aos[1], aos[4]),
                        vertexAO(aos[2], aos[1], aos[7])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - offset;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(Vertex{ quad_po1[0], quad_pos[1], quad_po1[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1[0], quad_pos[1], quad_pos[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1[0], quad_po1[1], quad_pos[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1[0], quad_po1[1], quad_po1[2], block, ao[0], ao[1], ao[2], ao[3] });
                }

                // X - 1
                if (block_get({ i[0] - 1, i[1], i[2] }) == 0)
                {
                    const bool aos[8]{
                        block_get({ i[0] - 1, i[1] - 1, i[2]     }) != 0,
                        block_get({ i[0] - 1, i[1]    , i[2] - 1 }) != 0,
                        block_get({ i[0] - 1, i[1] + 1, i[2]     }) != 0,
                        block_get({ i[0] - 1, i[1]    , i[2] + 1 }) != 0,

                        block_get({ i[0] - 1, i[1] - 1, i[2] - 1 }) != 0,
                        block_get({ i[0] - 1, i[1] - 1, i[2] + 1 }) != 0,
                        block_get({ i[0] - 1, i[1] + 1, i[2] + 1 }) != 0,
                        block_get({ i[0] - 1, i[1] + 1, i[2] - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        vertexAO(aos[0], aos[1], aos[4]),
                        vertexAO(aos[2], aos[1], aos[7]),
                        vertexAO(aos[0], aos[3], aos[5]),
                        vertexAO(aos[2], aos[3], aos[6])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - offset;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(Vertex{ quad_pos[0], quad_pos[1], quad_pos[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0], quad_pos[1], quad_po1[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0], quad_po1[1], quad_po1[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0], quad_po1[1], quad_pos[2], block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Y + 1
                if (block_get({ i[0], i[1] + 1, i[2] }) == 0)
                {
                    const bool aos[8]{
                        block_get({ i[0] - 1, i[1] + 1, i[2]     }) != 0,
                        block_get({ i[0]    , i[1] + 1, i[2] - 1 }) != 0,
                        block_get({ i[0] + 1, i[1] + 1, i[2]     }) != 0,
                        block_get({ i[0]    , i[1] + 1, i[2] + 1 }) != 0,

                        block_get({ i[0] - 1, i[1] + 1, i[2] - 1 }) != 0,
                        block_get({ i[0] - 1, i[1] + 1, i[2] + 1 }) != 0,
                        block_get({ i[0] + 1, i[1] + 1, i[2] + 1 }) != 0,
                        block_get({ i[0] + 1, i[1] + 1, i[2] - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        vertexAO(aos[2], aos[1], aos[7]),
                        vertexAO(aos[2], aos[3], aos[6]),
                        vertexAO(aos[0], aos[1], aos[4]),
                        vertexAO(aos[0], aos[3], aos[5])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - offset;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(Vertex{ quad_po1[0], quad_po1[1], quad_pos[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0], quad_po1[1], quad_pos[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0], quad_po1[1], quad_po1[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1[0], quad_po1[1], quad_po1[2], block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Y - 1
                if (block_get({ i[0], i[1] - 1, i[2] }) == 0)
                {
                    const bool aos[8]{
                        block_get({ i[0] - 1, i[1] - 1, i[2]     }) != 0,
                        block_get({ i[0]    , i[1] - 1, i[2] - 1 }) != 0,
                        block_get({ i[0] + 1, i[1] - 1, i[2]     }) != 0,
                        block_get({ i[0]    , i[1] - 1, i[2] + 1 }) != 0,

                        block_get({ i[0] - 1, i[1] - 1, i[2] - 1 }) != 0,
                        block_get({ i[0] - 1, i[1] - 1, i[2] + 1 }) != 0,
                        block_get({ i[0] + 1, i[1] - 1, i[2] + 1 }) != 0,
                        block_get({ i[0] + 1, i[1] - 1, i[2] - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        vertexAO(aos[0], aos[1], aos[4]),
                        vertexAO(aos[0], aos[3], aos[5]),
                        vertexAO(aos[2], aos[1], aos[7]),
                        vertexAO(aos[2], aos[3], aos[6])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - offset;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(Vertex{ quad_pos[0], quad_pos[1], quad_pos[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1[0], quad_pos[1], quad_pos[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1[0], quad_pos[1], quad_po1[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0], quad_pos[1], quad_po1[2], block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Z + 1
                if (block_get({ i[0], i[1], i[2] + 1 }) == 0)
                {
                    const bool aos[8]{
                        block_get({ i[0] - 1, i[1]    , i[2] + 1 }) != 0,
                        block_get({ i[0]    , i[1] - 1, i[2] + 1 }) != 0,
                        block_get({ i[0] + 1, i[1]    , i[2] + 1 }) != 0,
                        block_get({ i[0]    , i[1] + 1, i[2] + 1 }) != 0,

                        block_get({ i[0] - 1, i[1] - 1, i[2] + 1 }) != 0,
                        block_get({ i[0] - 1, i[1] + 1, i[2] + 1 }) != 0,
                        block_get({ i[0] + 1, i[1] + 1, i[2] + 1 }) != 0,
                        block_get({ i[0] + 1, i[1] - 1, i[2] + 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        vertexAO(aos[0], aos[1], aos[4]),
                        vertexAO(aos[0], aos[3], aos[5]),
                        vertexAO(aos[2], aos[1], aos[7]),
                        vertexAO(aos[2], aos[3], aos[6])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - offset;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(Vertex{ quad_pos[0], quad_pos[1], quad_po1[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1[0], quad_pos[1], quad_po1[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1[0], quad_po1[1], quad_po1[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0], quad_po1[1], quad_po1[2], block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Z - 1
                if (block_get({ i[0], i[1], i[2] - 1 }) == 0)
                {
                    const bool aos[8]{
                        block_get({ i[0] - 1, i[1]    , i[2] - 1 }) != 0,
                        block_get({ i[0]    , i[1] - 1, i[2] - 1 }) != 0,
                        block_get({ i[0] + 1, i[1]    , i[2] - 1 }) != 0,
                        block_get({ i[0]    , i[1] + 1, i[2] - 1 }) != 0,

                        block_get({ i[0] - 1, i[1] - 1, i[2] - 1 }) != 0,
                        block_get({ i[0] - 1, i[1] + 1, i[2] - 1 }) != 0,
                        block_get({ i[0] + 1, i[1] + 1, i[2] - 1 }) != 0,
                        block_get({ i[0] + 1, i[1] - 1, i[2] - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        vertexAO(aos[2], aos[1], aos[7]),
                        vertexAO(aos[2], aos[3], aos[6]),
                        vertexAO(aos[0], aos[1], aos[4]),
                        vertexAO(aos[0], aos[3], aos[5])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - offset;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    uint8_t tt = uint8_t(quad_pos[1]) + uint8_t(1);

                    mesh.push_back(Vertex{ quad_po1[0], quad_pos[1], quad_pos[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0], quad_pos[1], quad_pos[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0], quad_po1[1], quad_pos[2], block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1[0], quad_po1[1], quad_pos[2], block, ao[0], ao[1], ao[2], ao[3] });
                }
            }

    std::vector<uint8_t> tmp_mesh;
    tmp_mesh.reserve(mesh.size() * 8);
    for (const auto & v : mesh) {
        tmp_mesh.push_back(v.x);
        tmp_mesh.push_back(v.y);
        tmp_mesh.push_back(v.z);
        tmp_mesh.push_back(v.c);
        tmp_mesh.push_back(v.a0);
        tmp_mesh.push_back(v.a1);
        tmp_mesh.push_back(v.a2);
        tmp_mesh.push_back(v.a3);
    }
    return std::move(tmp_mesh);
}
