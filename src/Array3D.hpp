#pragma once

#include <memory>
#include <cassert>

template <
    uint64_t ARRAY_SIZE_X,
    uint64_t ARRAY_SIZE_Y,
    uint64_t ARRAY_SIZE_Z,
    typename T,
    uint64_t ELEMENT_COUNT
>
class Array3D {
    static constexpr uint64_t ARRAY_VOLUME{ ARRAY_SIZE_X * ARRAY_SIZE_Y * ARRAY_SIZE_Z };

public:
    Array3D() :
        m_array{ std::make_unique<T[]>(ARRAY_VOLUME * ELEMENT_COUNT) }
    {}

    T * get(uint64_t x, uint64_t y, uint64_t z) {
        assert(x < ARRAY_SIZE_X && y < ARRAY_SIZE_Y && z < ARRAY_SIZE_Z);
        const uint64_t i = z * ARRAY_SIZE_Y * ARRAY_SIZE_X + y * ARRAY_SIZE_X + x;
        return m_array.get() + i * ELEMENT_COUNT;
    }

private:
    std::unique_ptr<T[]> m_array;

};
