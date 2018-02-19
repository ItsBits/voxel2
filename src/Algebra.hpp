#pragma once

#include <cstdint>

template <typename T> struct Vec2 { T x, y; };
template <typename T> struct Vec3 { T x, y, z; };
template <typename T> struct Vec4 { T x, y, z, w; };

template <typename T> T floor_div_i(T x, T y) {
    return (x + (x < 0)) / y - (x < 0);
}

template <typename T> T floor_mod_i(T x, T y) {
    return (x % y + y) % y;
}

using s32Vec3 = Vec3<int32_t>;
using u32Vec3 = Vec3<uint32_t>;

//==============================================================================
template<typename T>
bool all_equal(const T & a, const T & b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

//==============================================================================
template<typename T>
T mul(const T & a, const T & b) {
    return T{ a.x * b.x, a.y * b.y, a.z * b.z };
}

//==============================================================================
template<typename T>
T floor_div(const T & a, const T & b) {
    // compiler might not generate best possible code
    // in case second arument is constexpr power of 2
    T tmp;
    tmp.x = (a.x + (a.x < 0)) / b.x - (a.x < 0);
    tmp.y = (a.y + (a.y < 0)) / b.y - (a.y < 0);
    tmp.z = (a.z + (a.z < 0)) / b.z - (a.z < 0);
    return tmp;
}

//==============================================================================
template<typename T>
T floor_mod(const T & a, const T & b) {
    // compiler might not generate best possible code
    // in case second arument is constexpr power of 2
    T tmp;
    tmp.x = (a.x % b.x + b.x) % b.x;
    tmp.y = (a.y % b.y + b.y) % b.y;
    tmp.z = (a.z % b.z + b.z) % b.z;
    return tmp;
}

//==============================================================================
template<typename T>
T to_index(const Vec3<T> & p, const Vec3<T> & s) {
    return p.z * s.y * s.x + p.y * s.x + p.x;
}

//==============================================================================
template<typename T>
T position_to_index(const Vec3<T> & p, const Vec3<T> & s) {
    return to_index(floor_mod(p, s), s);
}
