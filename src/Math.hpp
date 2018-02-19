#pragma once

namespace Math {
    template <typename T>
    constexpr T dot(const glm::tvec3<T> & a, const glm::tvec3<T> & b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    template <typename T>
    constexpr glm::tvec3<T> add(const glm::tvec3<T> & a, const glm::tvec3<T> & b) {
        return {
            a.x + b.x,
            a.y + b.y,
            a.z + b.z
        };
    }

    template <typename T>
    constexpr glm::tvec3<T> add(const glm::tvec3<T> & a, const T & b) {
        return {
            a.x + b,
            a.y + b,
            a.z + b
        };
    }

    template <typename T>
    constexpr glm::tvec3<T> sub(const glm::tvec3<T> & a, const glm::tvec3<T> & b) {
        return {
            a.x - b.x,
            a.y - b.y,
            a.z - b.z
        };
    }

    template <typename T>
    constexpr glm::tvec3<T> floor_div(const glm::tvec3<T> & a, const glm::tvec3<T> & b) {
        return {
            (a.x + (a.x < 0)) / b.x - (a.x < 0),
            (a.y + (a.y < 0)) / b.y - (a.y < 0),
            (a.z + (a.z < 0)) / b.z - (a.z < 0)
        };
    }

    template <typename T>
    constexpr glm::tvec3<T> floor_mod(const glm::tvec3<T> & a, const glm::tvec3<T> & b) {
        return {
            (a.x % b.x + b.x) % b.x,
            (a.y % b.y + b.y) % b.y,
            (a.z % b.z + b.z) % b.z
        };
    }

    template <typename T>
    constexpr T to_index(const glm::tvec3<T> & pos, const glm::tvec3<T> & dim) {
        return (pos.z * dim.y + pos.y) * dim.x + pos.x;
    }

    template <typename T>
    constexpr T position_to_index(const glm::tvec3<T> & pos, const glm::tvec3<T> & dim) {
        return to_index(floor_mod(pos, dim), dim);
    }

    template <typename T>
    constexpr T volume(glm::tvec3<T> v) {
        return v.x * v.y * v.z;
    }

    inline std::uint8_t vertexAO(const bool side_a, const bool side_b, const bool corner) {
    #if 0
        // is this branch free version correct?
        corner = corner || side_a && side_b;
        return (uint8_t)corner + (uint8_t)side_a + (uint8_t)size_b;
    #endif

        if (side_a && side_b) return 3;

        return
            static_cast<std::uint8_t>(side_a) +
            static_cast<std::uint8_t>(side_b) +
            static_cast<std::uint8_t>(corner);
    }
}
