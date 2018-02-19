#include "mesher.hpp"
#include "Math.hpp"
#include <glm/glm.hpp>

template <>
void mesher::mesh<mesher::MesherType::STANDARD>(
    std::vector<cfg::Vertex> & out_mesh,
    const std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> & chunks
) {
    /*
    // TODO: implement aka copy paste existing implementation
    mesh.resize(0);
    mesh.push_back({});
    for (size_t i = 0; i < 8; ++i)
        mesh[0].vals[i] = 0;
*/

    static constexpr glm::tvec3<cfg::Coord> fr{ cfg::MESH_OFFSET };
    const glm::tvec3<cfg::Coord> to{ fr + cfg::MESH_SIZE };



/*
    const glm::ivec3 from_block{ chunk_position * CHUNK_SIZES };
    const glm::ivec3 to_block{ from_block + CHUNK_SIZES };
    const uint8_t * blockss[27];
    size_t k = 0;
    for (int z = chunk_position.z - 1; z <= chunk_position.z + 1; ++z)
        for (int y = chunk_position.y - 1; y <= chunk_position.y + 1; ++y)
            for (int x = chunk_position.x - 1; x <= chunk_position.x + 1; ++x) {
                VoxelMap::ChunkPtr p = vs.get(x, y, z, true, false);
                if (p.n) {
                    std::cout << "new" << std::endl;
                    create_new_chunk(p.b, { x, y, z });
                }
                blockss[k++] = p.b;
            }
*/

    const auto block_get = [&chunks] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        const auto chunk_position = Math::floor_div(p, cfg::CHUNK_SIZE);
        const auto block_index = Math::position_to_index(p, cfg::CHUNK_SIZE);
        const auto chunk_index = Math::position_to_index(chunk_position, cfg::MESH_CHUNK_SIZE);
        return chunks[chunk_index][block_index];
    };

            /*
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
*/
    struct Vertex {
        uint8_t x, y, z, c, a0, a1, a2, a3;
    };

    constexpr std::uint8_t SHADOW_STRENGTH{ 60 };

    std::vector<Vertex> mesh;
    mesh.reserve(1024*1024); // whatever

    glm::tvec3<cfg::Coord> i;
    static constexpr glm::tvec3<cfg::Coord> offset{ cfg::MESH_OFFSET };
    for (i.z = fr.z; i.z < to.z; ++i.z)
        for (i.y = fr.y; i.y < to.y; ++i.y)
            for (i.x = fr.x; i.x < to.x; ++i.x)
            {
                auto block = block_get(i); // TODO: this can in future probably be set to reference
                if (block == 0) continue;

                block = std::rand() % 254 + 1;

                // X + 1
                if (block_get({ i.x + 1, i.y, i.z }) == 0)
                {
                    const bool aos[8]{
                        block_get({ i.x + 1, i.y - 1, i.z     }) != 0,
                        block_get({ i.x + 1, i.y    , i.z - 1 }) != 0,
                        block_get({ i.x + 1, i.y + 1, i.z     }) != 0,
                        block_get({ i.x + 1, i.y    , i.z + 1 }) != 0,

                        block_get({ i.x + 1, i.y - 1, i.z - 1 }) != 0,
                        block_get({ i.x + 1, i.y - 1, i.z + 1 }) != 0,
                        block_get({ i.x + 1, i.y + 1, i.z + 1 }) != 0,
                        block_get({ i.x + 1, i.y + 1, i.z - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[3], aos[6]),
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[2], aos[1], aos[7])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - offset;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(Vertex{ quad_po1.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // X - 1
                if (block_get({ i.x - 1, i.y, i.z }) == 0)
                {
                    const bool aos[8]{
                        block_get({ i.x - 1, i.y - 1, i.z     }) != 0,
                        block_get({ i.x - 1, i.y    , i.z - 1 }) != 0,
                        block_get({ i.x - 1, i.y + 1, i.z     }) != 0,
                        block_get({ i.x - 1, i.y    , i.z + 1 }) != 0,

                        block_get({ i.x - 1, i.y - 1, i.z - 1 }) != 0,
                        block_get({ i.x - 1, i.y - 1, i.z + 1 }) != 0,
                        block_get({ i.x - 1, i.y + 1, i.z + 1 }) != 0,
                        block_get({ i.x - 1, i.y + 1, i.z - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[3], aos[6])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - offset;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(Vertex{ quad_pos.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Y + 1
                if (block_get({ i.x, i.y + 1, i.z }) == 0)
                {
                    const bool aos[8]{
                        block_get({ i.x - 1, i.y + 1, i.z     }) != 0,
                        block_get({ i.x    , i.y + 1, i.z - 1 }) != 0,
                        block_get({ i.x + 1, i.y + 1, i.z     }) != 0,
                        block_get({ i.x    , i.y + 1, i.z + 1 }) != 0,

                        block_get({ i.x - 1, i.y + 1, i.z - 1 }) != 0,
                        block_get({ i.x - 1, i.y + 1, i.z + 1 }) != 0,
                        block_get({ i.x + 1, i.y + 1, i.z + 1 }) != 0,
                        block_get({ i.x + 1, i.y + 1, i.z - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6]),
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - offset;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(Vertex{ quad_po1.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Y - 1
                if (block_get({ i.x, i.y - 1, i.z }) == 0)
                {
                    const bool aos[8]{
                        block_get({ i.x - 1, i.y - 1, i.z     }) != 0,
                        block_get({ i.x    , i.y - 1, i.z - 1 }) != 0,
                        block_get({ i.x + 1, i.y - 1, i.z     }) != 0,
                        block_get({ i.x    , i.y - 1, i.z + 1 }) != 0,

                        block_get({ i.x - 1, i.y - 1, i.z - 1 }) != 0,
                        block_get({ i.x - 1, i.y - 1, i.z + 1 }) != 0,
                        block_get({ i.x + 1, i.y - 1, i.z + 1 }) != 0,
                        block_get({ i.x + 1, i.y - 1, i.z - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - offset;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(Vertex{ quad_pos.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Z + 1
                if (block_get({ i.x, i.y, i.z + 1 }) == 0)
                {
                    const bool aos[8]{
                        block_get({ i.x - 1, i.y    , i.z + 1 }) != 0,
                        block_get({ i.x    , i.y - 1, i.z + 1 }) != 0,
                        block_get({ i.x + 1, i.y    , i.z + 1 }) != 0,
                        block_get({ i.x    , i.y + 1, i.z + 1 }) != 0,

                        block_get({ i.x - 1, i.y - 1, i.z + 1 }) != 0,
                        block_get({ i.x - 1, i.y + 1, i.z + 1 }) != 0,
                        block_get({ i.x + 1, i.y + 1, i.z + 1 }) != 0,
                        block_get({ i.x + 1, i.y - 1, i.z + 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - offset;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(Vertex{ quad_pos.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Z - 1
                if (block_get({ i.x, i.y, i.z - 1 }) == 0)
                {
                    const bool aos[8]{
                        block_get({ i.x - 1, i.y    , i.z - 1 }) != 0,
                        block_get({ i.x    , i.y - 1, i.z - 1 }) != 0,
                        block_get({ i.x + 1, i.y    , i.z - 1 }) != 0,
                        block_get({ i.x    , i.y + 1, i.z - 1 }) != 0,

                        block_get({ i.x - 1, i.y - 1, i.z - 1 }) != 0,
                        block_get({ i.x - 1, i.y + 1, i.z - 1 }) != 0,
                        block_get({ i.x + 1, i.y + 1, i.z - 1 }) != 0,
                        block_get({ i.x + 1, i.y - 1, i.z - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6]),
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - offset;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    uint8_t tt = uint8_t(quad_pos.y) + uint8_t(1);

                    mesh.push_back(Vertex{ quad_po1.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_pos.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(Vertex{ quad_po1.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                }
            }

    out_mesh.clear();
    out_mesh.reserve(mesh.size());
    for (const auto & v : mesh) {
        cfg::Vertex u;
        u.vals[0] = v.x;
        u.vals[1] = v.y;
        u.vals[2] = v.z;
        u.vals[3] = v.c;
        u.vals[4] = v.a0;
        u.vals[5] = v.a1;
        u.vals[6] = v.a2;
        u.vals[7] = v.a3;
        out_mesh.push_back(u);
    }
}
