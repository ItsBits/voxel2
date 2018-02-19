#pragma once

#include <cstdint>
#include <glm/vec3.hpp>
#include "cfg.hpp"

class Region {
public:
    Region();
    ~Region();
    void read(void * buffer, size_t count, size_t position);
    void write(const void * buffer, size_t count, size_t position);
    void openNewRegion(const glm::tvec3<cfg::Coord> & region_position);

private:
    void closeRegion();
    int fd;
    cfg::RegUint garbage;
    cfg::RegUint end;

};
