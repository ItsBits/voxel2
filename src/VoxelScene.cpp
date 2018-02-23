#include "VoxelScene.hpp"

#include "Ray.hpp"

void VoxelScene::update(const glm::ivec3 & center, LockedQueue<Mesh> & queue, Ray<double> ray, VoxelContainer & vc) {
    // cast ray
    Ray<double>::State ray_state = ray.next();
    glm::tvec3<cfg::Coord> chunk_position = Math::floor_div(ray_state.block_position, cfg::CHUNK_SIZE);
    const cfg::Block * chunk = vc.getChunk(chunk_position);
    m_block_hit = false;

    while (ray_state.distance <= cfg::MAX_RAY_LENGTH && chunk != nullptr) {
        const auto block_index = Math::position_to_index(ray_state.block_position, cfg::CHUNK_SIZE);
        const cfg::Block & block = chunk[block_index];
        if (block != cfg::Block{ 0 }) {
            m_block_hit = true;
            break;
        }
        ray_state = ray.next();
        const auto new_chunk_position = Math::floor_div(ray_state.block_position, cfg::CHUNK_SIZE);
        if (!glm::all(glm::equal(new_chunk_position, chunk_position))) {
            chunk_position = new_chunk_position;
            chunk = vc.getChunk(chunk_position);
        }
    }

    m_selected_block = ray_state.block_position;

    // TODO: delete out of range meshes
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

    // upload meshes
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
        // TODO: glVertexAttribPointer + GL_UNSIGNED_BYTE
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

void VoxelScene::draw(GLint offset_uniform, const std::array<glm::vec4, 6> & planes, glm::tvec3<cfg::Coord> offset_offset) {
    static const float MESH_RADIUS{ glm::length(glm::vec3{ cfg::MESH_SIZE } / 2.0f) };
    auto s = m_meshes.size();
    offset_offset += cfg::MESH_OFFSET;
    for (const auto & m : m_meshes) {
        if (m.second.element_count <= 0)
            continue;
        const auto offset = m.first * cfg::MESH_SIZE + offset_offset;
        const glm::vec3 center = glm::vec3{ offset } + glm::vec3{ cfg::MESH_SIZE } / 2.0f;

        if (!Math::sphereInFrustum(planes, center, MESH_RADIUS))
            continue;
        glUniform3f(offset_uniform, offset.x, offset.y, offset.z);
        glBindVertexArray(m.second.VAO);
        glDrawElements(GL_TRIANGLES, m.second.element_count, m_quad_ebo.type(), 0);
        glBindVertexArray(0);
        
    }
}

void VoxelScene::draw_cube(const glm::mat4 & VP, const glm::dvec3 & camera_offset) {
//    if (m_block_hit)
        m_line_cube.draw(VP, m_selected_block + glm::ivec3{ camera_offset }, { 1, 1, 1 });
    //m_line_cube.draw(VP, { 0, 0, 0 }, { 1, 1, 1 });
}
