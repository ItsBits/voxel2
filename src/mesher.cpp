#include "mesher.hpp"
#include "Math.hpp"
#include <glm/glm.hpp>
#include "Print.hpp"

template <>
void mesher::mesh<mesher::MesherType::INDEX_LOOKUP_TABLE_UNROLL_SEMI_NO_LAMBDA>(
    std::vector<cfg::Vertex> & mesh,
    const std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> & chunks
) {
    constexpr std::uint8_t SHADOW_STRENGTH{ 60 };

    static constexpr glm::tvec3<cfg::Coord> DIM{ Math::add(cfg::MESH_SIZE, 2) };
    static constexpr glm::tvec3<cfg::Coord> FR{ cfg::MESH_OFFSET };
    static constexpr glm::tvec3<cfg::Coord> TO{ Math::add(FR, cfg::MESH_SIZE) };
    static constexpr glm::tvec3<cfg::Coord> OFFSET{ 1, 1, 1 };

    static constexpr std::array<int32_t, 6> NEIGHBOUR_OFFSETS{ {
        Math::to_index({ -1,  0,  0 }, DIM), Math::to_index({  1,  0,  0 }, DIM),
        Math::to_index({  0, -1,  0 }, DIM), Math::to_index({  0,  1,  0 }, DIM),
        Math::to_index({  0,  0, -1 }, DIM), Math::to_index({  0,  0,  1 }, DIM),
    } };

    static constexpr std::array<std::array<glm::tvec3<uint8_t>, 4>, 6> QUAD_VERTEX_OFFSETS{ {
        { { { 0, 0, 0 }, { 0, 0, 1 }, { 0, 1, 1 }, { 0, 1, 0 } } },
        { { { 1, 0, 0 }, { 1, 1, 0 }, { 1, 1, 1 }, { 1, 0, 1 } } },

        { { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 0, 1 }, { 0, 0, 1 } } },
        { { { 0, 1, 0 }, { 0, 1, 1 }, { 1, 1, 1 }, { 1, 1, 0 } } },

        { { { 0, 0, 0 }, { 0, 1, 0 }, { 1, 1, 0 }, { 1, 0, 0 } } },
        { { { 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 } } },
    } };

    static constexpr std::array<std::array<int32_t, 8>, 6> AOS_OFFSETS{ {
        { { Math::to_index({ -1, -1, -1 }, DIM), Math::to_index({ -1,  0, -1 }, DIM), Math::to_index({ -1,  1, -1 }, DIM), Math::to_index({ -1, -1,  0 }, DIM), Math::to_index({ -1,  1,  0 }, DIM), Math::to_index({ -1, -1,  1 }, DIM), Math::to_index({ -1,  0,  1 }, DIM), Math::to_index({ -1,  1,  1 }, DIM) } },
        { { Math::to_index({  1, -1, -1 }, DIM), Math::to_index({  1,  0, -1 }, DIM), Math::to_index({  1,  1, -1 }, DIM), Math::to_index({  1, -1,  0 }, DIM), Math::to_index({  1,  1,  0 }, DIM), Math::to_index({  1, -1,  1 }, DIM), Math::to_index({  1,  0,  1 }, DIM), Math::to_index({  1,  1,  1 }, DIM) } },

        { { Math::to_index({ -1, -1, -1 }, DIM), Math::to_index({  0, -1, -1 }, DIM), Math::to_index({  1, -1, -1 }, DIM), Math::to_index({ -1, -1,  0 }, DIM), Math::to_index({  1, -1,  0 }, DIM), Math::to_index({ -1, -1,  1 }, DIM), Math::to_index({  0, -1,  1 }, DIM), Math::to_index({  1, -1,  1 }, DIM) } },
        { { Math::to_index({ -1,  1, -1 }, DIM), Math::to_index({  0,  1, -1 }, DIM), Math::to_index({  1,  1, -1 }, DIM), Math::to_index({ -1,  1,  0 }, DIM), Math::to_index({  1,  1,  0 }, DIM), Math::to_index({ -1,  1,  1 }, DIM), Math::to_index({  0,  1,  1 }, DIM), Math::to_index({  1,  1,  1 }, DIM) } },

        { { Math::to_index({ -1, -1, -1 }, DIM), Math::to_index({  0, -1, -1 }, DIM), Math::to_index({  1, -1, -1 }, DIM), Math::to_index({ -1,  0, -1 }, DIM), Math::to_index({  1,  0, -1 }, DIM), Math::to_index({ -1,  1, -1 }, DIM), Math::to_index({  0,  1, -1 }, DIM), Math::to_index({  1,  1, -1 }, DIM) } },
        { { Math::to_index({ -1, -1,  1 }, DIM), Math::to_index({  0, -1,  1 }, DIM), Math::to_index({  1, -1,  1 }, DIM), Math::to_index({ -1,  0,  1 }, DIM), Math::to_index({  1,  0,  1 }, DIM), Math::to_index({ -1,  1,  1 }, DIM), Math::to_index({  0,  1,  1 }, DIM), Math::to_index({  1,  1,  1 }, DIM) } },
    } };

    static constexpr std::array<std::array<glm::tvec3<uint8_t>, 4>, 6> AO_OFFSETS{ {
        { { { 1, 3, 0 }, { 1, 4, 2 }, { 3, 6, 5 }, { 4, 6, 7 } } },
        { { { 1, 3, 0 }, { 3, 6, 5 }, { 1, 4, 2 }, { 4, 6, 7 } } },

        { { { 1, 3, 0 }, { 3, 6, 5 }, { 1, 4, 2 }, { 4, 6, 7 } } },
        { { { 1, 3, 0 }, { 1, 4, 2 }, { 3, 6, 5 }, { 4, 6, 7 } } },

        { { { 1, 3, 0 }, { 1, 4, 2 }, { 3, 6, 5 }, { 4, 6, 7 } } },
        { { { 1, 3, 0 }, { 3, 6, 5 }, { 1, 4, 2 }, { 4, 6, 7 } } },
    } };

    mesh.clear();
    mesh.reserve(1024 * 1024); // whatever
    
    std::vector<cfg::Block> chunk;
    chunk.reserve(Math::volume(DIM));

    glm::tvec3<cfg::Coord> i;
    for (i.z = FR.z - 1; i.z < TO.z + 1; ++i.z)
        for (i.y = FR.y - 1; i.y < TO.y + 1; ++i.y)
            for (i.x = FR.x - 1; i.x < TO.x + 1; ++i.x) {
                const auto chunk_position = Math::floor_div(i, cfg::CHUNK_SIZE);
                const auto block_index = Math::position_to_index(i, cfg::CHUNK_SIZE);
                const auto chunk_index = Math::position_to_index(chunk_position, cfg::MESH_CHUNK_SIZE);
                chunk.push_back(chunks[chunk_index][block_index]);
            }

    int32_t block_index = DIM.y * DIM.x + DIM.x + 1;
    for (i.z = 1; i.z < cfg::MESH_SIZE.z + 1; ++i.z)
        for (i.y = 1; i.y < cfg::MESH_SIZE.y + 1; ++i.y)
            for (i.x = 1; i.x < cfg::MESH_SIZE.x + 1; ++i.x) {
                const int32_t block_index = Math::to_index(i, DIM);
                const auto block = chunk[block_index];
                if (block == cfg::Block{ 0 })
                    continue;

                // trick compiler into unrolling the loop AND inlining constexpr values
                #define PROCESS_SIDE(SIDE_INDEX)                                                                                                                                                                                                                                        \
                    if (chunk[block_index + NEIGHBOUR_OFFSETS[SIDE_INDEX]] == cfg::Block{ 0 }) {                                                                                                                                                                                        \
                        std::array<bool, 8> aos;                                                                                                                                                                                                                                        \
                        aos[0] = chunk[block_index + AOS_OFFSETS[SIDE_INDEX][0]] != cfg::Block{ 0 };                                                                                                                                                                                    \
                        aos[1] = chunk[block_index + AOS_OFFSETS[SIDE_INDEX][1]] != cfg::Block{ 0 };                                                                                                                                                                                    \
                        aos[2] = chunk[block_index + AOS_OFFSETS[SIDE_INDEX][2]] != cfg::Block{ 0 };                                                                                                                                                                                    \
                        aos[3] = chunk[block_index + AOS_OFFSETS[SIDE_INDEX][3]] != cfg::Block{ 0 };                                                                                                                                                                                    \
                        aos[4] = chunk[block_index + AOS_OFFSETS[SIDE_INDEX][4]] != cfg::Block{ 0 };                                                                                                                                                                                    \
                        aos[5] = chunk[block_index + AOS_OFFSETS[SIDE_INDEX][5]] != cfg::Block{ 0 };                                                                                                                                                                                    \
                        aos[6] = chunk[block_index + AOS_OFFSETS[SIDE_INDEX][6]] != cfg::Block{ 0 };                                                                                                                                                                                    \
                        aos[7] = chunk[block_index + AOS_OFFSETS[SIDE_INDEX][7]] != cfg::Block{ 0 };                                                                                                                                                                                    \
                        std::array<uint8_t, 4> ao;                                                                                                                                                                                                                                      \
                        ao[0] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[SIDE_INDEX][0][0]], aos[AO_OFFSETS[SIDE_INDEX][0][1]], aos[AO_OFFSETS[SIDE_INDEX][0][2]]);                                                                   \
                        ao[1] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[SIDE_INDEX][1][0]], aos[AO_OFFSETS[SIDE_INDEX][1][1]], aos[AO_OFFSETS[SIDE_INDEX][1][2]]);                                                                   \
                        ao[2] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[SIDE_INDEX][2][0]], aos[AO_OFFSETS[SIDE_INDEX][2][1]], aos[AO_OFFSETS[SIDE_INDEX][2][2]]);                                                                   \
                        ao[3] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[SIDE_INDEX][3][0]], aos[AO_OFFSETS[SIDE_INDEX][3][1]], aos[AO_OFFSETS[SIDE_INDEX][3][2]]);                                                                   \
                        const auto vertex_position = glm::tvec3<uint8_t>{ i - OFFSET };                                                                                                                                                                                                 \
                        mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[SIDE_INDEX][0].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[SIDE_INDEX][0].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[SIDE_INDEX][0].z), block, ao[0], ao[1], ao[2], ao[3] }); \
                        mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[SIDE_INDEX][1].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[SIDE_INDEX][1].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[SIDE_INDEX][1].z), block, ao[0], ao[1], ao[2], ao[3] }); \
                        mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[SIDE_INDEX][2].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[SIDE_INDEX][2].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[SIDE_INDEX][2].z), block, ao[0], ao[1], ao[2], ao[3] }); \
                        mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[SIDE_INDEX][3].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[SIDE_INDEX][3].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[SIDE_INDEX][3].z), block, ao[0], ao[1], ao[2], ao[3] }); \
                    }
                    PROCESS_SIDE(0)
                    PROCESS_SIDE(1)
                    PROCESS_SIDE(2)
                    PROCESS_SIDE(3)
                    PROCESS_SIDE(4)
                    PROCESS_SIDE(5)
                #undef PROCESS_SIDE
            }
}

template <>
void mesher::mesh<mesher::MesherType::INDEX_LOOKUP_TABLE_UNROLL_SEMI>(
    std::vector<cfg::Vertex> & mesh,
    const std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> & chunks
) {
    constexpr std::uint8_t SHADOW_STRENGTH{ 60 };
    mesh.clear();
    mesh.reserve(1024 * 1024); // whatever

    const auto block_get = [&chunks] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        const auto chunk_position = Math::floor_div(p, cfg::CHUNK_SIZE);
        const auto block_index = Math::position_to_index(p, cfg::CHUNK_SIZE);
        const auto chunk_index = Math::position_to_index(chunk_position, cfg::MESH_CHUNK_SIZE);
        return chunks[chunk_index][block_index];
    };

    static constexpr glm::tvec3<cfg::Coord> DIM{ Math::add(cfg::MESH_SIZE, 2) };
    static constexpr glm::tvec3<cfg::Coord> FR{ cfg::MESH_OFFSET };
    static constexpr glm::tvec3<cfg::Coord> TO{ Math::add(FR, cfg::MESH_SIZE) };
    static constexpr glm::tvec3<cfg::Coord> OFFSET{ 1, 1, 1 };

    std::vector<cfg::Block> chunk;
    chunk.reserve(Math::volume(DIM));

    glm::tvec3<cfg::Coord> i;
    for (i.z = FR.z - 1; i.z < TO.z + 1; ++i.z)
        for (i.y = FR.y - 1; i.y < TO.y + 1; ++i.y)
            for (i.x = FR.x - 1; i.x < TO.x + 1; ++i.x)
                chunk.push_back(block_get(i));

    const auto block_get_2 = [&chunk] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        const auto block_index = Math::to_index(p, DIM);
        return chunk[block_index];
    };

    const auto block_get_3 = [&chunk] (int32_t i) -> cfg::Block & {
        return chunk[i];
    };

    static constexpr std::array<int32_t, 6> NEIGHBOUR_OFFSETS{ {
        Math::to_index({ -1,  0,  0 }, DIM), Math::to_index({  1,  0,  0 }, DIM),
        Math::to_index({  0, -1,  0 }, DIM), Math::to_index({  0,  1,  0 }, DIM),
        Math::to_index({  0,  0, -1 }, DIM), Math::to_index({  0,  0,  1 }, DIM),
    } };

    static constexpr std::array<std::array<glm::tvec3<uint8_t>, 4>, 6> QUAD_VERTEX_OFFSETS{ {
        { { { 0, 0, 0 }, { 0, 0, 1 }, { 0, 1, 1 }, { 0, 1, 0 } } },
        { { { 1, 0, 0 }, { 1, 1, 0 }, { 1, 1, 1 }, { 1, 0, 1 } } },

        { { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 0, 1 }, { 0, 0, 1 } } },
        { { { 0, 1, 0 }, { 0, 1, 1 }, { 1, 1, 1 }, { 1, 1, 0 } } },

        { { { 0, 0, 0 }, { 0, 1, 0 }, { 1, 1, 0 }, { 1, 0, 0 } } },
        { { { 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 } } },
    } };

    static constexpr std::array<std::array<int32_t, 8>, 6> AOS_OFFSETS{ {
        { { Math::to_index({ -1, -1, -1 }, DIM), Math::to_index({ -1,  0, -1 }, DIM), Math::to_index({ -1,  1, -1 }, DIM), Math::to_index({ -1, -1,  0 }, DIM), Math::to_index({ -1,  1,  0 }, DIM), Math::to_index({ -1, -1,  1 }, DIM), Math::to_index({ -1,  0,  1 }, DIM), Math::to_index({ -1,  1,  1 }, DIM) } },
        { { Math::to_index({  1, -1, -1 }, DIM), Math::to_index({  1,  0, -1 }, DIM), Math::to_index({  1,  1, -1 }, DIM), Math::to_index({  1, -1,  0 }, DIM), Math::to_index({  1,  1,  0 }, DIM), Math::to_index({  1, -1,  1 }, DIM), Math::to_index({  1,  0,  1 }, DIM), Math::to_index({  1,  1,  1 }, DIM) } },

        { { Math::to_index({ -1, -1, -1 }, DIM), Math::to_index({  0, -1, -1 }, DIM), Math::to_index({  1, -1, -1 }, DIM), Math::to_index({ -1, -1,  0 }, DIM), Math::to_index({  1, -1,  0 }, DIM), Math::to_index({ -1, -1,  1 }, DIM), Math::to_index({  0, -1,  1 }, DIM), Math::to_index({  1, -1,  1 }, DIM) } },
        { { Math::to_index({ -1,  1, -1 }, DIM), Math::to_index({  0,  1, -1 }, DIM), Math::to_index({  1,  1, -1 }, DIM), Math::to_index({ -1,  1,  0 }, DIM), Math::to_index({  1,  1,  0 }, DIM), Math::to_index({ -1,  1,  1 }, DIM), Math::to_index({  0,  1,  1 }, DIM), Math::to_index({  1,  1,  1 }, DIM) } },

        { { Math::to_index({ -1, -1, -1 }, DIM), Math::to_index({  0, -1, -1 }, DIM), Math::to_index({  1, -1, -1 }, DIM), Math::to_index({ -1,  0, -1 }, DIM), Math::to_index({  1,  0, -1 }, DIM), Math::to_index({ -1,  1, -1 }, DIM), Math::to_index({  0,  1, -1 }, DIM), Math::to_index({  1,  1, -1 }, DIM) } },
        { { Math::to_index({ -1, -1,  1 }, DIM), Math::to_index({  0, -1,  1 }, DIM), Math::to_index({  1, -1,  1 }, DIM), Math::to_index({ -1,  0,  1 }, DIM), Math::to_index({  1,  0,  1 }, DIM), Math::to_index({ -1,  1,  1 }, DIM), Math::to_index({  0,  1,  1 }, DIM), Math::to_index({  1,  1,  1 }, DIM) } },
    } };

    static constexpr std::array<std::array<glm::tvec3<uint8_t>, 4>, 6> AO_OFFSETS{ {
        { { { 1, 3, 0 }, { 1, 4, 2 }, { 3, 6, 5 }, { 4, 6, 7 } } },
        { { { 1, 3, 0 }, { 3, 6, 5 }, { 1, 4, 2 }, { 4, 6, 7 } } },

        { { { 1, 3, 0 }, { 3, 6, 5 }, { 1, 4, 2 }, { 4, 6, 7 } } },
        { { { 1, 3, 0 }, { 1, 4, 2 }, { 3, 6, 5 }, { 4, 6, 7 } } },

        { { { 1, 3, 0 }, { 1, 4, 2 }, { 3, 6, 5 }, { 4, 6, 7 } } },
        { { { 1, 3, 0 }, { 3, 6, 5 }, { 1, 4, 2 }, { 4, 6, 7 } } },
    } };

    int32_t block_index = DIM.y * DIM.x + DIM.x + 1;
    for (i.z = 1; i.z < cfg::MESH_SIZE.z + 1; ++i.z)
        for (i.y = 1; i.y < cfg::MESH_SIZE.y + 1; ++i.y)
            for (i.x = 1; i.x < cfg::MESH_SIZE.x + 1; ++i.x) {
                const int32_t block_index = Math::to_index(i, DIM);
                const auto block = block_get_3(block_index);
                if (block == cfg::Block{ 0 })
                    continue;

            // trick compiler into unrolling the loop AND inlining constexpr values
            #define PROCESS_SIDE(SIDE_INDEX)                                                                                                                                                                                                                                        \
                if (block_get_3(block_index + NEIGHBOUR_OFFSETS[SIDE_INDEX]) == cfg::Block{ 0 }) {                                                                                                                                                                                  \
                    std::array<bool, 8> aos;                                                                                                                                                                                                                                        \
                    aos[0] = block_get_3(block_index + AOS_OFFSETS[SIDE_INDEX][0]) != cfg::Block{ 0 };                                                                                                                                                                              \
                    aos[1] = block_get_3(block_index + AOS_OFFSETS[SIDE_INDEX][1]) != cfg::Block{ 0 };                                                                                                                                                                              \
                    aos[2] = block_get_3(block_index + AOS_OFFSETS[SIDE_INDEX][2]) != cfg::Block{ 0 };                                                                                                                                                                              \
                    aos[3] = block_get_3(block_index + AOS_OFFSETS[SIDE_INDEX][3]) != cfg::Block{ 0 };                                                                                                                                                                              \
                    aos[4] = block_get_3(block_index + AOS_OFFSETS[SIDE_INDEX][4]) != cfg::Block{ 0 };                                                                                                                                                                              \
                    aos[5] = block_get_3(block_index + AOS_OFFSETS[SIDE_INDEX][5]) != cfg::Block{ 0 };                                                                                                                                                                              \
                    aos[6] = block_get_3(block_index + AOS_OFFSETS[SIDE_INDEX][6]) != cfg::Block{ 0 };                                                                                                                                                                              \
                    aos[7] = block_get_3(block_index + AOS_OFFSETS[SIDE_INDEX][7]) != cfg::Block{ 0 };                                                                                                                                                                              \
                    std::array<uint8_t, 4> ao;                                                                                                                                                                                                                                      \
                    ao[0] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[SIDE_INDEX][0][0]], aos[AO_OFFSETS[SIDE_INDEX][0][1]], aos[AO_OFFSETS[SIDE_INDEX][0][2]]);                                                                   \
                    ao[1] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[SIDE_INDEX][1][0]], aos[AO_OFFSETS[SIDE_INDEX][1][1]], aos[AO_OFFSETS[SIDE_INDEX][1][2]]);                                                                   \
                    ao[2] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[SIDE_INDEX][2][0]], aos[AO_OFFSETS[SIDE_INDEX][2][1]], aos[AO_OFFSETS[SIDE_INDEX][2][2]]);                                                                   \
                    ao[3] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[SIDE_INDEX][3][0]], aos[AO_OFFSETS[SIDE_INDEX][3][1]], aos[AO_OFFSETS[SIDE_INDEX][3][2]]);                                                                   \
                    const auto vertex_position = glm::tvec3<uint8_t>{ i - OFFSET };                                                                                                                                                                                                 \
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[SIDE_INDEX][0].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[SIDE_INDEX][0].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[SIDE_INDEX][0].z), block, ao[0], ao[1], ao[2], ao[3] }); \
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[SIDE_INDEX][1].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[SIDE_INDEX][1].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[SIDE_INDEX][1].z), block, ao[0], ao[1], ao[2], ao[3] }); \
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[SIDE_INDEX][2].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[SIDE_INDEX][2].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[SIDE_INDEX][2].z), block, ao[0], ao[1], ao[2], ao[3] }); \
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[SIDE_INDEX][3].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[SIDE_INDEX][3].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[SIDE_INDEX][3].z), block, ao[0], ao[1], ao[2], ao[3] }); \
                }
                PROCESS_SIDE(0)
                PROCESS_SIDE(1)
                PROCESS_SIDE(2)
                PROCESS_SIDE(3)
                PROCESS_SIDE(4)
                PROCESS_SIDE(5)
            #undef PROCESS_SIDE
            }
}

template <>
void mesher::mesh<mesher::MesherType::INDEX_LOOKUP_TABLE_UNROLL>(
    std::vector<cfg::Vertex> & mesh,
    const std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> & chunks
) {
    constexpr std::uint8_t SHADOW_STRENGTH{ 60 };
    mesh.clear();
    mesh.reserve(1024 * 1024); // whatever

    const auto block_get = [&chunks] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        const auto chunk_position = Math::floor_div(p, cfg::CHUNK_SIZE);
        const auto block_index = Math::position_to_index(p, cfg::CHUNK_SIZE);
        const auto chunk_index = Math::position_to_index(chunk_position, cfg::MESH_CHUNK_SIZE);
        return chunks[chunk_index][block_index];
    };

    static constexpr glm::tvec3<cfg::Coord> DIM{ Math::add(cfg::MESH_SIZE, 2) };
    static constexpr glm::tvec3<cfg::Coord> FR{ cfg::MESH_OFFSET };
    static constexpr glm::tvec3<cfg::Coord> TO{ Math::add(FR, cfg::MESH_SIZE) };
    static constexpr glm::tvec3<cfg::Coord> OFFSET{ 1, 1, 1 };

    std::vector<cfg::Block> chunk;
    chunk.reserve(Math::volume(DIM));

    glm::tvec3<cfg::Coord> i;
    for (i.z = FR.z - 1; i.z < TO.z + 1; ++i.z)
        for (i.y = FR.y - 1; i.y < TO.y + 1; ++i.y)
            for (i.x = FR.x - 1; i.x < TO.x + 1; ++i.x)
                chunk.push_back(block_get(i));

    const auto block_get_2 = [&chunk] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        const auto block_index = Math::to_index(p, DIM);
        return chunk[block_index];
    };

    const auto block_get_3 = [&chunk] (int32_t i) -> cfg::Block & {
        return chunk[i];
    };

    static constexpr std::array<int32_t, 6> NEIGHBOUR_OFFSETS{ {
        Math::to_index({ -1,  0,  0 }, DIM), Math::to_index({  1,  0,  0 }, DIM),
        Math::to_index({  0, -1,  0 }, DIM), Math::to_index({  0,  1,  0 }, DIM),
        Math::to_index({  0,  0, -1 }, DIM), Math::to_index({  0,  0,  1 }, DIM),
    } };

    static constexpr std::array<std::array<glm::tvec3<uint8_t>, 4>, 6> QUAD_VERTEX_OFFSETS{ {
        { { { 0, 0, 0 }, { 0, 0, 1 }, { 0, 1, 1 }, { 0, 1, 0 } } },
        { { { 1, 0, 0 }, { 1, 1, 0 }, { 1, 1, 1 }, { 1, 0, 1 } } },

        { { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 0, 1 }, { 0, 0, 1 } } },
        { { { 0, 1, 0 }, { 0, 1, 1 }, { 1, 1, 1 }, { 1, 1, 0 } } },

        { { { 0, 0, 0 }, { 0, 1, 0 }, { 1, 1, 0 }, { 1, 0, 0 } } },
        { { { 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 } } },
    } };

    static constexpr std::array<std::array<int32_t, 8>, 6> AOS_OFFSETS{ {
        { { Math::to_index({ -1, -1, -1 }, DIM), Math::to_index({ -1,  0, -1 }, DIM), Math::to_index({ -1,  1, -1 }, DIM), Math::to_index({ -1, -1,  0 }, DIM), Math::to_index({ -1,  1,  0 }, DIM), Math::to_index({ -1, -1,  1 }, DIM), Math::to_index({ -1,  0,  1 }, DIM), Math::to_index({ -1,  1,  1 }, DIM) } },
        { { Math::to_index({  1, -1, -1 }, DIM), Math::to_index({  1,  0, -1 }, DIM), Math::to_index({  1,  1, -1 }, DIM), Math::to_index({  1, -1,  0 }, DIM), Math::to_index({  1,  1,  0 }, DIM), Math::to_index({  1, -1,  1 }, DIM), Math::to_index({  1,  0,  1 }, DIM), Math::to_index({  1,  1,  1 }, DIM) } },

        { { Math::to_index({ -1, -1, -1 }, DIM), Math::to_index({  0, -1, -1 }, DIM), Math::to_index({  1, -1, -1 }, DIM), Math::to_index({ -1, -1,  0 }, DIM), Math::to_index({  1, -1,  0 }, DIM), Math::to_index({ -1, -1,  1 }, DIM), Math::to_index({  0, -1,  1 }, DIM), Math::to_index({  1, -1,  1 }, DIM) } },
        { { Math::to_index({ -1,  1, -1 }, DIM), Math::to_index({  0,  1, -1 }, DIM), Math::to_index({  1,  1, -1 }, DIM), Math::to_index({ -1,  1,  0 }, DIM), Math::to_index({  1,  1,  0 }, DIM), Math::to_index({ -1,  1,  1 }, DIM), Math::to_index({  0,  1,  1 }, DIM), Math::to_index({  1,  1,  1 }, DIM) } },

        { { Math::to_index({ -1, -1, -1 }, DIM), Math::to_index({  0, -1, -1 }, DIM), Math::to_index({  1, -1, -1 }, DIM), Math::to_index({ -1,  0, -1 }, DIM), Math::to_index({  1,  0, -1 }, DIM), Math::to_index({ -1,  1, -1 }, DIM), Math::to_index({  0,  1, -1 }, DIM), Math::to_index({  1,  1, -1 }, DIM) } },
        { { Math::to_index({ -1, -1,  1 }, DIM), Math::to_index({  0, -1,  1 }, DIM), Math::to_index({  1, -1,  1 }, DIM), Math::to_index({ -1,  0,  1 }, DIM), Math::to_index({  1,  0,  1 }, DIM), Math::to_index({ -1,  1,  1 }, DIM), Math::to_index({  0,  1,  1 }, DIM), Math::to_index({  1,  1,  1 }, DIM) } },
    } };

    static constexpr std::array<std::array<glm::tvec3<uint8_t>, 4>, 6> AO_OFFSETS{ {
        { { { 1, 3, 0 }, { 1, 4, 2 }, { 3, 6, 5 }, { 4, 6, 7 } } },
        { { { 1, 3, 0 }, { 3, 6, 5 }, { 1, 4, 2 }, { 4, 6, 7 } } },

        { { { 1, 3, 0 }, { 3, 6, 5 }, { 1, 4, 2 }, { 4, 6, 7 } } },
        { { { 1, 3, 0 }, { 1, 4, 2 }, { 3, 6, 5 }, { 4, 6, 7 } } },

        { { { 1, 3, 0 }, { 1, 4, 2 }, { 3, 6, 5 }, { 4, 6, 7 } } },
        { { { 1, 3, 0 }, { 3, 6, 5 }, { 1, 4, 2 }, { 4, 6, 7 } } },
    } };

    for (i.z = 1; i.z < cfg::MESH_SIZE.z + 1; ++i.z)
        for (i.y = 1; i.y < cfg::MESH_SIZE.y + 1; ++i.y)
            for (i.x = 1; i.x < cfg::MESH_SIZE.x + 1; ++i.x) {
                const int32_t block_index = Math::to_index(i, DIM);
                const auto block = block_get_3(block_index);
                if (block == cfg::Block{ 0 })
                    continue;

                if (block_get_3(block_index + NEIGHBOUR_OFFSETS[0]) == cfg::Block{ 0 }) {
                    std::array<bool, 8> aos;
                    aos[0] = block_get_3(block_index + AOS_OFFSETS[0][0]) != cfg::Block{ 0 };
                    aos[1] = block_get_3(block_index + AOS_OFFSETS[0][1]) != cfg::Block{ 0 };
                    aos[2] = block_get_3(block_index + AOS_OFFSETS[0][2]) != cfg::Block{ 0 };
                    aos[3] = block_get_3(block_index + AOS_OFFSETS[0][3]) != cfg::Block{ 0 };
                    aos[4] = block_get_3(block_index + AOS_OFFSETS[0][4]) != cfg::Block{ 0 };
                    aos[5] = block_get_3(block_index + AOS_OFFSETS[0][5]) != cfg::Block{ 0 };
                    aos[6] = block_get_3(block_index + AOS_OFFSETS[0][6]) != cfg::Block{ 0 };
                    aos[7] = block_get_3(block_index + AOS_OFFSETS[0][7]) != cfg::Block{ 0 };

                    std::array<uint8_t, 4> ao;
                    ao[0] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[0][0][0]], aos[AO_OFFSETS[0][0][1]], aos[AO_OFFSETS[0][0][2]]);
                    ao[1] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[0][1][0]], aos[AO_OFFSETS[0][1][1]], aos[AO_OFFSETS[0][1][2]]);
                    ao[2] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[0][2][0]], aos[AO_OFFSETS[0][2][1]], aos[AO_OFFSETS[0][2][2]]);
                    ao[3] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[0][3][0]], aos[AO_OFFSETS[0][3][1]], aos[AO_OFFSETS[0][3][2]]);

                    const auto vertex_position = glm::tvec3<uint8_t>{ i - OFFSET };
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[0][0].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[0][0].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[0][0].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[0][1].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[0][1].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[0][1].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[0][2].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[0][2].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[0][2].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[0][3].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[0][3].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[0][3].z), block, ao[0], ao[1], ao[2], ao[3] });
                }
                
                if (block_get_3(block_index + NEIGHBOUR_OFFSETS[1]) == cfg::Block{ 0 }) {
                    std::array<bool, 8> aos;
                    aos[0] = block_get_3(block_index + AOS_OFFSETS[1][0]) != cfg::Block{ 0 };
                    aos[1] = block_get_3(block_index + AOS_OFFSETS[1][1]) != cfg::Block{ 0 };
                    aos[2] = block_get_3(block_index + AOS_OFFSETS[1][2]) != cfg::Block{ 0 };
                    aos[3] = block_get_3(block_index + AOS_OFFSETS[1][3]) != cfg::Block{ 0 };
                    aos[4] = block_get_3(block_index + AOS_OFFSETS[1][4]) != cfg::Block{ 0 };
                    aos[5] = block_get_3(block_index + AOS_OFFSETS[1][5]) != cfg::Block{ 0 };
                    aos[6] = block_get_3(block_index + AOS_OFFSETS[1][6]) != cfg::Block{ 0 };
                    aos[7] = block_get_3(block_index + AOS_OFFSETS[1][7]) != cfg::Block{ 0 };

                    std::array<uint8_t, 4> ao;
                    ao[0] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[1][0][0]], aos[AO_OFFSETS[1][0][1]], aos[AO_OFFSETS[1][0][2]]);
                    ao[1] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[1][1][0]], aos[AO_OFFSETS[1][1][1]], aos[AO_OFFSETS[1][1][2]]);
                    ao[2] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[1][2][0]], aos[AO_OFFSETS[1][2][1]], aos[AO_OFFSETS[1][2][2]]);
                    ao[3] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[1][3][0]], aos[AO_OFFSETS[1][3][1]], aos[AO_OFFSETS[1][3][2]]);

                    const auto vertex_position = glm::tvec3<uint8_t>{ i - OFFSET };
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[1][0].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[1][0].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[1][0].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[1][1].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[1][1].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[1][1].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[1][2].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[1][2].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[1][2].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[1][3].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[1][3].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[1][3].z), block, ao[0], ao[1], ao[2], ao[3] });
                }

                if (block_get_3(block_index + NEIGHBOUR_OFFSETS[2]) == cfg::Block{ 0 }) {
                    std::array<bool, 8> aos;
                    aos[0] = block_get_3(block_index + AOS_OFFSETS[2][0]) != cfg::Block{ 0 };
                    aos[1] = block_get_3(block_index + AOS_OFFSETS[2][1]) != cfg::Block{ 0 };
                    aos[2] = block_get_3(block_index + AOS_OFFSETS[2][2]) != cfg::Block{ 0 };
                    aos[3] = block_get_3(block_index + AOS_OFFSETS[2][3]) != cfg::Block{ 0 };
                    aos[4] = block_get_3(block_index + AOS_OFFSETS[2][4]) != cfg::Block{ 0 };
                    aos[5] = block_get_3(block_index + AOS_OFFSETS[2][5]) != cfg::Block{ 0 };
                    aos[6] = block_get_3(block_index + AOS_OFFSETS[2][6]) != cfg::Block{ 0 };
                    aos[7] = block_get_3(block_index + AOS_OFFSETS[2][7]) != cfg::Block{ 0 };

                    std::array<uint8_t, 4> ao;
                    ao[0] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[2][0][0]], aos[AO_OFFSETS[2][0][1]], aos[AO_OFFSETS[2][0][2]]);
                    ao[1] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[2][1][0]], aos[AO_OFFSETS[2][1][1]], aos[AO_OFFSETS[2][1][2]]);
                    ao[2] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[2][2][0]], aos[AO_OFFSETS[2][2][1]], aos[AO_OFFSETS[2][2][2]]);
                    ao[3] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[2][3][0]], aos[AO_OFFSETS[2][3][1]], aos[AO_OFFSETS[2][3][2]]);

                    const auto vertex_position = glm::tvec3<uint8_t>{ i - OFFSET };
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[2][0].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[2][0].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[2][0].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[2][1].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[2][1].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[2][1].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[2][2].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[2][2].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[2][2].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[2][3].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[2][3].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[2][3].z), block, ao[0], ao[1], ao[2], ao[3] });
                }

                if (block_get_3(block_index + NEIGHBOUR_OFFSETS[3]) == cfg::Block{ 0 }) {
                    std::array<bool, 8> aos;
                    aos[0] = block_get_3(block_index + AOS_OFFSETS[3][0]) != cfg::Block{ 0 };
                    aos[1] = block_get_3(block_index + AOS_OFFSETS[3][1]) != cfg::Block{ 0 };
                    aos[2] = block_get_3(block_index + AOS_OFFSETS[3][2]) != cfg::Block{ 0 };
                    aos[3] = block_get_3(block_index + AOS_OFFSETS[3][3]) != cfg::Block{ 0 };
                    aos[4] = block_get_3(block_index + AOS_OFFSETS[3][4]) != cfg::Block{ 0 };
                    aos[5] = block_get_3(block_index + AOS_OFFSETS[3][5]) != cfg::Block{ 0 };
                    aos[6] = block_get_3(block_index + AOS_OFFSETS[3][6]) != cfg::Block{ 0 };
                    aos[7] = block_get_3(block_index + AOS_OFFSETS[3][7]) != cfg::Block{ 0 };

                    std::array<uint8_t, 4> ao;
                    ao[0] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[3][0][0]], aos[AO_OFFSETS[3][0][1]], aos[AO_OFFSETS[3][0][2]]);
                    ao[1] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[3][1][0]], aos[AO_OFFSETS[3][1][1]], aos[AO_OFFSETS[3][1][2]]);
                    ao[2] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[3][2][0]], aos[AO_OFFSETS[3][2][1]], aos[AO_OFFSETS[3][2][2]]);
                    ao[3] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[3][3][0]], aos[AO_OFFSETS[3][3][1]], aos[AO_OFFSETS[3][3][2]]);

                    const auto vertex_position = glm::tvec3<uint8_t>{ i - OFFSET };
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[3][0].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[3][0].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[3][0].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[3][1].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[3][1].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[3][1].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[3][2].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[3][2].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[3][2].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[3][3].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[3][3].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[3][3].z), block, ao[0], ao[1], ao[2], ao[3] });
                }

                if (block_get_3(block_index + NEIGHBOUR_OFFSETS[4]) == cfg::Block{ 0 }) {
                    std::array<bool, 8> aos;
                    aos[0] = block_get_3(block_index + AOS_OFFSETS[4][0]) != cfg::Block{ 0 };
                    aos[1] = block_get_3(block_index + AOS_OFFSETS[4][1]) != cfg::Block{ 0 };
                    aos[2] = block_get_3(block_index + AOS_OFFSETS[4][2]) != cfg::Block{ 0 };
                    aos[3] = block_get_3(block_index + AOS_OFFSETS[4][3]) != cfg::Block{ 0 };
                    aos[4] = block_get_3(block_index + AOS_OFFSETS[4][4]) != cfg::Block{ 0 };
                    aos[5] = block_get_3(block_index + AOS_OFFSETS[4][5]) != cfg::Block{ 0 };
                    aos[6] = block_get_3(block_index + AOS_OFFSETS[4][6]) != cfg::Block{ 0 };
                    aos[7] = block_get_3(block_index + AOS_OFFSETS[4][7]) != cfg::Block{ 0 };

                    std::array<uint8_t, 4> ao;
                    ao[0] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[4][0][0]], aos[AO_OFFSETS[4][0][1]], aos[AO_OFFSETS[4][0][2]]);
                    ao[1] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[4][1][0]], aos[AO_OFFSETS[4][1][1]], aos[AO_OFFSETS[4][1][2]]);
                    ao[2] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[4][2][0]], aos[AO_OFFSETS[4][2][1]], aos[AO_OFFSETS[4][2][2]]);
                    ao[3] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[4][3][0]], aos[AO_OFFSETS[4][3][1]], aos[AO_OFFSETS[4][3][2]]);

                    const auto vertex_position = glm::tvec3<uint8_t>{ i - OFFSET };
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[4][0].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[4][0].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[4][0].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[4][1].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[4][1].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[4][1].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[4][2].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[4][2].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[4][2].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[4][3].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[4][3].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[4][3].z), block, ao[0], ao[1], ao[2], ao[3] });
                }

                if (block_get_3(block_index + NEIGHBOUR_OFFSETS[5]) == cfg::Block{ 0 }) {
                    std::array<bool, 8> aos;
                    aos[0] = block_get_3(block_index + AOS_OFFSETS[5][0]) != cfg::Block{ 0 };
                    aos[1] = block_get_3(block_index + AOS_OFFSETS[5][1]) != cfg::Block{ 0 };
                    aos[2] = block_get_3(block_index + AOS_OFFSETS[5][2]) != cfg::Block{ 0 };
                    aos[3] = block_get_3(block_index + AOS_OFFSETS[5][3]) != cfg::Block{ 0 };
                    aos[4] = block_get_3(block_index + AOS_OFFSETS[5][4]) != cfg::Block{ 0 };
                    aos[5] = block_get_3(block_index + AOS_OFFSETS[5][5]) != cfg::Block{ 0 };
                    aos[6] = block_get_3(block_index + AOS_OFFSETS[5][6]) != cfg::Block{ 0 };
                    aos[7] = block_get_3(block_index + AOS_OFFSETS[5][7]) != cfg::Block{ 0 };

                    std::array<uint8_t, 4> ao;
                    ao[0] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[5][0][0]], aos[AO_OFFSETS[5][0][1]], aos[AO_OFFSETS[5][0][2]]);
                    ao[1] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[5][1][0]], aos[AO_OFFSETS[5][1][1]], aos[AO_OFFSETS[5][1][2]]);
                    ao[2] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[5][2][0]], aos[AO_OFFSETS[5][2][1]], aos[AO_OFFSETS[5][2][2]]);
                    ao[3] = std::numeric_limits<std::uint8_t>::max() - SHADOW_STRENGTH * Math::vertexAO(aos[AO_OFFSETS[5][3][0]], aos[AO_OFFSETS[5][3][1]], aos[AO_OFFSETS[5][3][2]]);

                    const auto vertex_position = glm::tvec3<uint8_t>{ i - OFFSET };
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[5][0].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[5][0].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[5][0].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[5][1].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[5][1].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[5][1].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[5][2].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[5][2].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[5][2].z), block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back({ uint8_t(vertex_position.x + QUAD_VERTEX_OFFSETS[5][3].x), uint8_t(vertex_position.y + QUAD_VERTEX_OFFSETS[5][3].y), uint8_t(vertex_position.z + QUAD_VERTEX_OFFSETS[5][3].z), block, ao[0], ao[1], ao[2], ao[3] });
                }
            }
}

template <>
void mesher::mesh<mesher::MesherType::INDEX_LOOKUP_TABLE>(
    std::vector<cfg::Vertex> & mesh,
    const std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> & chunks
) {
    constexpr std::uint8_t SHADOW_STRENGTH{ 60 };
    mesh.clear();
    mesh.reserve(1024 * 1024); // whatever

    const auto block_get = [&chunks] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        const auto chunk_position = Math::floor_div(p, cfg::CHUNK_SIZE);
        const auto block_index = Math::position_to_index(p, cfg::CHUNK_SIZE);
        const auto chunk_index = Math::position_to_index(chunk_position, cfg::MESH_CHUNK_SIZE);
        return chunks[chunk_index][block_index];
    };

    static constexpr glm::tvec3<cfg::Coord> DIM{ Math::add(cfg::MESH_SIZE, 2) };
    static constexpr glm::tvec3<cfg::Coord> FR{ cfg::MESH_OFFSET };
    static constexpr glm::tvec3<cfg::Coord> TO{ Math::add(FR, cfg::MESH_SIZE) };
    static constexpr glm::tvec3<cfg::Coord> OFFSET{ 1, 1, 1 };

    std::vector<cfg::Block> chunk;
    chunk.reserve(Math::volume(DIM));

    glm::tvec3<cfg::Coord> i;
    for (i.z = FR.z - 1; i.z < TO.z + 1; ++i.z)
        for (i.y = FR.y - 1; i.y < TO.y + 1; ++i.y)
            for (i.x = FR.x - 1; i.x < TO.x + 1; ++i.x)
                chunk.push_back(block_get(i));

    const auto block_get_2 = [&chunk] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        const auto block_index = Math::to_index(p, DIM);
        return chunk[block_index];
    };

    const auto block_get_3 = [&chunk] (int32_t i) -> cfg::Block & {
        return chunk[i];
    };

    static constexpr std::array<int32_t, 6> NEIGHBOUR_OFFSETS{ {
        Math::to_index({ -1,  0,  0 }, DIM), Math::to_index({  1,  0,  0 }, DIM),
        Math::to_index({  0, -1,  0 }, DIM), Math::to_index({  0,  1,  0 }, DIM),
        Math::to_index({  0,  0, -1 }, DIM), Math::to_index({  0,  0,  1 }, DIM),
    } };

    static constexpr std::array<std::array<glm::tvec3<uint8_t>, 4>, 6> QUAD_VERTEX_OFFSETS{ {
        { { { 0, 0, 0 }, { 0, 0, 1 }, { 0, 1, 1 }, { 0, 1, 0 } } },
        { { { 1, 0, 0 }, { 1, 1, 0 }, { 1, 1, 1 }, { 1, 0, 1 } } },

        { { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 0, 1 }, { 0, 0, 1 } } },
        { { { 0, 1, 0 }, { 0, 1, 1 }, { 1, 1, 1 }, { 1, 1, 0 } } },

        { { { 0, 0, 0 }, { 0, 1, 0 }, { 1, 1, 0 }, { 1, 0, 0 } } },
        { { { 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 } } },
    } };

    static constexpr std::array<std::array<int32_t, 8>, 6> AOS_OFFSETS{ {
        { { Math::to_index({ -1, -1, -1 }, DIM), Math::to_index({ -1,  0, -1 }, DIM), Math::to_index({ -1,  1, -1 }, DIM), Math::to_index({ -1, -1,  0 }, DIM), Math::to_index({ -1,  1,  0 }, DIM), Math::to_index({ -1, -1,  1 }, DIM), Math::to_index({ -1,  0,  1 }, DIM), Math::to_index({ -1,  1,  1 }, DIM) } },
        { { Math::to_index({  1, -1, -1 }, DIM), Math::to_index({  1,  0, -1 }, DIM), Math::to_index({  1,  1, -1 }, DIM), Math::to_index({  1, -1,  0 }, DIM), Math::to_index({  1,  1,  0 }, DIM), Math::to_index({  1, -1,  1 }, DIM), Math::to_index({  1,  0,  1 }, DIM), Math::to_index({  1,  1,  1 }, DIM) } },

        { { Math::to_index({ -1, -1, -1 }, DIM), Math::to_index({  0, -1, -1 }, DIM), Math::to_index({  1, -1, -1 }, DIM), Math::to_index({ -1, -1,  0 }, DIM), Math::to_index({  1, -1,  0 }, DIM), Math::to_index({ -1, -1,  1 }, DIM), Math::to_index({  0, -1,  1 }, DIM), Math::to_index({  1, -1,  1 }, DIM) } },
        { { Math::to_index({ -1,  1, -1 }, DIM), Math::to_index({  0,  1, -1 }, DIM), Math::to_index({  1,  1, -1 }, DIM), Math::to_index({ -1,  1,  0 }, DIM), Math::to_index({  1,  1,  0 }, DIM), Math::to_index({ -1,  1,  1 }, DIM), Math::to_index({  0,  1,  1 }, DIM), Math::to_index({  1,  1,  1 }, DIM) } },

        { { Math::to_index({ -1, -1, -1 }, DIM), Math::to_index({  0, -1, -1 }, DIM), Math::to_index({  1, -1, -1 }, DIM), Math::to_index({ -1,  0, -1 }, DIM), Math::to_index({  1,  0, -1 }, DIM), Math::to_index({ -1,  1, -1 }, DIM), Math::to_index({  0,  1, -1 }, DIM), Math::to_index({  1,  1, -1 }, DIM) } },
        { { Math::to_index({ -1, -1,  1 }, DIM), Math::to_index({  0, -1,  1 }, DIM), Math::to_index({  1, -1,  1 }, DIM), Math::to_index({ -1,  0,  1 }, DIM), Math::to_index({  1,  0,  1 }, DIM), Math::to_index({ -1,  1,  1 }, DIM), Math::to_index({  0,  1,  1 }, DIM), Math::to_index({  1,  1,  1 }, DIM) } },
    } };

    static constexpr std::array<std::array<glm::tvec3<uint8_t>, 4>, 6> AO_OFFSETS{ {
        { { { 1, 3, 0 }, { 1, 4, 2 }, { 3, 6, 5 }, { 4, 6, 7 } } },
        { { { 1, 3, 0 }, { 3, 6, 5 }, { 1, 4, 2 }, { 4, 6, 7 } } },

        { { { 1, 3, 0 }, { 3, 6, 5 }, { 1, 4, 2 }, { 4, 6, 7 } } },
        { { { 1, 3, 0 }, { 1, 4, 2 }, { 3, 6, 5 }, { 4, 6, 7 } } },

        { { { 1, 3, 0 }, { 1, 4, 2 }, { 3, 6, 5 }, { 4, 6, 7 } } },
        { { { 1, 3, 0 }, { 3, 6, 5 }, { 1, 4, 2 }, { 4, 6, 7 } } },
    } };

    for (i.z = 1; i.z < cfg::MESH_SIZE.z + 1; ++i.z)
        for (i.y = 1; i.y < cfg::MESH_SIZE.y + 1; ++i.y)
            for (i.x = 1; i.x < cfg::MESH_SIZE.x + 1; ++i.x) {
                const int32_t block_index = Math::to_index(i, DIM);
                const auto block = block_get_3(block_index);
                if (block == cfg::Block{ 0 })
                    continue;

                for (size_t j = 0; j < 6; ++j) {
                    if (block_get_3(block_index + NEIGHBOUR_OFFSETS[j]) != cfg::Block{ 0 }) continue;

                    std::array<bool, 8> aos;
                    for (size_t k = 0; k < 8; ++k)
                        aos[k] = block_get_3(block_index + AOS_OFFSETS[j][k]) != cfg::Block{ 0 };

                    std::array<uint8_t, 4> ao;
                    for (size_t k = 0; k < 4; ++k) {
                        ao[k] = Math::vertexAO(aos[AO_OFFSETS[j][k][0]], aos[AO_OFFSETS[j][k][1]], aos[AO_OFFSETS[j][k][2]]);
                        ao[k] = std::numeric_limits<std::uint8_t>::max() - ao[k] * SHADOW_STRENGTH;
                    }

                    const auto quad_pos = glm::tvec3<uint8_t>{ i - OFFSET };
                    for (size_t k = 0; k < 4; ++k) {
                        const auto vertex_position = quad_pos + QUAD_VERTEX_OFFSETS[j][k];
                        mesh.push_back({ vertex_position.x, vertex_position.y, vertex_position.z, block, ao[0], ao[1], ao[2], ao[3] });
                    }
                }
            }
}

#define NORMALIZED_INDICES
#define CALCULATE_AO
template <>
void mesher::mesh<mesher::MesherType::VECTOR_LOOKUP_TABLE>(
    std::vector<cfg::Vertex> & mesh,
    const std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> & chunks
) {
    constexpr std::uint8_t SHADOW_STRENGTH{ 60 };
    mesh.clear();
    mesh.reserve(1024 * 1024); // whatever

    const auto block_get = [&chunks] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        const auto chunk_position = Math::floor_div(p, cfg::CHUNK_SIZE);
        const auto block_index = Math::position_to_index(p, cfg::CHUNK_SIZE);
        const auto chunk_index = Math::position_to_index(chunk_position, cfg::MESH_CHUNK_SIZE);
        return chunks[chunk_index][block_index];
    };

    static constexpr glm::tvec3<cfg::Coord> DIM{ Math::add(cfg::MESH_SIZE, 2) };
    static constexpr glm::tvec3<cfg::Coord> FR{ cfg::MESH_OFFSET };
    static constexpr glm::tvec3<cfg::Coord> TO{ Math::add(FR, cfg::MESH_SIZE) };
    static constexpr glm::tvec3<cfg::Coord> OFFSET{ 1, 1, 1 };

    std::vector<cfg::Block> chunk;
    chunk.reserve(Math::volume(DIM));

    glm::tvec3<cfg::Coord> i;
    for (i.z = FR.z - 1; i.z < TO.z + 1; ++i.z)
        for (i.y = FR.y - 1; i.y < TO.y + 1; ++i.y)
            for (i.x = FR.x - 1; i.x < TO.x + 1; ++i.x)
                chunk.push_back(block_get(i));

    const auto block_get_2 = [&chunk] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        const auto block_index = Math::to_index(p, DIM);
        return chunk[block_index];
    };

#ifdef NORMALIZED_INDICES // TODO: array of structs
    static constexpr std::array<glm::tvec3<cfg::Coord>, 6> NEIGHBOUR_OFFSETS{ {
        { -1,  0,  0 }, {  1,  0,  0 },
        {  0, -1,  0 }, {  0,  1,  0 },
        {  0,  0, -1 }, {  0,  0,  1 },
    } };

    static constexpr std::array<std::array<glm::tvec3<uint8_t>, 4>, 6> QUAD_VERTEX_OFFSETS{ {
        { { { 0, 0, 0 }, { 0, 0, 1 }, { 0, 1, 1 }, { 0, 1, 0 } } },
        { { { 1, 0, 0 }, { 1, 1, 0 }, { 1, 1, 1 }, { 1, 0, 1 } } },

        { { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 0, 1 }, { 0, 0, 1 } } },
        { { { 0, 1, 0 }, { 0, 1, 1 }, { 1, 1, 1 }, { 1, 1, 0 } } },

        { { { 0, 0, 0 }, { 0, 1, 0 }, { 1, 1, 0 }, { 1, 0, 0 } } },
        { { { 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 } } },
    } };

    static constexpr std::array<std::array<glm::tvec3<cfg::Coord>, 8>, 6> AOS_OFFSETS{ {
        { { { -1, -1, -1 }, { -1,  0, -1 }, { -1,  1, -1 }, { -1, -1,  0 }, { -1,  1,  0 }, { -1, -1,  1 }, { -1,  0,  1 }, { -1,  1,  1 } } },
        { { {  1, -1, -1 }, {  1,  0, -1 }, {  1,  1, -1 }, {  1, -1,  0 }, {  1,  1,  0 }, {  1, -1,  1 }, {  1,  0,  1 }, {  1,  1,  1 } } },

        { { { -1, -1, -1 }, {  0, -1, -1 }, {  1, -1, -1 }, { -1, -1,  0 }, {  1, -1,  0 }, { -1, -1,  1 }, {  0, -1,  1 }, {  1, -1,  1 } } },
        { { { -1,  1, -1 }, {  0,  1, -1 }, {  1,  1, -1 }, { -1,  1,  0 }, {  1,  1,  0 }, { -1,  1,  1 }, {  0,  1,  1 }, {  1,  1,  1 } } },

        { { { -1, -1, -1 }, {  0, -1, -1 }, {  1, -1, -1 }, { -1,  0, -1 }, {  1,  0, -1 }, { -1,  1, -1 }, {  0,  1, -1 }, {  1,  1, -1 } } },
        { { { -1, -1,  1 }, {  0, -1,  1 }, {  1, -1,  1 }, { -1,  0,  1 }, {  1,  0,  1 }, { -1,  1,  1 }, {  0,  1,  1 }, {  1,  1,  1 } } },
    } };

    static constexpr std::array<std::array<glm::tvec3<uint8_t>, 4>, 6> AO_OFFSETS{ {
        { { { 1, 3, 0 }, { 1, 4, 2 }, { 3, 6, 5 }, { 4, 6, 7 } } },
        { { { 1, 3, 0 }, { 3, 6, 5 }, { 1, 4, 2 }, { 4, 6, 7 } } },

        { { { 1, 3, 0 }, { 3, 6, 5 }, { 1, 4, 2 }, { 4, 6, 7 } } },
        { { { 1, 3, 0 }, { 1, 4, 2 }, { 3, 6, 5 }, { 4, 6, 7 } } },

        { { { 1, 3, 0 }, { 1, 4, 2 }, { 3, 6, 5 }, { 4, 6, 7 } } },
        { { { 1, 3, 0 }, { 3, 6, 5 }, { 1, 4, 2 }, { 4, 6, 7 } } },
    } };
#else
    static constexpr std::array<glm::tvec3<cfg::Coord>, 6> NEIGHBOUR_OFFSETS{ {
        { 1, 0, 0 }, { -1, 0, 0 },
        { 0, 1, 0 }, { 0, -1, 0 },
        { 0, 0, 1 }, { 0, 0, -1 }
    } };

    static constexpr std::array<std::array<glm::tvec3<uint8_t>, 4>, 6> QUAD_VERTEX_OFFSETS{ {
        { { { 1, 0, 1 }, { 1, 0, 0 }, { 1, 1, 0 }, { 1, 1, 1 } } },
        { { { 0, 0, 0 }, { 0, 0, 1 }, { 0, 1, 1 }, { 0, 1, 0 } } },

        { { { 1, 1, 0 }, { 0, 1, 0 }, { 0, 1, 1 }, { 1, 1, 1 } } },
        { { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 0, 1 }, { 0, 0, 1 } } },

        { { { 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 } } },
        { { { 1, 0, 0 }, { 0, 0, 0 }, { 0, 1, 0 }, { 1, 1, 0 } } },
    } };

    static constexpr std::array<std::array<glm::tvec3<cfg::Coord>, 8>, 6> AOS_OFFSETS{ {
        { { {  1, -1,  0 }, {  1,  0, -1 }, {  1,  1,  0 }, {  1,  0,  1 }, {  1, -1, -1 }, {  1, -1,  1 }, {  1,  1,  1 }, {  1,  1, -1 } } },
        { { { -1, -1,  0 }, { -1,  0, -1 }, { -1,  1,  0 }, { -1,  0,  1 }, { -1, -1, -1 }, { -1, -1,  1 }, { -1,  1,  1 }, { -1,  1, -1 } } },
        { { { -1,  1,  0 }, {  0,  1, -1 }, { +1,  1,  0 }, {  0,  1, +1 }, { -1,  1, -1 }, { -1,  1, +1 }, { +1,  1, +1 }, { +1,  1, -1 } } },
        { { { -1, -1,  0 }, {  0, -1, -1 }, { +1, -1,  0 }, {  0, -1, +1 }, { -1, -1, -1 }, { -1, -1, +1 }, { +1, -1, +1 }, { +1, -1, -1 } } },
        { { { -1,  0,  1 }, {  0, -1,  1 }, { +1,  0,  1 }, {  0, +1,  1 }, { -1, -1,  1 }, { -1, +1,  1 }, { +1, +1,  1 }, { +1, -1,  1 } } },
        { { { -1,  0, -1 }, {  0, -1, -1 }, { +1,  0, -1 }, {  0, +1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 }, { +1, -1, -1 } } },
    } };

    static constexpr std::array<std::array<glm::tvec3<uint8_t>, 4>, 6> AO_OFFSETS{ {
        { { { 0, 3, 5 }, { 2, 3, 6 }, { 0, 1, 4 }, { 2, 1, 7 } } },
        { { { 0, 1, 4 }, { 2, 1, 7 }, { 0, 3, 5 }, { 2, 3, 6 } } },
        { { { 2, 1, 7 }, { 2, 3, 6 }, { 0, 1, 4 }, { 0, 3, 5 } } },

        { { { 0, 1, 4 }, { 0, 3, 5 }, { 2, 1, 7 }, { 2, 3, 6 } } },
        { { { 0, 1, 4 }, { 0, 3, 5 }, { 2, 1, 7 }, { 2, 3, 6 } } },
        { { { 2, 1, 7 }, { 2, 3, 6 }, { 0, 1, 4 }, { 0, 3, 5 } } },
    } };
#endif

    for (i.z = 1; i.z < cfg::MESH_SIZE.z + 1; ++i.z)
        for (i.y = 1; i.y < cfg::MESH_SIZE.y + 1; ++i.y)
            for (i.x = 1; i.x < cfg::MESH_SIZE.x + 1; ++i.x) {
                const auto block = block_get_2(i);
                if (block == cfg::Block{ 0 })
                    continue;

                for (size_t j = 0; j < 6; ++j) {
                    if (block_get_2(i + NEIGHBOUR_OFFSETS[j]) != cfg::Block{ 0 }) continue;

                    std::array<bool, 8> aos;
                    for (size_t k = 0; k < 8; ++k)
                        aos[k] = block_get_2(i + AOS_OFFSETS[j][k]) != cfg::Block{ 0 };

                    std::array<uint8_t, 4> ao;
                    for (size_t k = 0; k < 4; ++k) {
#ifdef CALCULATE_AO
                        ao[k] = Math::vertexAO(aos[AO_OFFSETS[j][k][0]], aos[AO_OFFSETS[j][k][1]], aos[AO_OFFSETS[j][k][2]]);
                        ao[k] = std::numeric_limits<std::uint8_t>::max() - ao[k] * SHADOW_STRENGTH;
#else
                        ao[k] = std::numeric_limits<std::uint8_t>::max();
#endif
                    }

                    const auto quad_pos = glm::tvec3<uint8_t>{ i - OFFSET };
                    for (size_t k = 0; k < 4; ++k) {
                        const auto vertex_position = quad_pos + QUAD_VERTEX_OFFSETS[j][k];
                        mesh.push_back({ vertex_position.x, vertex_position.y, vertex_position.z, block, ao[0], ao[1], ao[2], ao[3] });
                    }
                }
            }
}

template <>
void mesher::mesh<mesher::MesherType::HISTO_PYRAMID>(
    std::vector<cfg::Vertex> & mesh,
    const std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> & chunks
) {
    constexpr std::uint8_t SHADOW_STRENGTH{ 60 };
    mesh.clear();
    mesh.reserve(1024 * 1024); // whatever

    const auto block_get = [&chunks] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        const auto chunk_position = Math::floor_div(p, cfg::CHUNK_SIZE);
        const auto block_index = Math::position_to_index(p, cfg::CHUNK_SIZE);
        const auto chunk_index = Math::position_to_index(chunk_position, cfg::MESH_CHUNK_SIZE);
        return chunks[chunk_index][block_index];
    };

    static constexpr glm::tvec3<cfg::Coord> DIM{ Math::add(cfg::MESH_SIZE, 2) };
    static constexpr glm::tvec3<cfg::Coord> FR{ cfg::MESH_OFFSET };
    static constexpr glm::tvec3<cfg::Coord> TO{ Math::add(FR, cfg::MESH_SIZE) };
    static constexpr glm::tvec3<cfg::Coord> OFFSET{ 1, 1, 1 };

    std::vector<cfg::Block> chunk;
    chunk.reserve(Math::volume(DIM));

    glm::tvec3<cfg::Coord> i;
    for (i.z = FR.z - 1; i.z < TO.z + 1; ++i.z)
        for (i.y = FR.y - 1; i.y < TO.y + 1; ++i.y)
            for (i.x = FR.x - 1; i.x < TO.x + 1; ++i.x)
                chunk.push_back(block_get(i));

    const auto block_get_2 = [&chunk] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        const auto block_index = Math::to_index(p, DIM);
        return chunk[block_index];
    };

    std::vector<std::vector<uint32_t>> histo_pyramid;
    histo_pyramid.push_back({});
    auto & base_histo_layer = histo_pyramid.back();
    base_histo_layer.reserve(cfg::MESH_VOLUME * 6);
    static constexpr size_t MAX_QUAD_COUNT{ cfg::MESH_VOLUME * 6 };
    base_histo_layer.reserve(((MAX_QUAD_COUNT + 4) / 5) * 5);

    for (i.z = 1; i.z < cfg::MESH_SIZE.z + 1; ++i.z)
        for (i.y = 1; i.y < cfg::MESH_SIZE.y + 1; ++i.y)
            for (i.x = 1; i.x < cfg::MESH_SIZE.x + 1; ++i.x)
            {
                auto block = block_get_2(i);
                if (block == 0) {
                    for (size_t i = 0; i < 6; ++i)
                        base_histo_layer.push_back(0);
                    continue;
                }

                // X + 1
                const auto set_xp = block_get_2({ i.x + 1, i.y, i.z }) == 0;
                base_histo_layer.push_back(set_xp);
                if (set_xp)
                {
                    const bool aos[8]{
                        block_get_2({ i.x + 1, i.y - 1, i.z     }) != 0,
                        block_get_2({ i.x + 1, i.y    , i.z - 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z     }) != 0,
                        block_get_2({ i.x + 1, i.y    , i.z + 1 }) != 0,

                        block_get_2({ i.x + 1, i.y - 1, i.z - 1 }) != 0,
                        block_get_2({ i.x + 1, i.y - 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[3], aos[6]),
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[2], aos[1], aos[7])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // X - 1
                const auto set_xm = block_get_2({ i.x - 1, i.y, i.z }) == 0;
                base_histo_layer.push_back(set_xm);
                if (set_xm)
                {
                    const bool aos[8]{
                        block_get_2({ i.x - 1, i.y - 1, i.z     }) != 0,
                        block_get_2({ i.x - 1, i.y    , i.z - 1 }) != 0,
                        block_get_2({ i.x - 1, i.y + 1, i.z     }) != 0,
                        block_get_2({ i.x - 1, i.y    , i.z + 1 }) != 0,

                        block_get_2({ i.x - 1, i.y - 1, i.z - 1 }) != 0,
                        block_get_2({ i.x - 1, i.y - 1, i.z + 1 }) != 0,
                        block_get_2({ i.x - 1, i.y + 1, i.z + 1 }) != 0,
                        block_get_2({ i.x - 1, i.y + 1, i.z - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[3], aos[6])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Y + 1
                const auto set_yp = block_get_2({ i.x, i.y + 1, i.z }) == 0;
                base_histo_layer.push_back(set_yp);
                if (set_yp)
                {
                    const bool aos[8]{
                        block_get_2({ i.x - 1, i.y + 1, i.z     }) != 0,
                        block_get_2({ i.x    , i.y + 1, i.z - 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z     }) != 0,
                        block_get_2({ i.x    , i.y + 1, i.z + 1 }) != 0,

                        block_get_2({ i.x - 1, i.y + 1, i.z - 1 }) != 0,
                        block_get_2({ i.x - 1, i.y + 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6]),
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Y - 1
                const auto set_ym = block_get_2({ i.x, i.y - 1, i.z }) == 0;
                base_histo_layer.push_back(set_ym);
                if (set_ym)
                {
                    const bool aos[8]{
                        block_get_2({ i.x - 1, i.y - 1, i.z     }) != 0,
                        block_get_2({ i.x    , i.y - 1, i.z - 1 }) != 0,
                        block_get_2({ i.x + 1, i.y - 1, i.z     }) != 0,
                        block_get_2({ i.x    , i.y - 1, i.z + 1 }) != 0,

                        block_get_2({ i.x - 1, i.y - 1, i.z - 1 }) != 0,
                        block_get_2({ i.x - 1, i.y - 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y - 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y - 1, i.z - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Z + 1
                const auto set_zp = block_get_2({ i.x, i.y, i.z + 1 }) == 0;
                base_histo_layer.push_back(set_zp);
                if (set_zp)
                {
                    const bool aos[8]{
                        block_get_2({ i.x - 1, i.y    , i.z + 1 }) != 0,
                        block_get_2({ i.x    , i.y - 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y    , i.z + 1 }) != 0,
                        block_get_2({ i.x    , i.y + 1, i.z + 1 }) != 0,

                        block_get_2({ i.x - 1, i.y - 1, i.z + 1 }) != 0,
                        block_get_2({ i.x - 1, i.y + 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y - 1, i.z + 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Z - 1
                const auto set_zm = block_get_2({ i.x, i.y, i.z - 1 }) == 0;
                base_histo_layer.push_back(set_zm);
                if (set_zm)
                {
                    const bool aos[8]{
                        block_get_2({ i.x - 1, i.y    , i.z - 1 }) != 0,
                        block_get_2({ i.x    , i.y - 1, i.z - 1 }) != 0,
                        block_get_2({ i.x + 1, i.y    , i.z - 1 }) != 0,
                        block_get_2({ i.x    , i.y + 1, i.z - 1 }) != 0,

                        block_get_2({ i.x - 1, i.y - 1, i.z - 1 }) != 0,
                        block_get_2({ i.x - 1, i.y + 1, i.z - 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z - 1 }) != 0,
                        block_get_2({ i.x + 1, i.y - 1, i.z - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6]),
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    uint8_t tt = uint8_t(quad_pos.y) + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                }
            }

    // TODO: Histo Pyramid 5-to-1
    // TODO: sidepyramid for the 5th element
    
    histo_pyramid.back().resize(MAX_QUAD_COUNT == 1 ? 1 : (((MAX_QUAD_COUNT + 4) / 5) * 5));

    while (histo_pyramid.back().size() != 1) {
        // round to next multiple of 5

        histo_pyramid.push_back({});
        auto & current = histo_pyramid[histo_pyramid.size() - 2];
        auto & next = histo_pyramid[histo_pyramid.size() - 1];

        const size_t next_neto_size = current.size() / 5;
        const size_t next_bruto_size = current.size() == 5 ? 1 : (((next_neto_size + 4) / 5) * 5);
        next.reserve(next_bruto_size);
        
        for (size_t i = 0; i < current.size();) {
            uint32_t val = 0;
            for (size_t j = 0; j < 5; ++i, ++j) {
                val += current[i];
            }
            next.push_back(val);
        }

        next.resize(next_bruto_size, 0);
    }
/*
    for (const auto & p : histo_pyramid)
        Print(p.size());
    Print(histo_pyramid.back()[0]);
    int dummy = 42;
*/
}

template <>
void mesher::mesh<mesher::MesherType::MULTI_PASS>(
    std::vector<cfg::Vertex> & mesh,
    const std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> & chunks
) {
    constexpr std::uint8_t SHADOW_STRENGTH{ 60 };
    mesh.clear();
    mesh.reserve(1024 * 1024); // whatever

    const auto block_get = [&chunks] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        const auto chunk_position = Math::floor_div(p, cfg::CHUNK_SIZE);
        const auto block_index = Math::position_to_index(p, cfg::CHUNK_SIZE);
        const auto chunk_index = Math::position_to_index(chunk_position, cfg::MESH_CHUNK_SIZE);
        return chunks[chunk_index][block_index];
    };

    static constexpr glm::tvec3<cfg::Coord> DIM{ Math::add(cfg::MESH_SIZE, 2) };
    static constexpr glm::tvec3<cfg::Coord> FR{ cfg::MESH_OFFSET };
    static constexpr glm::tvec3<cfg::Coord> TO{ Math::add(FR, cfg::MESH_SIZE) };
    static constexpr glm::tvec3<cfg::Coord> OFFSET{ 1, 1, 1 };

    std::vector<cfg::Block> chunk;
    chunk.reserve(Math::volume(DIM));
    std::vector<uint32_t> mask;
    mask.reserve(Math::volume(DIM));

    // center pass
    glm::tvec3<cfg::Coord> i;
    for (i.z = FR.z - 1; i.z < TO.z + 1; ++i.z)
        for (i.y = FR.y - 1; i.y < TO.y + 1; ++i.y)
            for (i.x = FR.x - 1; i.x < TO.x + 1; ++i.x) {
                const auto val = block_get(i);
                chunk.push_back(val);
                mask.push_back(uint32_t(val != cfg::Block{ 0 }) << uint32_t(13));
            }

    // TODo: simplify to just incrementing iteratos
    const auto mask_get = [&mask] (const glm::tvec3<cfg::Coord> & p) -> uint32_t & {
        return mask[Math::to_index(p, DIM)];
    };

    // y pass
    for (i.z = 1; i.z < cfg::MESH_SIZE.z + 1; ++i.z)
        for (i.y = 1; i.y < cfg::MESH_SIZE.y + 1; ++i.y)
            for (i.x = 1; i.x < cfg::MESH_SIZE.x + 1; ++i.x) {
                const auto is1 = mask_get({ i.x, i.y - 1, i.z });
                const auto is2 = mask_get({ i.x, i.y + 1, i.z });
                mask_get(i) |= (is1 >> uint32_t(3)) | (is2 << uint32_t(3));
            }

    // x pass
    for (i.z = 1; i.z < cfg::MESH_SIZE.z + 1; ++i.z)
        for (i.y = 1; i.y < cfg::MESH_SIZE.y + 1; ++i.y)
            for (i.x = 1; i.x < cfg::MESH_SIZE.x + 1; ++i.x) {
                const auto is1 = mask_get({ i.x - 1, i.y, i.z });
                const auto is2 = mask_get({ i.x + 1, i.y, i.z });
                mask_get(i) |= (is1 >> uint32_t(1)) | (is2 << uint32_t(1));
            }

    // z pass
    for (i.z = 1; i.z < cfg::MESH_SIZE.z + 1; ++i.z)
        for (i.y = 1; i.y < cfg::MESH_SIZE.y + 1; ++i.y)
            for (i.x = 1; i.x < cfg::MESH_SIZE.x + 1; ++i.x) {
                const auto is1 = mask_get({ i.x, i.y, i.z - 1 });
                const auto is2 = mask_get({ i.x, i.y, i.z + 1 });
                mask_get(i) |= (is1 >> uint32_t(9)) | (is2 << uint32_t(9));
            }

    const auto block_get_2 = [&chunk] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        return chunk[Math::to_index(p, DIM)];
    };

    for (i.z = 1; i.z < cfg::MESH_SIZE.z + 1; ++i.z)
        for (i.y = 1; i.y < cfg::MESH_SIZE.y + 1; ++i.y)
            for (i.x = 1; i.x < cfg::MESH_SIZE.x + 1; ++i.x)
            {
                auto block = block_get_2(i);
                if (block == 0) continue;
                const auto mask = mask_get(i);

                // X + 1
                if ((mask & (uint32_t(1) << ((1 + 1) + 3 * 1 + 9 * 1))) == 0)
                {
                    const bool aos[8]{
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 - 1) + 9 * (1    ))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1    ) + 9 * (1 - 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 + 1) + 9 * (1    ))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1    ) + 9 * (1 + 1))) != 0,

                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 - 1) + 9 * (1 - 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 - 1) + 9 * (1 + 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 + 1) + 9 * (1 + 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 + 1) + 9 * (1 - 1))) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[3], aos[6]),
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[2], aos[1], aos[7])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // X - 1
                if ((mask & (uint32_t(1) << ((1 - 1) + 3 * 1 + 9 * 1))) == 0)
                {
                    const bool aos[8]{
                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 - 1) + 9 * (1    ))) != 0,
                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1    ) + 9 * (1 - 1))) != 0,
                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 + 1) + 9 * (1    ))) != 0,
                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1    ) + 9 * (1 + 1))) != 0,

                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 - 1) + 9 * (1 - 1))) != 0,
                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 - 1) + 9 * (1 + 1))) != 0,
                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 + 1) + 9 * (1 + 1))) != 0,
                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 + 1) + 9 * (1 - 1))) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[3], aos[6])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Y + 1
                if ((mask & (uint32_t(1) << (1 + 3 * (1 + 1) + 9 * 1))) == 0)
                {
                    const bool aos[8]{
                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 + 1) + 9 * (1    ))) != 0,
                        (mask & uint32_t(1) << ((1    ) + 3 * (1 + 1) + 9 * (1 - 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 + 1) + 9 * (1    ))) != 0,
                        (mask & uint32_t(1) << ((1    ) + 3 * (1 + 1) + 9 * (1 + 1))) != 0,

                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 + 1) + 9 * (1 - 1))) != 0,
                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 + 1) + 9 * (1 + 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 + 1) + 9 * (1 + 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 + 1) + 9 * (1 - 1))) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6]),
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Y - 1
                if ((mask & (uint32_t(1) << (1 + 3 * (1 - 1) + 9 * 1))) == 0)
                {
                    const bool aos[8]{
                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 - 1) + 9 * (1    ))) != 0,
                        (mask & uint32_t(1) << ((1    ) + 3 * (1 - 1) + 9 * (1 - 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 - 1) + 9 * (1    ))) != 0,
                        (mask & uint32_t(1) << ((1    ) + 3 * (1 - 1) + 9 * (1 + 1))) != 0,

                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 - 1) + 9 * (1 - 1))) != 0,
                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 - 1) + 9 * (1 + 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 - 1) + 9 * (1 + 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 - 1) + 9 * (1 - 1))) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Z + 1
                if ((mask & (uint32_t(1) << (1 + 3 * 1 + 9 * (1 + 1)))) == 0)
                {
                    const bool aos[8]{
                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1    ) + 9 * (1 + 1))) != 0,
                        (mask & uint32_t(1) << ((1    ) + 3 * (1 - 1) + 9 * (1 + 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1    ) + 9 * (1 + 1))) != 0,
                        (mask & uint32_t(1) << ((1    ) + 3 * (1 + 1) + 9 * (1 + 1))) != 0,

                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 - 1) + 9 * (1 + 1))) != 0,
                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 + 1) + 9 * (1 + 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 + 1) + 9 * (1 + 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 - 1) + 9 * (1 + 1))) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Z - 1
                if ((mask & (uint32_t(1) << (1 + 3 * 1 + 9 * (1 - 1)))) == 0)
                {
                    const bool aos[8]{
                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1    ) + 9 * (1 - 1))) != 0,
                        (mask & uint32_t(1) << ((1    ) + 3 * (1 - 1) + 9 * (1 - 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1    ) + 9 * (1 - 1))) != 0,
                        (mask & uint32_t(1) << ((1    ) + 3 * (1 + 1) + 9 * (1 - 1))) != 0,

                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 - 1) + 9 * (1 - 1))) != 0,
                        (mask & uint32_t(1) << ((1 - 1) + 3 * (1 + 1) + 9 * (1 - 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 + 1) + 9 * (1 - 1))) != 0,
                        (mask & uint32_t(1) << ((1 + 1) + 3 * (1 - 1) + 9 * (1 - 1))) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6]),
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    uint8_t tt = uint8_t(quad_pos.y) + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                }
            }
}

template <>
void mesher::mesh<mesher::MesherType::COPY_THEN_MESH>(
    std::vector<cfg::Vertex> & mesh,
    const std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> & chunks
) {
    constexpr std::uint8_t SHADOW_STRENGTH{ 60 };
    mesh.clear();
    mesh.reserve(1024 * 1024); // whatever

    const auto block_get = [&chunks] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        const auto chunk_position = Math::floor_div(p, cfg::CHUNK_SIZE);
        const auto block_index = Math::position_to_index(p, cfg::CHUNK_SIZE);
        const auto chunk_index = Math::position_to_index(chunk_position, cfg::MESH_CHUNK_SIZE);
        return chunks[chunk_index][block_index];
    };

    static constexpr glm::tvec3<cfg::Coord> DIM{ Math::add(cfg::MESH_SIZE, 2) };
    static constexpr glm::tvec3<cfg::Coord> FR{ cfg::MESH_OFFSET };
    static constexpr glm::tvec3<cfg::Coord> TO{ Math::add(FR, cfg::MESH_SIZE) };
    static constexpr glm::tvec3<cfg::Coord> OFFSET{ 1, 1, 1 };

    std::vector<cfg::Block> chunk;
    chunk.reserve(Math::volume(DIM));

    glm::tvec3<cfg::Coord> i;
    for (i.z = FR.z - 1; i.z < TO.z + 1; ++i.z)
        for (i.y = FR.y - 1; i.y < TO.y + 1; ++i.y)
            for (i.x = FR.x - 1; i.x < TO.x + 1; ++i.x)
                chunk.push_back(block_get(i));

    const auto block_get_2 = [&chunk] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        const auto block_index = Math::to_index(p, DIM);
        return chunk[block_index];
    };


    for (i.z = 1; i.z < cfg::MESH_SIZE.z + 1; ++i.z)
        for (i.y = 1; i.y < cfg::MESH_SIZE.y + 1; ++i.y)
            for (i.x = 1; i.x < cfg::MESH_SIZE.x + 1; ++i.x)
            {
                auto block = block_get_2(i);
                if (block == 0) continue;

                // X + 1
                if (block_get_2({ i.x + 1, i.y, i.z }) == 0)
                {
                    const bool aos[8]{
                        block_get_2({ i.x + 1, i.y - 1, i.z     }) != 0,
                        block_get_2({ i.x + 1, i.y    , i.z - 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z     }) != 0,
                        block_get_2({ i.x + 1, i.y    , i.z + 1 }) != 0,

                        block_get_2({ i.x + 1, i.y - 1, i.z - 1 }) != 0,
                        block_get_2({ i.x + 1, i.y - 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[3], aos[6]),
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[2], aos[1], aos[7])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // X - 1
                if (block_get_2({ i.x - 1, i.y, i.z }) == 0)
                {
                    const bool aos[8]{
                        block_get_2({ i.x - 1, i.y - 1, i.z     }) != 0,
                        block_get_2({ i.x - 1, i.y    , i.z - 1 }) != 0,
                        block_get_2({ i.x - 1, i.y + 1, i.z     }) != 0,
                        block_get_2({ i.x - 1, i.y    , i.z + 1 }) != 0,

                        block_get_2({ i.x - 1, i.y - 1, i.z - 1 }) != 0,
                        block_get_2({ i.x - 1, i.y - 1, i.z + 1 }) != 0,
                        block_get_2({ i.x - 1, i.y + 1, i.z + 1 }) != 0,
                        block_get_2({ i.x - 1, i.y + 1, i.z - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[3], aos[6])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Y + 1
                if (block_get_2({ i.x, i.y + 1, i.z }) == 0)
                {
                    const bool aos[8]{
                        block_get_2({ i.x - 1, i.y + 1, i.z     }) != 0,
                        block_get_2({ i.x    , i.y + 1, i.z - 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z     }) != 0,
                        block_get_2({ i.x    , i.y + 1, i.z + 1 }) != 0,

                        block_get_2({ i.x - 1, i.y + 1, i.z - 1 }) != 0,
                        block_get_2({ i.x - 1, i.y + 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6]),
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Y - 1
                if (block_get_2({ i.x, i.y - 1, i.z }) == 0)
                {
                    const bool aos[8]{
                        block_get_2({ i.x - 1, i.y - 1, i.z     }) != 0,
                        block_get_2({ i.x    , i.y - 1, i.z - 1 }) != 0,
                        block_get_2({ i.x + 1, i.y - 1, i.z     }) != 0,
                        block_get_2({ i.x    , i.y - 1, i.z + 1 }) != 0,

                        block_get_2({ i.x - 1, i.y - 1, i.z - 1 }) != 0,
                        block_get_2({ i.x - 1, i.y - 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y - 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y - 1, i.z - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Z + 1
                if (block_get_2({ i.x, i.y, i.z + 1 }) == 0)
                {
                    const bool aos[8]{
                        block_get_2({ i.x - 1, i.y    , i.z + 1 }) != 0,
                        block_get_2({ i.x    , i.y - 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y    , i.z + 1 }) != 0,
                        block_get_2({ i.x    , i.y + 1, i.z + 1 }) != 0,

                        block_get_2({ i.x - 1, i.y - 1, i.z + 1 }) != 0,
                        block_get_2({ i.x - 1, i.y + 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z + 1 }) != 0,
                        block_get_2({ i.x + 1, i.y - 1, i.z + 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5]),
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                }

                // Z - 1
                if (block_get_2({ i.x, i.y, i.z - 1 }) == 0)
                {
                    const bool aos[8]{
                        block_get_2({ i.x - 1, i.y    , i.z - 1 }) != 0,
                        block_get_2({ i.x    , i.y - 1, i.z - 1 }) != 0,
                        block_get_2({ i.x + 1, i.y    , i.z - 1 }) != 0,
                        block_get_2({ i.x    , i.y + 1, i.z - 1 }) != 0,

                        block_get_2({ i.x - 1, i.y - 1, i.z - 1 }) != 0,
                        block_get_2({ i.x - 1, i.y + 1, i.z - 1 }) != 0,
                        block_get_2({ i.x + 1, i.y + 1, i.z - 1 }) != 0,
                        block_get_2({ i.x + 1, i.y - 1, i.z - 1 }) != 0
                    };

                    const glm::tvec4<uint8_t> ao = std::numeric_limits<std::uint8_t>::max() - glm::tvec4<uint8_t>{
                        Math::vertexAO(aos[2], aos[1], aos[7]),
                        Math::vertexAO(aos[2], aos[3], aos[6]),
                        Math::vertexAO(aos[0], aos[1], aos[4]),
                        Math::vertexAO(aos[0], aos[3], aos[5])
                    } * SHADOW_STRENGTH;

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    uint8_t tt = uint8_t(quad_pos.y) + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                }
            }
}

template <>
void mesher::mesh<mesher::MesherType::STANDARD>(
    std::vector<cfg::Vertex> & mesh,
    const std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> & chunks
) {
    static constexpr glm::tvec3<cfg::Coord> fr{ cfg::MESH_OFFSET };
    static constexpr glm::tvec3<cfg::Coord> to{ Math::add(fr, cfg::MESH_SIZE) };

    const auto block_get = [&chunks] (const glm::tvec3<cfg::Coord> & p) -> cfg::Block & {
        const auto chunk_position = Math::floor_div(p, cfg::CHUNK_SIZE);
        const auto block_index = Math::position_to_index(p, cfg::CHUNK_SIZE);
        const auto chunk_index = Math::position_to_index(chunk_position, cfg::MESH_CHUNK_SIZE);
        return chunks[chunk_index][block_index];
    };

    constexpr std::uint8_t SHADOW_STRENGTH{ 60 };

    mesh.clear();
    mesh.reserve(1024 * 1024); // whatever

    glm::tvec3<cfg::Coord> i;
    static constexpr glm::tvec3<cfg::Coord> OFFSET{ cfg::MESH_OFFSET };
    for (i.z = fr.z; i.z < to.z; ++i.z)
        for (i.y = fr.y; i.y < to.y; ++i.y)
            for (i.x = fr.x; i.x < to.x; ++i.x)
            {
                auto block = block_get(i); // TODO: this can in future probably be set to reference
                if (block == 0) continue;

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

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
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

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
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

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
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

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
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

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_po1.z, block, ao[0], ao[1], ao[2], ao[3] });
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

                    const auto quad_pos_int = i - OFFSET;
                    const auto quad_pos = glm::tvec3<uint8_t>{ quad_pos_int.x, quad_pos_int.y, quad_pos_int.z };
                    const auto quad_po1 = quad_pos + uint8_t(1);

                    uint8_t tt = uint8_t(quad_pos.y) + uint8_t(1);

                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_pos.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_pos.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                    mesh.push_back(cfg::Vertex{ quad_po1.x, quad_po1.y, quad_pos.z, block, ao[0], ao[1], ao[2], ao[3] });
                }
            }
}


    
void mesher::generic(
    std::vector<cfg::Vertex> & out_mesh,
    const std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> & chunks
) {
    // benchmark
    std::vector<void (*) (std::vector<cfg::Vertex> &, const std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> &)> meshing_functions{
//        mesh<MesherType::STANDARD>,
//        mesh<MesherType::COPY_THEN_MESH>,
//        mesh<MesherType::INDEX_LOOKUP_TABLE>,
//        mesh<MesherType::INDEX_LOOKUP_TABLE_UNROLL>,
        mesh<MesherType::INDEX_LOOKUP_TABLE_UNROLL_SEMI_NO_LAMBDA>,
    };

    struct Results {
        std::vector<cfg::Vertex> mesh;
        double run_time;
    };

    std::vector<Results> results;
    results.reserve(meshing_functions.size());

    for (const auto & meshing_function : meshing_functions) {
        results.push_back({});
        const auto start = std::chrono::high_resolution_clock::now();
        meshing_function(results.back().mesh, chunks);
        const auto stop = std::chrono::high_resolution_clock::now();
        results.back().run_time = std::chrono::duration_cast<std::chrono::duration<double>>(stop - start).count();
    }
/*
    std::string result_string;
    result_string += std::to_string(results[0].mesh.size());
    for (const auto & result : results) {
        result_string += " ";
        result_string += std::to_string(result.run_time);
    }
    Print(result_string);
*/
//    out_mesh = std::move(results[std::rand() % results.size()].mesh);
    out_mesh = std::move(results.back().mesh);
}
