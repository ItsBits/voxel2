#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>

class VoxelStorage {
public:
    template<typename T> struct Vec3 { 
        using element_type = T;
        T x, y, z; 
    };
    using Block = uint8_t;
    using s32Vec3 = Vec3<int32_t>;
    using u32Vec3 = Vec3<uint32_t>;
    static constexpr s32Vec3 REGION_SIZE{ 32, 32, 32 };
    static constexpr s32Vec3 CHUNK_SIZE{ 16, 16, 16 };
    static constexpr int32_t CHUNK_VOLUME{ CHUNK_SIZE.x * CHUNK_SIZE.y * CHUNK_SIZE.z };
    static constexpr int32_t REGION_VOLUME{ REGION_SIZE.x * REGION_SIZE.y * REGION_SIZE.z };
    // TODO: static assertions for sane values

private:
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

public:
    // public interface
    VoxelStorage();
    ~VoxelStorage();

    Block * get(const s32Vec3 & chunk_position, bool cache, bool edit);

    void set_block(const s32Vec3 & block_position, const Block & b);
    void get_chunk_blocks(const s32Vec3 & chunk_position, Block * c);
    // pointer is valid until CHUNK_HEAP_SIZE get_* functions are called or 1 set_* function is called
    const Block * get_chunk_blocks_reference(const s32Vec3 & chunk_position);

    // TODO: rework interface
    /*
    get_chunk_blocks_reference(chunk_position, single_use, editable)
    single_use == true => do not put in cache
    editable == true => set dirty to true

    // just convenience functions:
    get_chunk_copy(chunk_position)
    set_block(block_position, value)
    set_chunk(chunk_position, single_use, blocks)
    */

    // TODO: huge idea
    /*
    - treat chunks simpy as block of data and let the user deal with the details. (get rid of Block and use Byte for example)
    - go from 3D to 1D key aka. chunk index is single 32 or 64 or whatever bit int instead of vec3
    - no more set_block function
    - only get_chunk_blocks_reference(chunk_position, single_use, editable)
    */

private:
    struct Chunk {
        // TODO: maybe also store compressed version here when compressed requested and dirty
        s32Vec3 position;
        bool dirty;
        Block blocks[CHUNK_VOLUME];
        // list
        Chunk * next;
        Chunk * previous;
        // map
        Chunk * * head;
        Chunk * down;
    };

    struct Region {
        s32Vec3 position;
        int fd;
        uint32_t garbage;
        uint32_t end;

        void read(void * buffer, size_t count, size_t position);
        void write(const void * buffer, size_t count, size_t position);
        // returns true if file is new
        bool open_file(const char * const file_name);
        Region * next;
        Region * previous;
        Region * * head;
        Region * down;
    };

    template<typename T, uint32_t N>
    class LeastRecentlyUsed {
        // 2-in-1: LRU cache and unused cache
    public:
        LeastRecentlyUsed();

        void add_node(T * node, uint32_t index);
        T * get_node(const s32Vec3 & node_position, uint32_t node_index);
        T * remove_lru_node();
        void remove_node(T * node);

        void add_to_heap(T * node);
        T * get_from_heap();

    private:
        // LRU map
        T * m_map[N];
        // free heap list
        T * m_free_list;
        // LRU list
        T * m_front;
        T * m_back;
    };

    // variables
    LeastRecentlyUsed<Chunk, CHUNK_MAP_SIZE> m_chunk_LRU;
    LeastRecentlyUsed<Region, REGION_MAP_SIZE> m_region_LRU;
    const size_t c_maximum_compressed_size;
    std::unique_ptr<uint8_t[]> m_compress_buffer;
    std::unique_ptr<uint32_t[]> m_defragment_buffer_raw;
    std::unique_ptr<u32Vec3[]> m_defragment_buffer;

    // functions
    uint32_t region_index(const s32Vec3 & region_position);
    Region * get_region(const s32Vec3 & region_position);
    void close_region(Region * region);
    void defragment_region(Region * region);
    uint32_t chunk_index(const s32Vec3 & chunk_position);
    Chunk * get_chunk(const s32Vec3 & chunk_position);
    void close_chunk(Chunk * chunk);
    void create_new_chunk(Chunk * chunk, const s32Vec3 & chunk_position);
    
    // TODO: per region compression (for region transportation)
    //         -> uncompress all chunks and compress them as one stream
    
    // TODO: also implement get_chunk_blocks_compressed()
    //       load from region if chunk not dirty, else load compressed from chunk (if not compressed up to date compress)
};
