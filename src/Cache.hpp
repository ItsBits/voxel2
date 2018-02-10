#pragma once

//==============================================================================
template <typename K, typename V>
class Cache {
public:
    Cache();
    ~Cache();

    struct Node {
        K key;
        V val;
        // list
        Node * next;
        Node * prev;
        // map
        Node * * head;
        Node * down;
    };

    // insert remove
    V * insert(const K & key);
    void delete_lru();

    // get
    V * get(const K & key);
    V * replace_lru(const K & key);
    V * get_lru(); // use before delete_lru to cleanup

    // container size
    size_t size() const { return m_size; }

private:
    // LRU map
    Node * * m_map;
    // free heap list
    Node * m_free_list;
    // LRU list
    Node * m_front;
    Node * m_back;
    size_t m_map_size;
    size_t m_size;

    void add(const K & key, Node * node);
    void remove(Node * node);

};
//==============================================================================
template <typename K, typename V>
V * Cache<K, V>::insert(const K & key) {
    Node * node = new Node;
    ++m_size;
    add(key, node);
    return &node->val;
}

//==============================================================================
template <typename K, typename V>
Cache<K, V>::Cache() {
    m_size = 0;
    m_map_size = 100;
    m_map = new Node * [m_map_size];
    for (size_t i = 0; i < m_map_size; ++i) m_map[i] = nullptr;
    m_free_list = nullptr;
    m_front = nullptr;
    m_back = nullptr;
}

//==============================================================================
template <typename K, typename V>
Cache<K, V>::~Cache() {
    while (size() > 0) {
        delete_lru();
    }
}

//==============================================================================
template <typename K, typename V>
V * Cache<K, V>::get(const K & key) {
    const size_t index = hash(key) % m_map_size;
    Node * node = m_map[index];
    while (node != nullptr && !equal(key, node->key))
        node = node->down;
    // move to most recently used
    if (node != nullptr) {
        remove(node);
        add(key, node);
        return &node->val;
    } else {
        return nullptr;
    }
}

//==============================================================================
template <typename K, typename V>
V * Cache<K, V>::replace_lru(const K & key) {
    if (m_back != nullptr) {
        Node * node = m_back;
        remove(node);
        add(key, node);
        return &node->val;
    } else {
        return nullptr;
    }
}
//==============================================================================
template <typename K, typename V>
V * Cache<K, V>::get_lru() {
    if (m_back != nullptr) {
        return &m_back->val;
    } else {
        return nullptr;
    }
}

//==============================================================================
template <typename K, typename V>
void Cache<K, V>::delete_lru() {
    if (m_back != nullptr) {
        Node * node = m_back;
        remove(node);
        delete node;
        --m_size;
    }
}

//==============================================================================
template <typename K, typename V>
void Cache<K, V>::add(const K & key, Node * node) {
    const size_t index = hash(key) % m_map_size;
    // add to map
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
template <typename K, typename V>
void Cache<K, V>::remove(Node * node) {
    // remove from map
    Node * * i = node->head;
    while (*i != node)
        i = &(*i)->down;
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
