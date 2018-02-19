#pragma once

#include <iostream>
#include <mutex>

namespace {
    static void printRest() { }
    template<typename T>
    static void printRest(const T & val) { std::cout << val; }
    template<typename T, typename ... Args>
    static void printRest(const T & val, Args && ... args) { std::cout << val; printRest(args ...); }
    template <typename T>
    void lock(bool l) {
        static std::mutex m;
        if (l)
            m.lock();
        else
            m.unlock();
    }
}

template<typename T, typename ... Args>
void Print(const T & val, Args && ... args) {
    lock<int>(true);
    std::cout << val;
    printRest(args ...);
    std::cout << std::endl;
    lock<int>(false);
}
