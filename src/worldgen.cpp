#include "worldgen.hpp"
#include "Math.hpp"
#include <glm/glm.hpp>

template <>
void worldgen::generate<worldgen::WorldGenType::STANDARD>(
    cfg::Block * chunk, const glm::tvec3<cfg::Coord> & chunk_position
) {
    glm::tvec3<cfg::Coord> i;
    const glm::tvec3<cfg::Coord> fr{ chunk_position * cfg::CHUNK_SIZE };
    const glm::tvec3<cfg::Coord> to{ fr + cfg::CHUNK_SIZE };
    for (i.z = fr.z; i.z < to.z; ++i.z)
        for (i.y = fr.y; i.y < to.y; ++i.y)
            for (i.x = fr.x; i.x < to.x; ++i.x) {
                const auto index = Math::position_to_index(i, cfg::CHUNK_SIZE);
                if (i.y < 0)
                    chunk[index] = std::rand() % 100 == 0;
                else
                    chunk[index] = 0;

/*
                const auto index = Math::position_to_index(i, cfg::CHUNK_SIZE);
                const auto f = Math::floor_mod(i, cfg::CHUNK_SIZE);
                if (glm::any(glm::equal(f, cfg::CHUNK_SIZE - 1)) || glm::any(glm::equal(f, glm::tvec3<cfg::Coord>{ 0, 0, 0 })))
                    chunk[index] = 0;
                else {
                    chunk[index] = std::rand();
                    if (chunk[index] == 0) ++chunk[index];
                }
*/


//                chunk[index] = i.y == -5;
                //chunk[index] = std::rand() % 10 == 0;
/*
                if (glm::all(glm::equal(chunk_position, glm::tvec3<cfg::Coord>{ 0, 0, 0 }))) {
                    chunk[index] = std::rand();
                    if (chunk[index] == 0) chunk[index] = 100;
                } else {
                    chunk[index] = 0;
                }
*/
            }

}