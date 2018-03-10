#pragma once

#include <glm/glm.hpp>
#include <array>
#include <vector>
#include "Print.hpp"

namespace Math {
    template <typename T>
    struct VecKeyHash {
    std::size_t operator () (const glm::tvec3<T> & k) const {
        return k.x * k.y * k.z; // TODO: better hash function
    }};
    template <typename T>
    struct VecKeyEqual {
    bool operator () (const glm::tvec3<T> & lhs, const glm::tvec3<T> & rhs) const {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
    }};

    // introduced DumbVec3 because struct { int32_t x, y, z; }; does not work with std::aromic (at least in gcc)
    // and sizeof(T) > 64bit might need mutex anyway
    // this limits the world to 2^21*CHUNK_SIZE.x world width (and so on for other dimensions)
    using DumbVec3 = uint64_t;
    template<typename T>
    glm::tvec3<T> toVec3(DumbVec3 d, bool & valid_flag) {
        static_assert(std::is_integral<T>::value && std::is_signed<T>::value);
        static_assert(sizeof(T) * CHAR_BIT >= 21);
        static_assert(-1 == ~0);
        glm::tvec3<T> v;
        v.x = (d >> DumbVec3(0 * 21)) & (1 << 21) - 1;
        v.y = (d >> DumbVec3(1 * 21)) & (1 << 21) - 1;
        v.z = (d >> DumbVec3(2 * 21)) & (1 << 21) - 1;
        v.x <<= sizeof(T) * CHAR_BIT - 21;
        v.y <<= sizeof(T) * CHAR_BIT - 21;
        v.z <<= sizeof(T) * CHAR_BIT - 21;
        v.x >>= sizeof(T) * CHAR_BIT - 21;
        v.y >>= sizeof(T) * CHAR_BIT - 21;
        v.z >>= sizeof(T) * CHAR_BIT - 21;
        valid_flag = ((DumbVec3(1) << DumbVec3(63)) & d) != 0;
        return v;
    }
    template<typename T>
    DumbVec3 toDumb3(glm::tvec3<T> v, bool valid_flag) {
        static_assert(std::is_integral<T>::value && std::is_signed<T>::value);
        static_assert(sizeof(T) * CHAR_BIT >= 21);
        static_assert(-1 == ~0);
        v.x &= (1 << 21) - 1;
        v.y &= (1 << 21) - 1;
        v.z &= (1 << 21) - 1;
        return
            DumbVec3(v.x) << DumbVec3(0 * 21) |
            DumbVec3(v.y) << DumbVec3(1 * 21) |
            DumbVec3(v.z) << DumbVec3(2 * 21) | (valid_flag ? (DumbVec3(1) << DumbVec3(63)) : DumbVec3(0));
    }

    template <typename T>
    struct AABB3 {
        glm::tvec3<T> min, max;
    };

    // word union is reserved :(
    template <typename T>
    constexpr AABB3<T> overlap(const AABB3<T> & a, const AABB3<T> & b) {
        AABB3<T> aabb{
            glm::max(a.min, b.min),
            glm::min(a.max, b.max)
        };

        if (glm::any(glm::lessThanEqual(aabb.max - aabb.min, glm::tvec3<T>{ 0, 0, 0 }))) {
            aabb.min = { 0, 0, 0 };
            aabb.max = { 0, 0, 0 };
        }

        return aabb;
    }

    template <typename T>
    constexpr AABB3<T> toAABB3(const glm::tvec3<T> & center, const glm::tvec3<T> & radius) {
        return {
            center - radius,
            center + radius
        };
    }

    template <typename T>
    constexpr bool inside(const AABB3<T> & a, const glm::tvec3<T> & b) {
        const auto c = glm::greaterThanEqual(a.max, b);
        const auto d = glm::lessThanEqual(a.min, b);
        return glm::all(c) && glm::all(d);
    }

    template <typename T>
    constexpr bool inRange(const glm::tvec3<T> & center, const glm::tvec3<T> & radius, const glm::tvec3<T> & point) {
        return glm::all(glm::lessThanEqual(glm::abs(point - center), radius));
    }

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

    inline uint8_t vertexAOInv(bool side_a, bool side_b, bool corner) {
        if (side_a && side_b) return 1;
        return !side_a + !side_b + !corner + 1;
    }

    inline std::uint8_t vertexAO2(
        const bool side_a, const bool side_b, const bool corner,
        const bool side_a_l, const bool side_b_l, const bool corner_l
    ) {
        uint8_t as = !side_a ? !side_a + !side_a_l : 0;
        uint8_t bs = !side_b ? !side_b + !side_b_l : 0;
        uint8_t cs = !corner ? !corner + !corner_l : 0;

        uint8_t result = 0;
        if (side_a && side_b) result = 0;
        else result = as + bs + cs;

#if 0
        return 6 - result;
#else
        // clamp scale translate
        result = 6 - result;
        if (result < 3) result = 3;
        result -= 3;
        return result * 2;
#endif
    }

    template <typename T>
    glm::tvec4<T> normalizePlane(const glm::tvec4<T> & plane) {
        return plane / glm::length(glm::tvec3<T>{ plane });
    }

    template <typename T>
    std::array<glm::tvec4<T>, 6> matrixToNormalizedFrustumPlanes(glm::tmat4x4<T> MVP) {
        MVP = glm::transpose(MVP);
        return {
            normalizePlane(MVP[3] + MVP[0]),
            normalizePlane(MVP[3] - MVP[0]),
            normalizePlane(MVP[3] - MVP[1]),
            normalizePlane(MVP[3] + MVP[1]),
            normalizePlane(MVP[3] + MVP[2]),
            normalizePlane(MVP[3] - MVP[2])
        };
    }

    template <typename T>
    T planePointDistance(const glm::tvec4<T> & plane, const glm::tvec3<T> & point) {
        return glm::dot(plane, glm::tvec4<T>{ point, T{ 1 } });
    }

    template <typename T>
    bool sphereInFrustum(const std::array<glm::tvec4<T>, 6> & planes, const glm::tvec3<T> & center, const T & radius) {
        for (const auto & plane : planes)
            if (planePointDistance(plane, center) < -radius)
                return false;
        return true;
    }
}
