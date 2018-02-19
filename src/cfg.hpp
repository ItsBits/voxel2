#pragma once

#include <cstdint>
#include <glm/vec3.hpp>

#include "Math.hpp"

// TODO: configure at runtime by making this namespace a class and
// members non static and pass the class to whoever needs it
namespace cfg {
    using Block = uint8_t;
    using Coord = int32_t;

    // TODO: static asserts to check for sane and valid values
    static constexpr glm::tvec3<Coord> CHUNK_SIZE{ 16, 16, 16 };
    static constexpr glm::tvec3<Coord> CHUNK_ARRAY_SIZE{ 16, 16, 16 };
    static constexpr glm::tvec3<Coord> MESH_SIZE{ 16, 16, 16 };
    static constexpr glm::tvec3<Coord> MESH_ARRAY_SIZE{ 16, 16, 16 };
    static constexpr glm::tvec3<Coord> REGION_SIZE{ 32, 32, 32 };

    static constexpr glm::tvec3<Coord> MESH_OFFSET{ 8, 8, 8 };
    static constexpr glm::tvec3<Coord> MESH_LOADING_RADIUS{ 6, 6, 6 };
    
    static constexpr Coord CHUNK_VOLUME{ Math::volume(CHUNK_SIZE) };
    static constexpr Coord CHUNK_ARRAY_VOLUME{ Math::volume(CHUNK_ARRAY_SIZE) };
    static constexpr Coord MESH_VOLUME{ Math::volume(MESH_SIZE) };
    static constexpr Coord MESH_ARRAY_VOLUME{ Math::volume(MESH_ARRAY_SIZE) };
    static constexpr Coord REGION_VOLUME{ Math::volume(REGION_SIZE) };

    static constexpr glm::tvec3<Coord> MESH_LOADING_SIZE{
         MESH_LOADING_RADIUS.x * 2 + 1,
         MESH_LOADING_RADIUS.y * 2 + 1,
         MESH_LOADING_RADIUS.z * 2 + 1
    };

    static constexpr Coord MESH_LOADING_VOLUME{ Math::volume(MESH_LOADING_SIZE) };

    static constexpr size_t WORKER_THREAD_COUNT{ 4 };

    static_assert(
        cfg::MESH_LOADING_RADIUS.x >= 0 &&
        cfg::MESH_LOADING_RADIUS.y >= 0 &&
        cfg::MESH_LOADING_RADIUS.z >= 0
    );

    static_assert(
        cfg::MESH_SIZE.x % cfg::CHUNK_SIZE.x == 0 &&
        cfg::MESH_SIZE.y % cfg::CHUNK_SIZE.y == 0 &&
        cfg::MESH_SIZE.z % cfg::CHUNK_SIZE.z == 0
    );

    static constexpr glm::tvec3<cfg::Coord> MESH_BLOCK_START{ Math::add(MESH_OFFSET, -1) };
    static constexpr glm::tvec3<cfg::Coord> MESH_BLOCK_END{ Math::add(Math::add(MESH_OFFSET, MESH_SIZE), 1) };
    
    static constexpr glm::tvec3<cfg::Coord> MESH_CHUNK_START{ Math::floor_div(MESH_BLOCK_START, CHUNK_SIZE) };
    static constexpr glm::tvec3<cfg::Coord> MESH_CHUNK_END{ Math::add(Math::floor_div(MESH_BLOCK_END, CHUNK_SIZE), 1) };
    static constexpr glm::tvec3<cfg::Coord> MESH_CHUNK_SIZE{ Math::sub(MESH_CHUNK_END, MESH_CHUNK_START) };
    static constexpr Coord MESH_CHUNK_VOLUME{ Math::volume(MESH_CHUNK_SIZE) };

    static constexpr glm::tvec3<cfg::Coord> CHUNK_MESH_START{ Math::add(MESH_CHUNK_START, -1) };
    static constexpr glm::tvec3<cfg::Coord> CHUNK_MESH_END{ Math::add(MESH_CHUNK_END, -1) };
    static constexpr glm::tvec3<cfg::Coord> CHUNK_MESH_SIZE{ Math::sub(CHUNK_MESH_END, CHUNK_MESH_START) };
    static constexpr Coord CHUNK_MESH_VOLUME{ Math::volume(CHUNK_MESH_SIZE) };

    static_assert(
        MESH_CHUNK_VOLUME <= 8 &&
        MESH_LOADING_SIZE.x < CHUNK_ARRAY_SIZE.x &&
        MESH_LOADING_SIZE.y < CHUNK_ARRAY_SIZE.y &&
        MESH_LOADING_SIZE.z < CHUNK_ARRAY_SIZE.z,
        "Must be true, because of VoxelContainer::m_mesh_readines"
    );

    static_assert(
        cfg::MESH_SIZE.x == cfg::CHUNK_SIZE.x &&
        cfg::MESH_SIZE.y == cfg::CHUNK_SIZE.y &&
        cfg::MESH_SIZE.z == cfg::CHUNK_SIZE.z,
        "Due to incomplete iterator implementation, this must be true. And because CHUNK_MESH_START and CHUNK_MESH_END"
    );

    using RegUint = uint32_t;

    // deprecated
    struct Vertex {
        uint8_t vals[8];
    };
    static constexpr int32_t CHUNK_SIZE_X{ 16 };
    static constexpr int32_t CHUNK_SIZE_Y{ 16 };
    static constexpr int32_t CHUNK_SIZE_Z{ 16 };

    static constexpr int32_t REGION_SIZE_X{ 32 };
    static constexpr int32_t REGION_SIZE_Y{ 32 };
    static constexpr int32_t REGION_SIZE_Z{ 32 };

    static constexpr int32_t CHUNK_ARRAY_SIZE_X{ 16 };
    static constexpr int32_t CHUNK_ARRAY_SIZE_Y{ 16 };
    static constexpr int32_t CHUNK_ARRAY_SIZE_Z{ 16 };

    static constexpr int32_t MESH_SIZE_X{ 16 };
    static constexpr int32_t MESH_SIZE_Y{ 16 };
    static constexpr int32_t MESH_SIZE_Z{ 16 };

    static constexpr int32_t MESH_OFFSET_X{ 8 };
    static constexpr int32_t MESH_OFFSET_Y{ 8 };
    static constexpr int32_t MESH_OFFSET_Z{ 8 };

    static constexpr int32_t MESH_LOADING_RADIUS_X{ 5 };
    static constexpr int32_t MESH_LOADING_RADIUS_Y{ 5 };
    static constexpr int32_t MESH_LOADING_RADIUS_Z{ 5 };

    static constexpr int32_t MESH_ARRAY_SIZE_X{ 16 };
    static constexpr int32_t MESH_ARRAY_SIZE_Y{ 16 };
    static constexpr int32_t MESH_ARRAY_SIZE_Z{ 16 };

    static_assert(MESH_LOADING_RADIUS_X * 2 + 1 <= MESH_ARRAY_SIZE_X);
    static_assert(MESH_LOADING_RADIUS_Y * 2 + 1 <= MESH_ARRAY_SIZE_Y);
    static_assert(MESH_LOADING_RADIUS_Z * 2 + 1 <= MESH_ARRAY_SIZE_Z);

    //static constexpr int32_t REGION_VOLUME{ REGION_SIZE_X * REGION_SIZE_Y * REGION_SIZE_Z };
    //static constexpr int32_t CHUNK_VOLUME{ CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z };
    //static constexpr int32_t CHUNK_ARRAY_VOLUME{ CHUNK_ARRAY_SIZE_X * CHUNK_ARRAY_SIZE_Y * CHUNK_ARRAY_SIZE_Z };
    //static constexpr int32_t MESH_ARRAY_VOLUME{ MESH_ARRAY_SIZE_X * MESH_ARRAY_SIZE_Y * MESH_ARRAY_SIZE_Z };
    //static constexpr int32_t MESH_VOLUME{ MESH_SIZE_X * MESH_SIZE_Y * MESH_SIZE_Z };

}
