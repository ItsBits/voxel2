#pragma once

#include <mutex>
#include <unordered_map>
#include <list>
#include "Region.hpp"
#include "cfg.hpp"

class RegionContainer {
private:
    using Key = glm::tvec3<cfg::Coord>;
    using Val = Region;
    using ListEntry = std::pair<Key, Val>;
    std::list<ListEntry> m_used;
    std::list<ListEntry> m_lru;
    using Iterator = typename decltype(m_lru)::iterator;
    std::unordered_map<Key, Iterator, Math::VecKeyHash<cfg::Coord>, Math::VecKeyEqual<cfg::Coord>> m_map;
    std::mutex m_mutex;

public:
    Region & get(const Key & key);
    void release(const Key & key);

};
