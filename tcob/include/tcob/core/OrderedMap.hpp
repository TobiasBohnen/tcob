// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tcob {
////////////////////////////////////////////////////////////

template <typename K, typename V, typename Hash = std::hash<K>, typename Eq = std::equal_to<K>>
class ordered_map {
public:
    using container = std::vector<std::pair<K, V>>;
    using map       = std::unordered_map<K, usize, Hash, Eq>;

    using iterator       = typename container::iterator;
    using const_iterator = typename container::const_iterator;

    auto begin() -> iterator;
    auto begin() const -> const_iterator;

    auto end() -> iterator;
    auto end() const -> const_iterator;

    auto size() const -> usize;
    auto empty() const -> bool;
    void reserve(usize cap);
    auto capacity() const -> usize;

    auto get(auto const& key) -> V*;
    auto get(auto const& key) const -> V const*;

    void set(K key, V value);

    void erase(auto const& key);
    void clear();

    auto contains(auto const& key) const -> bool;

    auto find(auto const& key) -> iterator;
    auto find(auto const& key) const -> const_iterator;

private:
    container _order;
    map       _index;
};

}

#include "OrderedMap.inl"
