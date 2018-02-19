#include "Region.hpp"

#include <array>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

Region::Region() {
    fd = -1;
}

Region::~Region() {
    closeRegion();
}

void Region::read(void * buffer, size_t count, size_t position) {
    pread(fd, buffer, count, position);
}

void Region::write(const void * buffer, size_t count, size_t position) {
    pwrite(fd, buffer, count, position);
}

void Region::openNewRegion(const glm::tvec3<cfg::Coord> & region_position) {
    closeRegion();

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
        end = (2 * cfg::REGION_VOLUME + 2) * sizeof(cfg::RegUint);
        garbage = 0;
        ftruncate(fd, end);
    } else {
        std::array<cfg::RegUint, 2> eg;
        read(eg.data(), 2 * sizeof(cfg::RegUint), 0);
        end = eg[0];
        garbage = eg[1];
    }
}

void Region::closeRegion() {
    if (fd < 0) return;
    const std::array<cfg::RegUint, 2> eg{ end, garbage };
    write(eg.data(), 2 * sizeof(cfg::RegUint), 0);
    close(fd);
    fd = -1;
}
