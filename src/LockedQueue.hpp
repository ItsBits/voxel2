#pragma once

#include <queue>
#include <mutex>
#include <cassert>
#include <condition_variable>

template <typename T, size_t N>
struct LockedQueue {
public:
    void push(T && value) {
        std::unique_lock<std::mutex> l{ lock };
        while (m_queue.size() == N)
            m_condition.wait(l);
        m_queue.push(std::move(value));
    }

    bool pop(T && result) {
        { // unlock before notify
            std::unique_lock<std::mutex> l{ lock };
            if (m_queue.empty()) return false;
            result = std::move(m_queue.front());
            m_queue.pop();
        }
        m_condition.notify_one();
        return true;
    }
private:
    std::queue<T> m_queue;
    std::mutex lock;
    std::condition_variable m_condition;

};
