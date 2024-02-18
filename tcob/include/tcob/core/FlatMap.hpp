// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <vector>

namespace tcob {
////////////////////////////////////////////////////////////

template <typename Key, typename Value>
class flat_map {
public:
    using key_type       = Key;
    using value_type     = Value;
    using pair_type      = std::pair<key_type, value_type>;
    using iterator       = typename std::vector<pair_type>::iterator;
    using const_iterator = typename std::vector<pair_type>::const_iterator;

    flat_map() = default;

    flat_map(std::initializer_list<pair_type> list);

    auto operator[](key_type const& key) -> value_type&;

    auto at(key_type const& key) const -> value_type const&;

    void insert(key_type const& key, value_type const& value);
    void insert(key_type const& key, value_type&& value);
    template <typename InputIt>
    void insert(InputIt first, InputIt last);

    void clear();

    auto erase(iterator const& it) -> iterator;
    auto erase(const_iterator const& it) -> iterator;
    auto erase(key_type const& key) -> iterator;
    void erase_if(std::function<bool(pair_type const&)> predicate);

    auto find(key_type const& key) -> iterator;
    auto find(key_type const& key) const -> const_iterator;

    auto contains(key_type const& key) const -> bool;

    auto empty() const -> bool;

    auto begin() -> iterator;
    auto begin() const -> const_iterator;

    auto end() -> iterator;
    auto end() const -> const_iterator;

    auto front() -> pair_type&;

    auto back() -> pair_type&;

    auto size() const -> usize;

    auto capacity() const -> usize;

    void reserve(usize cap);

private:
    std::vector<pair_type> _data;
};

}

#include "FlatMap.inl"
