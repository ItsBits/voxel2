#pragma once

#include <queue>
#include <mutex>

template <typename T>
struct LockedQueue {
public:
    void push(T && value) {
        std::lock_guard<std::mutex> l{ lock };
        tasks.push(std::move(value));
    }

    bool pop(T && result) {
        std::lock_guard<std::mutex> l{ lock };
        if (tasks.empty()) return false;
        result = std::move(tasks.front());
        tasks.pop();
        return true;
    }
private:
    std::queue<T> tasks;
    std::mutex lock;

};
