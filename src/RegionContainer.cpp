#include "RegionContainer.hpp"
#include "Print.hpp"

Region & RegionContainer::get(const Key & key) {
    std::lock_guard<std::mutex> lock{ m_mutex };
    const auto entry = m_map.find(key);
    decltype(m_used.begin()) iterator;
    if (entry == m_map.end()) {
        while (m_map.size() >= cfg::REGION_CACHE_SIZE && m_lru.size() > 0) {
            m_map.erase(m_lru.back().first);
            m_lru.pop_back();
        }

        if (m_map.size() >= cfg::REGION_CACHE_SIZE)
            Print("Warning region map full.");

        // create new region class
        m_used.emplace_front(
            std::piecewise_construct,
            std::forward_as_tuple(key),
            std::forward_as_tuple(key)
        );
        iterator = m_used.begin();
        m_map.insert({ key, iterator });
    } else {
        // get existing region class
        iterator = entry->second;
        if (iterator->second.refCountGet() == 0)
            // if it is unused, move from lru to used list
            m_used.splice(m_used.begin(), m_lru, iterator);
    }
    iterator->second.refCountIncrement();
    return iterator->second;
}

void RegionContainer::release(const Key & key) {
    std::lock_guard<std::mutex> lock{ m_mutex };
    const auto entry = m_map.find(key);
    assert(entry != m_map.end());
    Region & region = entry->second->second;
    assert(region.refCountGet() > 0);
    region.refCountDecrement();
    if (region.refCountGet() == 0)
        m_lru.splice(m_lru.begin(), m_used, entry->second);
}
