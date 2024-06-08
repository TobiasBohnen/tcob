// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "FlatMap.hpp"

#include <algorithm>

namespace tcob {

template <typename Key, typename Value>
inline flat_map<Key, Value>::flat_map(std::initializer_list<pair_type> list)
    : _data {list}
{
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::operator[](key_type const& key) -> value_type&
{
    auto it {find(key)};
    if (it != _data.end()) {
        return it->second;
    }

    insert(key, value_type {});
    return _data.back().second;
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::at(key_type const& key) const -> value_type const&
{
    auto it {find(key)};
    if (it != _data.end()) {
        return it->second;
    }

    throw std::out_of_range("Key not found in flat_map");
}

template <typename Key, typename Value>
inline void flat_map<Key, Value>::insert(key_type const& key, value_type const& value)
{
    _data.emplace_back(key, value);
}

template <typename Key, typename Value>
inline void flat_map<Key, Value>::insert(key_type const& key, value_type&& value)
{
    _data.emplace_back(key, std::move(value));
}

template <typename Key, typename Value>
template <typename InputIt>
inline void flat_map<Key, Value>::insert(InputIt first, InputIt last)
{
    _data.insert(_data.end(), first, last);
}

template <typename Key, typename Value>
inline void flat_map<Key, Value>::clear()
{
    _data.clear();
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::erase(iterator const& it) -> iterator
{
    return _data.erase(it);
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::erase(const_iterator const& it) -> iterator
{
    return _data.erase(it);
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::erase(key_type const& key) -> iterator
{
    return erase(find(key));
}

template <typename Key, typename Value>
inline void flat_map<Key, Value>::erase_if(std::function<bool(pair_type const&)> predicate)
{
    std::erase_if(_data, predicate);
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::find(key_type const& key) -> iterator
{
    return std::find_if(_data.begin(), _data.end(),
                        [key](pair_type const& pair) { return pair.first == key; });
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::find(key_type const& key) const -> const_iterator
{
    return std::find_if(_data.begin(), _data.end(),
                        [key](pair_type const& pair) { return pair.first == key; });
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::contains(key_type const& key) const -> bool
{
    return find(key) != _data.end();
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::empty() const -> bool
{
    return _data.empty();
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::begin() -> iterator
{
    return _data.begin();
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::begin() const -> const_iterator
{
    return _data.begin();
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::end() -> iterator
{
    return _data.end();
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::end() const -> const_iterator
{
    return _data.end();
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::front() -> pair_type&
{
    return _data.front();
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::back() -> pair_type&
{
    return _data.back();
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::size() const -> usize
{
    return _data.size();
}

template <typename Key, typename Value>
inline auto flat_map<Key, Value>::capacity() const -> usize
{
    return _data.capacity();
}

template <typename Key, typename Value>
inline void flat_map<Key, Value>::reserve(usize cap)
{
    _data.reserve(cap);
}

}
