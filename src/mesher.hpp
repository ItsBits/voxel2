#pragma once

#include <array>
#include <vector>
#include "cfg.hpp"

namespace mesher {
    enum class MesherType {
        STANDARD
    };

    template <MesherType T>
    void mesh(
        std::vector<cfg::Vertex> & mesh,
        const std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> & chunks
    );
}
