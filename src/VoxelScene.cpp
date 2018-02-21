#include "VoxelScene.hpp"

void VoxelScene::update(const glm::ivec3 & center, LockedQueue<Mesh> & queue) {
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

void VoxelScene::draw(GLint offset_uniform, const std::array<glm::vec4, 6> & planes) {
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