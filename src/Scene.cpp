#include "Scene.hpp"
#include <vector>

//==============================================================================
inline std::uint8_t vertexAO(const bool side_a, const bool side_b, const bool corner)
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
std::vector<uint8_t> Scene::generateChunkMesh(VoxelStorage & vs, const glm::ivec3 & chunk_position) {
    // x, y, z

    const auto block_get = [](const glm::ivec3 & p) -> uint8_t {
        return 0;
    };

    struct Vertex {
        uint8_t x, y, z, c, a0, a1, a2, a3;
    };

    constexpr std::uint8_t SHADOW_STRENGTH{ 60 };

    std::vector<Vertex> mesh;

    glm::ivec3 i;

    const auto CHUNK_SIZES = glm::ivec3{ VoxelStorage::CHUNK_SIZE.x, VoxelStorage::CHUNK_SIZE.y, VoxelStorage::CHUNK_SIZE.z };
    const glm::ivec3 from_block{ chunk_position * CHUNK_SIZES };
    const glm::ivec3 to_block{ from_block + CHUNK_SIZES };

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

                    mesh.push_back(Vertex{ quad_pos[0] + 1, quad_pos[1]    , quad_pos[2] + 1, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0] + 1, quad_pos[1]    , quad_pos[2]    , block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0] + 1, quad_pos[1] + 1, quad_pos[2]    , block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0] + 1, quad_pos[1] + 1, quad_pos[2] + 1, block, ao[0], ao[1], ao[2], ao[3] });
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

                    mesh.push_back(Vertex{ quad_pos[0]    , quad_pos[1]    , quad_pos[2]    , block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0]    , quad_pos[1]    , quad_pos[2] + 1, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0]    , quad_pos[1] + 1, quad_pos[2] + 1, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0]    , quad_pos[1] + 1, quad_pos[2]    , block, ao[0], ao[1], ao[2], ao[3] });
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

                    mesh.push_back(Vertex{ quad_pos[0] + 1, quad_pos[1] + 1, quad_pos[2]    , block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0]    , quad_pos[1] + 1, quad_pos[2]    , block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0]    , quad_pos[1] + 1, quad_pos[2] + 1, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0] + 1, quad_pos[1] + 1, quad_pos[2] + 1, block, ao[0], ao[1], ao[2], ao[3] });
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

                    mesh.push_back(Vertex{ quad_pos[0]    , quad_pos[1]    , quad_pos[2]    , block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0] + 1, quad_pos[1]    , quad_pos[2]    , block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0] + 1, quad_pos[1]    , quad_pos[2] + 1, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0]    , quad_pos[1]    , quad_pos[2] + 1, block, ao[0], ao[1], ao[2], ao[3] });
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

                    mesh.push_back(Vertex{ quad_pos[0]    , quad_pos[1]    , quad_pos[2] + 1, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0] + 1, quad_pos[1]    , quad_pos[2] + 1, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0] + 1, quad_pos[1] + 1, quad_pos[2] + 1, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0]    , quad_pos[1] + 1, quad_pos[2] + 1, block, ao[0], ao[1], ao[2], ao[3] });
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
                    const glm::tvec3<uint8_t> quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };

                    mesh.push_back(Vertex{ quad_pos[0] + 1, quad_pos[1]    , quad_pos[2]     , block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0]    , quad_pos[1]    , quad_pos[2]     , block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0]    , quad_pos[1] + 1, quad_pos[2]     , block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos[0] + 1, quad_pos[1] + 1, quad_pos[2]     , block, ao[0], ao[1], ao[2], ao[3] });
                }
            }

    std::vector<uint8_t> tmp_mesh;
    tmp_mesh.reserve(mesh.size() * 3);
    for (const auto & v : mesh) {
        tmp_mesh.push_back(v.x);
        tmp_mesh.push_back(v.y);
        tmp_mesh.push_back(v.z);
    }
    return std::move(tmp_mesh);
}
