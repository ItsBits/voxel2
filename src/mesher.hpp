#pragma once

#include <array>
#include <vector>
#include "cfg.hpp"
#include <chrono>

namespace mesher {
    // TODO: add NO_AO option to meshers
    enum class MesherType {
        STANDARD,
        HISTO_PYRAMID, // TODO: implement fast block updates
        MULTI_PASS, // TODO: cache mask array because building that mask appears to be 90% of all work
        COPY_THEN_MESH,
        VECTOR_LOOKUP_TABLE,
        INDEX_LOOKUP_TABLE,
        INDEX_LOOKUP_TABLE_UNROLL,
        INDEX_LOOKUP_TABLE_UNROLL_SEMI,
        INDEX_LOOKUP_TABLE_UNROLL_SEMI_NO_LAMBDA,
        ITERATE_1D_ARRAY, // TODO: implement
        INDEX_QUADS, // TODO: implement (each block stores index to quad in mesh, this is adjusted when mesh modified (remove->allbigger--, add->allbigger++))
        SPARSE_MAP // WILL USE A LOT OF EXTRA MEMORY but it is O(1) for all needed operations
    };

    template <MesherType T>
    void mesh(
        std::vector<cfg::Vertex> & mesh,
        const std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> & chunks
    );
    
    // benchmark
    void generic(
        std::vector<cfg::Vertex> & out_mesh,
        const std::array<cfg::Block *, cfg::MESH_CHUNK_VOLUME> & chunks
    );

}
