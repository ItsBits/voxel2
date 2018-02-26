#pragma once

#include <shared_mutex>
#include <atomic>
#include <cstdint>
#include <glm/vec3.hpp>
#include "cfg.hpp"

class Region {
public:
    Region() = delete;
    Region(const Region &) = delete;
    Region(const Region &&) = delete;
    Region & operator = (const Region &) = delete;
    Region & operator = (Region &&) = delete;

    // only called by workers and ~ChunkContainer()
    void saveChunk(cfg::RegUint chunk_index, const cfg::Block * chunk);
    bool loadChunk(cfg::RegUint chunk_index, cfg::Block * chunk);

    // only call these from RegionContainer
    Region(const glm::tvec3<cfg::Coord> & region_position);
    ~Region();
    size_t refCountGet() const;
    void refCountIncrement();
    void refCountDecrement();

private:
    // TODO: shared lock
    std::shared_mutex mutex;
    size_t ref_count;
    int fd;
    // TODO: atomic garbage and end
    std::atomic<cfg::RegUint> garbage;
    std::atomic<cfg::RegUint> end;

    void read(void * buffer, cfg::RegUint count, cfg::RegUint position);
    void write(const void * buffer, cfg::RegUint count, cfg::RegUint position);
    void defragment();

};
