#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>

class VoxelStorage {
public:
    using Block = uint8_t;
    template<typename T> struct Vec3 { 
        using element_type = T;
        T x, y, z; 
    };
    using s32Vec3 = Vec3<int32_t>;
    using u32Vec3 = Vec3<uint32_t>;

    static constexpr s32Vec3 REGION_SIZE{ 32, 32, 32 };
    static constexpr s32Vec3 CHUNK_SIZE{ 16, 16, 16 };

    VoxelStorage();
    ~VoxelStorage();
    Block * get(int32_t x, int32_t y, int32_t z, bool cache, bool edit);


private:

    static constexpr int32_t CHUNK_VOLUME{ CHUNK_SIZE.x * CHUNK_SIZE.y * CHUNK_SIZE.z };
    static constexpr int32_t REGION_VOLUME{ REGION_SIZE.x * REGION_SIZE.y * REGION_SIZE.z };

    // constants
    static constexpr uint32_t DEFRAGMENT_GARBAGE_THRESHOLD{ 16 * 1024 };
    // TODO: *_MAP_SIZE > *_HEAP_SIZE (for best performance)
    static constexpr uint32_t CHUNK_MAP_SIZE{ 1024 }; // TODO: should that be prime?
    static constexpr uint32_t CHUNK_HEAP_SIZE{ 32 * 1024 };
    static constexpr uint32_t REGION_MAP_SIZE{ 64 }; // TODO: should that be prime?
    static constexpr uint32_t REGION_HEAP_SIZE{ 2 * 1024 };
    // TODO: choose factors at runtime and rehash when hash collision attack detected?
    static constexpr u32Vec3 CHUNK_POSITION_HASH_SEED{ 73856093, 19349663, 83492791 };
    static constexpr u32Vec3 REGION_POSITION_HASH_SEED{ 73856093, 19349663, 83492791 };

    template <typename K, typename V>
    struct MapNode {
        K key;
        V val;
        // list
        MapNode * next;
        MapNode * prev;
        // map
        MapNode * * head;
        MapNode * down;
    };

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

    template<typename T, uint32_t N>
    class LeastRecentlyUsed {
        // 2-in-1: LRU cache and unused cache
    public:
        LeastRecentlyUsed();

        T * get_node(const s32Vec3 & node_position, uint32_t node_index);
        T * get_from_heap();
        T * remove_lru_node();
        void add_node(T * node, uint32_t index);
        // only constructor and destructor calls it:
        void add_to_heap(T * node);

    private:
        // LRU map
        T * m_map[N];
        // free heap list
        T * m_free_list;
        // LRU list
        T * m_front;
        T * m_back;

        void remove_node(T * node);

    };

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
    ChunkNode * get_chunk(const s32Vec3 & chunk_position);
    void close_chunk(ChunkNode * chunk);
    void create_new_chunk(ChunkNode * chunk, const s32Vec3 & chunk_position);

};
