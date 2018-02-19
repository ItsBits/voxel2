#pragma once

#include <cstdint>

//==============================================================================
struct Task {
    virtual void execute() = 0;

};

//==============================================================================
struct TaskLoadChunk : public Task {
    void execute() {

    }

    int32_t x, y, z;
    uint8_t * blocks;

};
