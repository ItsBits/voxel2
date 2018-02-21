#pragma once

#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include "../gl3w/gl3w.h"
#include "QuadEBO.hpp"
#include <iostream>
#include <cmath>

#include "Mesh.hpp"
#include "LockedQueue.hpp"

#include <glm/gtx/string_cast.hpp>

class VoxelScene {
public:
    void update(const glm::ivec3 & center, LockedQueue<Mesh> & queue);
    void draw(GLint offset_uniform, const std::array<glm::vec4, 6> & planes);

private:
    QuadEBO m_quad_ebo;

    struct ChunkMesh {
        GLuint VAO, VBO;
        GLsizei element_count;
    };
    struct KeyHash {
    std::size_t operator () (const glm::ivec3 & k) const {
        return k.x * k.y * k.z; // TODO: better hash function
    }};
    struct KeyEqual {
    bool operator()(const glm::ivec3 & lhs, const glm::ivec3 & rhs) const {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
    }};
    // TODO: use Coord (cfg.hpp)
    std::unordered_map<glm::ivec3, ChunkMesh, KeyHash, KeyEqual> m_meshes;

};
