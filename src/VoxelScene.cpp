#include "VoxelScene.hpp"
#include <vector>

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

//==============================================================================
std::vector<uint8_t> VoxelScene::generateChunkMesh(VoxelStorage & vs, const glm::ivec3 & chunk_position) {
    const glm::ivec3 from_block{ chunk_position * CHUNK_SIZES };
    const glm::ivec3 to_block{ from_block + CHUNK_SIZES };
    const uint8_t * blockss[27];
    size_t k = 0;
    for (int z = chunk_position.z - 1; z <= chunk_position.z + 1; ++z)
        for (int y = chunk_position.y - 1; y <= chunk_position.y + 1; ++y)
            for (int x = chunk_position.x - 1; x <= chunk_position.x + 1; ++x) {
                VoxelStorage::ChunkPtr p = vs.get(x, y, z, true, false);
                if (p.n) {
                    std::cout << "new" << std::endl;
                    create_new_chunk(p.b, { x, y, z });
                }
                blockss[k++] = p.b;
            }
//    const uint8_t * blockss = vs.get({ chunk_position.x, chunk_position.y, chunk_position.z }, true, false);
    const auto block_get = [this, blockss, chunk_position] (glm::ivec3 p) -> uint8_t {
        glm::ivec3 pp = p;
        // floor division
        // r[i] = (x[i] + (x[i] < 0)) / y[i] - (x[i] < 0)
        pp.x = (pp.x + (pp.x < 0)) / CHUNK_SIZES.x - (pp.x < 0);
        pp.y = (pp.y + (pp.y < 0)) / CHUNK_SIZES.y - (pp.y < 0);
        pp.z = (pp.z + (pp.z < 0)) / CHUNK_SIZES.z - (pp.z < 0);
        // TODO: handle out of ranges (aka. decouble mesh from chunk)
//        if (pp.x != chunk_position.x || pp.y != chunk_position.y || pp.z != chunk_position.z)
//            return 0;
        pp -= chunk_position - 1;
        assert(pp.x >= 0 && pp.y >= 0 && pp.z >= 0 && pp.x < 3 && pp.y < 3 && pp.z < 3);


        // floor modulus
        // r[i] = (x[i] % y[i] + y[i]) % y[i]
        p.x = (p.x % CHUNK_SIZES.x + CHUNK_SIZES.x) % CHUNK_SIZES.x;
        p.y = (p.y % CHUNK_SIZES.y + CHUNK_SIZES.y) % CHUNK_SIZES.y;
        p.z = (p.z % CHUNK_SIZES.z + CHUNK_SIZES.z) % CHUNK_SIZES.z;
        // TODO: don't hardcode lookup array dimensions (but calculate from mesh dimensions)
        return blockss[pp.z * 9 + pp.y * 3 + pp.x][p.z * CHUNK_SIZES.y * CHUNK_SIZES.x + p.y * CHUNK_SIZES.x + p.x];
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
