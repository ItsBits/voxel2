#include "Region.hpp"

#include <array>
#include <memory> // TODO: remove

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <zlib.h>

#include "Print.hpp"

Region::Region(const glm::tvec3<cfg::Coord> & region_position) {
    ref_count = 0;
    
    std::array<char, 128> file_name;
    const int name_result = std::snprintf(
        std::begin(file_name), file_name.size(), "%s/%i|%i|%i",
        "world", region_position.x, region_position.y, region_position.z
    );

    if (name_result < 0 || name_result >= file_name.size())
        throw std::runtime_error("Oops, region file name too large.");

    fd = open(file_name.data(), O_RDWR | O_CREAT, 0666);
    struct stat file_info;
    const auto result = fstat(fd, &file_info);
    const auto new_region = file_info.st_size == 0;

    if (new_region) {
        const cfg::RegUint end_value = (2 * cfg::REGION_VOLUME + 2) * sizeof(cfg::RegUint);
        end.store(end_value);
        garbage.store(0);
        ftruncate(fd, end_value);
    } else {
        std::array<cfg::RegUint, 2> eg;
        read(eg.data(), 2 * sizeof(cfg::RegUint), 0);
        end.store(eg[0]);
        garbage.store(eg[1]);
    }
}

size_t Region::refCountGet() const { return ref_count; }
void Region::refCountIncrement() { ++ref_count; assert(ref_count != 0); }
void Region::refCountDecrement() { assert(ref_count != 0); --ref_count; }

Region::~Region() {
    if (fd < 0) return;
    const std::array<cfg::RegUint, 2> eg{ end.load(), garbage.load() };
    write(eg.data(), 2 * sizeof(cfg::RegUint), 0);
    close(fd);
}

void Region::read(void * buffer, cfg::RegUint count, cfg::RegUint position) {
    pread(fd, buffer, count, position);
}

void Region::write(const void * buffer, cfg::RegUint count, cfg::RegUint position) {
    pwrite(fd, buffer, count, position);
}

void Region::saveChunk(cfg::RegUint chunk_index, const cfg::Block * chunk) {
    // TODO: allocate the buffer only once (and with the maximum needed size)
    std::unique_ptr<cfg::RegByte[]> buffer{ std::make_unique<cfg::RegByte[]>(cfg::COMPRESS_BUFFER_SIZE_IN_BYTES) };
    uLongf compressed_size = cfg::COMPRESS_BUFFER_SIZE_IN_BYTES;
    const auto compression_result = compress2(
        buffer.get(), &compressed_size,
        chunk, cfg::CHUNK_VOLUME * sizeof(cfg::Block),
        Z_BEST_COMPRESSION
    );
    if (compression_result != Z_OK)
        throw std::runtime_error("Failed to compress chunk.");

    std::array<cfg::RegUint, 2> ops; // old position, size
    // locking shared is safe assuming no other thread will access loaded version
    // or the in region version of the chunk
    std::shared_lock<std::shared_mutex> lock{ mutex };
    read(ops.data(), sizeof(cfg::RegUint) * ops.size(), (2 + 2 * chunk_index) * sizeof(cfg::RegUint));
    const cfg::RegUint new_size = compressed_size;
    const int64_t size_difference = static_cast<int64_t>(ops[1]) - static_cast<int64_t>(new_size);
    if (size_difference < 0) {
        // update garbage and end
        const cfg::RegUint old_end = end.fetch_add(new_size);
        garbage.fetch_add(ops[1]);
        // append in file
        std::array<cfg::RegUint, 2> nps{ old_end, new_size }; // new position, size
        // write chunk
        write(buffer.get(), new_size, old_end);
        // write position and size
        write(nps.data(), 2 * sizeof(cfg::RegUint), (2 + 2 * chunk_index) * sizeof(cfg::RegUint));

    } else {
        // replace
        // TODO: replace 'uint32: size' with 'uint16: size, uint16: capacity' to be able to use in-place more often
        // write chunk
        write(buffer.get(), new_size, ops[0]);
        // write size
        write(&new_size, sizeof(new_size), (2 + 2 * chunk_index + 1) * sizeof(cfg::RegUint));
        // set garbage size
        garbage.fetch_add(static_cast<cfg::RegUint>(size_difference));
    }

    lock.unlock();


    // "double checked locking" (defragment will lock unique and check garbage again before defragmenting
    // in case someone already defragmented between garbage.load() and defragment())
    if (garbage.load() >= cfg::DEFRAGMENT_GARBAGE_THRESHOLD)
        defragment();
//    Print("saved :)");
}

void Region::defragment() {
    // double checked locking (see caller function)
    std::unique_lock<std::shared_mutex> lock{ mutex };
    if (garbage.load() < cfg::DEFRAGMENT_GARBAGE_THRESHOLD)
        return;
    
    // TODO: implement defragmenting




/*
        assert(region != nullptr);
    
    uint32_t * raw_start = m_defragment_buffer_raw.get();
    uint32_t * raw_end = raw_start + REGION_VOLUME * 2;
    region->read(raw_start, REGION_VOLUME * 2 * sizeof(uint32_t), 2 * sizeof(uint32_t));
    u32Vec3 * vec_start = m_defragment_buffer.get();
    u32Vec3 * vec_end = vec_start; // will be set to correct value in loop
    uint32_t index = 0;
    for (uint32_t * raw = raw_start; raw != raw_end; raw += 2, ++index) {
        if (*raw != 0) {
            // x: index, y: position, z: size
            vec_end->x = index;
            vec_end->y = *raw;
            vec_end->z = *(raw + 1);
            ++vec_end;
        }
    }

    // sort by position to prevent overwriting needed data
    // (since everything will be moved towards position 0)
    std::sort(vec_start, vec_end,
        [](const u32Vec3 & a, const u32Vec3 & b) {
            return a.y < b.y;
        }
    );

    uint8_t * buffer = m_compress_buffer.get();
    uint32_t new_end = (2 * REGION_VOLUME + 2) * (uint32_t)sizeof(uint32_t);
    for (u32Vec3 * i = vec_start; i != vec_end; ++i) {
        // move to new location
        assert(i->y < ((2 * REGION_VOLUME + 2) * (uint32_t)sizeof(uint32_t)));
        assert(i->z <= c_maximum_compressed_size);
        region->read(buffer, i->z, i->y);
        region->write(buffer, i->z, new_end);
        raw_start[2 * i->x] = new_end;
        assert(raw_start[2 * i->x + 1] == i->z);
        new_end += i->z;
    }

    region->garbage = 0;
    region->end = new_end;
    // write back header
    region->write(raw_start, REGION_VOLUME * 2 * sizeof(uint32_t), 2 * sizeof(uint32_t));*/
}

bool Region::loadChunk(cfg::RegUint chunk_index, cfg::Block * chunk) {
    std::shared_lock<std::shared_mutex> lock{ mutex };
    std::array<cfg::RegUint, 2> ps;
    read(ps.data(), sizeof(cfg::RegUint) * ps.size(), (2 + 2 * chunk_index) * sizeof(cfg::RegUint));
    if (ps[0] != 0) { // if exists
        assert(ps[1] > 0); // TODO: assert ps[1] < MAX_ALLOWED_SIZE
        // TODO: replace with per-worker buffer already with allocated max needed size
        std::unique_ptr<cfg::RegByte[]> buffer{ std::make_unique<cfg::RegByte[]>(ps[1]) };
        read(buffer.get(), ps[1], ps[0]);
        lock.unlock(); // don't need file anymore
        uLongf uncompressed_size = cfg::CHUNK_VOLUME * sizeof(cfg::Block);
        const auto decompression_result = uncompress(
            chunk, &uncompressed_size,
            buffer.get(), ps[1]
        );
        // TODO: handle differently?
        if (decompression_result != Z_OK || uncompressed_size != cfg::CHUNK_VOLUME * sizeof(cfg::Block))
            throw std::runtime_error("Broken save file I guess.");
//        Print("loaded :)");
        return true;
    } else {
        return false;
    }
}
