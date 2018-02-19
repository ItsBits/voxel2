#pragma once

#include <cassert>
#include <algorithm>
#include <cstdint>
#include "Algebra.hpp"

template <typename K, typename V>
struct MapNode {
    K key;
    V val;
    // list
    MapNode * next;
    MapNode * prev;
    // map
    MapNode * * head;
    MapNode * down;
};

template<typename T, uint32_t N>
class LeastRecentlyUsed {
    // 2-in-1: LRU cache and unused cache
public:
    LeastRecentlyUsed();

    T * get_node(const s32Vec3 & node_position, uint32_t node_index);
    T * get_from_heap();
    T * remove_lru_node();
    void add_node(T * node, uint32_t index);
    // only constructor and destructor calls it:
    void add_to_heap(T * node);

private:
    // LRU map
    T * m_map[N];
    // free heap list
    T * m_free_list;
    // LRU list
    T * m_front;
    T * m_back;

    void remove_node(T * node);

};


//==============================================================================
template<typename T, uint32_t N>
T * LeastRecentlyUsed<T, N>::get_node(const s32Vec3 & node_position, uint32_t node_index) {
    assert(node_index < N);
    T * node = m_map[node_index];
    while (node != nullptr && !all_equal(node_position, node->key))
        node = node->down;

    // move to most recently used
    if (node != nullptr) {
        // TODO: only move in map if not top
        // TODO: only move in list if not front
        remove_node(node);
        add_node(node, node_index);
    }
    return node;
}

//==============================================================================
template<typename T, uint32_t N>
void LeastRecentlyUsed<T, N>::add_node(T * node, uint32_t index) {
    assert(node != nullptr);
    // add to map
    assert(index < N);
    node->head = m_map + index;
    node->down = m_map[index];
    m_map[index] = node;
    // add to list
    node->prev = nullptr;
    node->next = m_front;
    if (m_front != nullptr)
        m_front->prev = node;
    else
        m_back = node;
    m_front = node;
}

//==============================================================================
template<typename T, uint32_t N>
T * LeastRecentlyUsed<T, N>::remove_lru_node() {
    T * lru = m_back;
    if (lru != nullptr)
        remove_node(lru);
    return lru;
}

//==============================================================================
template<typename T, uint32_t N>
void LeastRecentlyUsed<T, N>::remove_node(T * node) {
    assert(node != nullptr);

    // remove from map
    T * * i = node->head;
    while (*i != node) {
        assert(*i != nullptr);
        i = &(*i)->down;
    }
    *i = node->down;

    // remove from list
    if (node->prev != nullptr)
        node->prev->next = node->next;
    else
        m_front = node->next;
    if (node->next != nullptr)
        node->next->prev = node->prev;
    else
        m_back = node->prev;
}

//==============================================================================
template<typename T, uint32_t N>
LeastRecentlyUsed<T, N>::LeastRecentlyUsed() {
    std::fill(std::begin(m_map), std::end(m_map), nullptr);
    m_free_list = nullptr;
    m_front = nullptr;
    m_back = nullptr;
}

//==============================================================================
template<typename T, uint32_t N>
void LeastRecentlyUsed<T, N>::add_to_heap(T * node) {
    assert(node != nullptr);
    node->next = m_free_list;
    m_free_list = node;
}

//==============================================================================
template<typename T, uint32_t N>
T * LeastRecentlyUsed<T, N>::get_from_heap() {
    T * node = m_free_list;
    if (node != nullptr)
        m_free_list = node->next;
    return node;
}
