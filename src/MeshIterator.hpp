#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <algorithm>

class MeshIterator {
public:
    MeshIterator(int radius) {
        m_radius = 0;
        if (radius < 0) return;
        m_radius = radius;

        m_values.reserve((2 * (size_t)radius + 1) * (2 * (size_t)radius + 1) * (2 * (size_t)radius + 1));
        for (int z = -radius; z <= radius; ++z)
            for (int y = -radius; y <= radius; ++y)
                for (int x = -radius; x <= radius; ++x) {
                    m_values.emplace_back(x, y, z);
                }
        std::sort(m_values.begin(), m_values.end(),
        [](const glm::ivec3 & a, const glm::ivec3 & b) {
            return a.x * a.x + a.y * a.y + a.z * a.z < b.x * b.x + b.y * b.y + b.z * b.z;
        });
    }

    size_t size() const { return m_values.size(); }
    int getRadius() const { return m_radius; }

    glm::ivec3 get(size_t i) const {
        assert(i < m_values.size());
        return m_values[i];
    }

private:
    std::vector<glm::ivec3> m_values;
    int m_radius;

};
