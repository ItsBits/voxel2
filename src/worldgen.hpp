#pragma once

#include "cfg.hpp"

namespace worldgen {
    enum class WorldGenType {
        STANDARD,
        AIR,
        SINE
    };

    template <WorldGenType T>
    void generate(
        cfg::Block * chunk, const glm::tvec3<cfg::Coord> & chunk_position
    );
}
