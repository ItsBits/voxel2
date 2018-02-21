#pragma once

#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include "../gl3w/gl3w.h"
#include "VoxelMap.hpp"
#include "MeshIterator.hpp"
#include "QuadEBO.hpp"
#include <iostream>
#include <cmath>

#include "Mesh.hpp"
#include "LockedQueue.hpp"

#include <glm/gtx/string_cast.hpp>

class VoxelScene {
public:
    VoxelScene() : m_mesh_iterator{ 10 } {

    }

    void update(const glm::ivec3 & center, LockedQueue<Mesh> & queue) {
        /*
        for(auto m = m_meshes.begin(); m != m_meshes.end();) {
            const glm::ivec3 distance = glm::abs(center - m->first);
            const int r = m_mesh_iterator.getRadius();
            if (distance.x > r || distance.y > r || distance.z > r) {
                glDeleteBuffers(1, &m->second.VBO);
                glDeleteVertexArrays(1, &m->second.VAO);
                m = m_meshes.erase(m);
            } else {
                ++m;
            }
        }*/

        Mesh m;
        while (queue.pop(std::move(m))) {
            ChunkMesh chunk_mesh;
            const size_t vetrex_count = m.mesh.size();
            if (vetrex_count == 0) continue;
            // size should always be divisible by 2
            chunk_mesh.element_count = vetrex_count + (vetrex_count / 2);

            glGenVertexArrays(1, &chunk_mesh.VAO);
            glGenBuffers(1, &chunk_mesh.VBO);
            glBindVertexArray(chunk_mesh.VAO);
            glBindBuffer(GL_ARRAY_BUFFER, chunk_mesh.VBO);
            m_quad_ebo.bind();
            m_quad_ebo.resize(chunk_mesh.element_count);
            glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, sizeof(cfg::Vertex), (GLvoid *)(0));
            glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, sizeof(cfg::Vertex), (GLvoid *)(3));
            glVertexAttribIPointer(2, 4, GL_UNSIGNED_BYTE, sizeof(cfg::Vertex), (GLvoid *)(4));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glBindVertexArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, chunk_mesh.VBO);
            glBufferData(GL_ARRAY_BUFFER, m.mesh.size() * sizeof(m.mesh[0]), m.mesh.data(), GL_STATIC_DRAW);

            m_meshes.insert({ m.position, chunk_mesh });
        }
    }

    void draw(GLint offset_uniform, const std::array<glm::vec4, 6> & planes) {
        static const float MESH_RADIUS{ glm::length(glm::vec3{ cfg::MESH_SIZE } / 2.0f) };
        auto s = m_meshes.size();
        for (const auto & m : m_meshes) {
            if (m.second.element_count <= 0)
                continue;
            const auto offset = m.first * cfg::MESH_SIZE + cfg::MESH_OFFSET;
            const glm::vec3 center = glm::vec3{ offset } + glm::vec3{ cfg::MESH_SIZE } / 2.0f;

            if (!Math::sphereInFrustum(planes, center, MESH_RADIUS))
                continue;
            glUniform3f(offset_uniform, offset.x, offset.y, offset.z);
            glBindVertexArray(m.second.VAO);
            glDrawElements(GL_TRIANGLES, m.second.element_count, m_quad_ebo.type(), 0);
            glBindVertexArray(0);
            
        }
    }

    glm::ivec3 get_chunk_sizes() const { return CHUNK_SIZES; }

private:
    glm::ivec3 CHUNK_SIZES{ 16, 16, 16 };
    QuadEBO m_quad_ebo;

    void create_new_chunk(uint8_t * blocks, const glm::ivec3 & chunk_position) {
        const glm::ivec3 from_block = chunk_position * CHUNK_SIZES; 
        const glm::ivec3 to_block = from_block + CHUNK_SIZES; 
        glm::ivec3 i; 
        size_t j = 0; 
        for (i.z = from_block.z; i.z < to_block.z; ++i.z) 
            for (i.y = from_block.y; i.y < to_block.y; ++i.y) 
                for (i.x = from_block.x; i.x < to_block.x; ++i.x) { 
                    if (std::sin(i.x * 0.1f) * std::sin(i.z * 0.1f) * 10.0f > static_cast<float>(i.y)) 
                        blocks[j] = 1; 
                    else 
                        blocks[j] = 0; 
                    ++j; 
                }
    }

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
    MeshIterator m_mesh_iterator;

    std::vector<uint8_t> generateChunkMesh(VoxelMap & vs, const glm::ivec3 & chunk_position);

    ChunkMesh generateAndUploadChunkMesh(VoxelMap & vs, const glm::ivec3 & chunk_position) {
        std::vector<uint8_t> mesh = generateChunkMesh(vs, chunk_position);

        ChunkMesh chunk_mesh;
        const size_t vetrex_count = mesh.size() / 8;
        // size should always be divisible by 2
        chunk_mesh.element_count = vetrex_count + (vetrex_count / 2);

        glGenVertexArrays(1, &chunk_mesh.VAO);
        glGenBuffers(1, &chunk_mesh.VBO);
        glBindVertexArray(chunk_mesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, chunk_mesh.VBO);
        m_quad_ebo.bind();
        m_quad_ebo.resize(chunk_mesh.element_count);
        glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, sizeof(uint8_t) * 8, (GLvoid *)(0));
        glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, sizeof(uint8_t) * 8, (GLvoid *)(3));
        glVertexAttribIPointer(2, 4, GL_UNSIGNED_BYTE, sizeof(uint8_t) * 8, (GLvoid *)(4));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, chunk_mesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(mesh[0]), mesh.data(), GL_STATIC_DRAW);

        return chunk_mesh;
    }
};
