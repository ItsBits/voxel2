#pragma once

#include <cstdint>
#include <glm/vec3.hpp>
#include "Math.hpp"

// TODO: configure at runtime by making this namespace a class and
// members non static and pass the class to whoever needs it
namespace cfg {
    using Block = uint8_t;
    using Coord = int32_t;

    // when generating will always produce same chunk and generating is very cheap
    // like worldgen::WorldGenType::AIR, this can be set to false to save disk space
    static constexpr bool SAVE_NEWLY_GENERATED_CHUNKS{ true };
    static constexpr size_t DEFRAGMENT_GARBAGE_THRESHOLD{ 1024 * 128 };

    static constexpr double MAX_RAY_LENGTH{ 10 };
    static constexpr size_t MESH_QUEUE_SIZE_LIMIT{ 128 };
    static constexpr size_t BLOCK_UPDATE_QUEUE_SIZE_LIMIT{ 8 };

    // TODO: static asserts to check for sane and valid values
    // most must be > 1
    static constexpr glm::tvec3<Coord> CHUNK_SIZE{ 16, 16, 16 };
    static constexpr glm::tvec3<Coord> CHUNK_ARRAY_SIZE{ 32, 16, 32 };
    static constexpr glm::tvec3<Coord> MESH_SIZE{ 16, 16, 16 };
    static constexpr glm::tvec3<Coord> MESH_ARRAY_SIZE{ 32, 16, 32 };
    static constexpr glm::tvec3<Coord> REGION_SIZE{ 32, 32, 32 };

    static constexpr glm::tvec3<Coord> MESH_OFFSET{ 8, 8, 8 }; // TODO: MESH_SIZE / 2
    static constexpr glm::tvec3<Coord> MESH_LOADING_RADIUS{ 14, 6, 14 };

    static constexpr glm::tvec3<Coord> BLOCK_MESH_EFFECT_RADIUS{ 1, 1, 1 };

    // TODO: better calculation method
    static_assert( // for CHUNK_LOADING_RADIUS
        MESH_OFFSET.x > 0 &&
        MESH_OFFSET.y > 0 &&
        MESH_OFFSET.z > 0
    );
    static_assert( // for CHUNK_LOADING_RADIUS
        MESH_OFFSET.x < MESH_SIZE.x &&
        MESH_OFFSET.y < MESH_SIZE.y &&
        MESH_OFFSET.z < MESH_SIZE.z
    );
    static_assert( // for CHUNK_LOADING_RADIUS
        CHUNK_SIZE.x == MESH_SIZE.x &&
        CHUNK_SIZE.y == MESH_SIZE.y &&
        CHUNK_SIZE.z == MESH_SIZE.z
    );
    // it's actually a little more
    static constexpr glm::tvec3<Coord> CHUNK_LOADING_RADIUS{ MESH_LOADING_RADIUS };
    
    static constexpr Coord CHUNK_VOLUME{ Math::volume(CHUNK_SIZE) };
    static constexpr Coord CHUNK_ARRAY_VOLUME{ Math::volume(CHUNK_ARRAY_SIZE) };
    static constexpr Coord MESH_VOLUME{ Math::volume(MESH_SIZE) };
    static constexpr Coord MESH_ARRAY_VOLUME{ Math::volume(MESH_ARRAY_SIZE) };
    static constexpr Coord REGION_VOLUME{ Math::volume(REGION_SIZE) };

    // TODO: profile and see if caching makes sense (it is supposed to reduce ChunkContainer::m_mutex contention)
    static constexpr glm::tvec3<Coord> WORKER_REGION_CACHE_SIZE{ 2, 2, 2 };
    static constexpr Coord WORKER_REGION_CACHE_VOLUME{ Math::volume(WORKER_REGION_CACHE_SIZE) };

    static constexpr glm::tvec3<Coord> MESH_LOADING_SIZE{
         MESH_LOADING_RADIUS.x * 2 + 1,
         MESH_LOADING_RADIUS.y * 2 + 1,
         MESH_LOADING_RADIUS.z * 2 + 1
    };

    static constexpr Coord MESH_LOADING_VOLUME{ Math::volume(MESH_LOADING_SIZE) };

    static constexpr size_t WORKER_THREAD_COUNT{ 4 };
    static_assert(WORKER_THREAD_COUNT < MESH_QUEUE_SIZE_LIMIT, "Becasue ~VoxelContainer().");
    // TODO: calcualte good value from REGION_SIZE, MESH_LOADING_SIZE, WORKER_THREAD_COUNT, WORKER_REGION_CACHE_VOLUME ...
    static constexpr size_t REGION_CACHE_SIZE{ 128 };
    static_assert((WORKER_REGION_CACHE_VOLUME + 2) * WORKER_THREAD_COUNT < REGION_CACHE_SIZE);

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

    static constexpr size_t COMPRESS_BUFFER_SIZE_IN_BYTES{ CHUNK_VOLUME * sizeof(Block) * 2 };

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
    using RegByte = uint8_t;

    // deprecated
    struct Vertex {
        uint8_t vals[8];
    };

}
