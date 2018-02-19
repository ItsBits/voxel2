#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>

#include "LRUH.hpp"
#include "Algebra.hpp"

class VoxelMap {
    /* CONFIGURATION */
public:
    using Block = uint8_t;
private:
    static constexpr int32_t CHUNK_VOLUME{ 16 * 16 * 16 };
    static constexpr int32_t REGION_SIZE_X{ 32 };
    static constexpr int32_t REGION_SIZE_Y{ 32 };
    static constexpr int32_t REGION_SIZE_Z{ 32 };
    static constexpr uint32_t DEFRAGMENT_GARBAGE_THRESHOLD{ 16 * 1024 };
    static constexpr uint32_t CHUNK_HEAP_SIZE{ 32 * 1024 };
    static constexpr uint32_t CHUNK_MAP_SIZE{ 4 * CHUNK_HEAP_SIZE };
    static constexpr uint32_t REGION_HEAP_SIZE{ 2 * 1024 };
    static constexpr uint32_t REGION_MAP_SIZE{ 4 * REGION_HEAP_SIZE };
    /* END OF CONFIGURATION */

public:
    VoxelMap();
    ~VoxelMap();
    struct ChunkPtr { Block * b; bool n; };
    ChunkPtr get(int32_t x, int32_t y, int32_t z, bool cache, bool edit);
    size_t memory_size() const { return CHUNK_HEAP_SIZE; }


private:
    static constexpr s32Vec3 REGION_SIZE{ REGION_SIZE_X, REGION_SIZE_Y, REGION_SIZE_Z };
    static constexpr int32_t REGION_VOLUME{ REGION_SIZE.x * REGION_SIZE.y * REGION_SIZE.z };

    // constants
    // TODO: choose factors at runtime and rehash when hash collision attack detected?
    static constexpr u32Vec3 CHUNK_POSITION_HASH_SEED{ 73856093, 19349663, 83492791 };
    static constexpr u32Vec3 REGION_POSITION_HASH_SEED{ 73856093, 19349663, 83492791 };

    struct Chunk {
        std::array<Block, CHUNK_VOLUME> blocks;
        bool dirty;
    };

    struct Region {
        int fd;
        uint32_t garbage;
        uint32_t end;
        void read(void * buffer, size_t count, size_t position);
        void write(const void * buffer, size_t count, size_t position);
        // returns true if file is new
        bool open_file(const char * const file_name);
    };

    using ChunkNode = MapNode<s32Vec3, Chunk>;
    using RegionNode = MapNode<s32Vec3, Region>;

    // variables
    LeastRecentlyUsed<ChunkNode, CHUNK_MAP_SIZE> m_chunk_LRU;
    LeastRecentlyUsed<RegionNode, REGION_MAP_SIZE> m_region_LRU;
    const size_t c_maximum_compressed_size;
    std::unique_ptr<uint8_t[]> m_compress_buffer;
    std::unique_ptr<uint32_t[]> m_defragment_buffer_raw;
    std::unique_ptr<u32Vec3[]> m_defragment_buffer;

    // functions
    uint32_t region_index(const s32Vec3 & region_position);
    RegionNode * get_region(const s32Vec3 & region_position);
    void close_region(RegionNode * region);
    void defragment_region(RegionNode * region);
    uint32_t chunk_index(const s32Vec3 & chunk_position);
    ChunkNode * get_chunk(const s32Vec3 & chunk_position, bool & is_new);
    void close_chunk(ChunkNode * chunk);
    void create_new_chunk(ChunkNode * chunk, const s32Vec3 & chunk_position);

};
