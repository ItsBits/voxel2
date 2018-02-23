#pragma once

#include <unordered_map>
#include <glm/vec3.hpp>
#include "QuadEBO.hpp"
#include "Ray.hpp"
#include "Mesh.hpp"
#include "LockedQueue.hpp"
#include "VoxelContainer.hpp"
#include "LineCube.hpp"

class VoxelScene {
public:
    void update(const glm::ivec3 & center, LockedQueue<Mesh> & queue, const glm::dvec3 player_position, const glm::dvec3 player_facing, VoxelContainer & vc);
    void draw(GLint offset_uniform, const std::array<glm::vec4, 6> & planes, glm::tvec3<cfg::Coord> offset_offset);
    void draw_cube(const glm::mat4 & VP, const glm::dvec3 & camera_offset);

private:
    LineCube m_line_cube;

    QuadEBO m_quad_ebo;
    glm::tvec3<cfg::Coord> m_selected_block;
    bool m_block_hit;

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
    // TODO: based on SparseMap try to also use std::vector for potentially faster iteration
    std::unordered_map<glm::ivec3, ChunkMesh, KeyHash, KeyEqual> m_meshes;

};
