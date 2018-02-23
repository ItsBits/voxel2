#pragma once

#include "cfg.hpp"
#include <type_traits>

template <typename F, typename I>
class Ray {
    static_assert(std::is_floating_point<F>::value);
    static_assert(std::is_integral<I>::value);

public:
    struct State {
        glm::tvec3<I> block_position;
        F distance;
    };

    Ray(const glm::tvec3<F> & start, const glm::tvec3<F> & dir) {
        const auto start_floor = glm::floor(start);
        m_state.block_position = start_floor;
        m_state.distance = F{ 0 };
        m_step = glm::sign(dir);
        m_delta = glm::abs(glm::length(dir) /* * VOXEL_DIMENSIONS */ / dir);
        m_t_max.x = m_step.x == 1 ? (m_delta.x * (F{ 1 } - start.x + start_floor.x)) : (m_delta.x * (start.x - start_floor.x));
        m_t_max.y = m_step.y == 1 ? (m_delta.y * (F{ 1 } - start.y + start_floor.y)) : (m_delta.y * (start.y - start_floor.y));
        m_t_max.z = m_step.z == 1 ? (m_delta.z * (F{ 1 } - start.z + start_floor.z)) : (m_delta.z * (start.z - start_floor.z));
    }

    State next() {
        // current state is already calculated
        const State result = m_state;

        // calculate future step
        size_t i;
        if (m_t_max.x < m_t_max.y) {
            if (m_t_max.x < m_t_max.z)
                i = 0;
            else
                i = 2;
        } else {
            if (m_t_max.y < m_t_max.z)
                i = 1;
            else
                i = 2;
        }

        m_state.block_position[i] += m_step[i];
        m_state.distance = m_t_max[i];
        m_t_max[i] += m_delta[i];

        return result;
    }

private:
    State m_state;
    glm::tvec3<I> m_step;
    glm::tvec3<F> m_delta;
    glm::tvec3<F> m_t_max;

};
