// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "OrderedMap.hpp"

#include <utility>

namespace tcob {
////////////////////////////////////////////////////////////

template <typename K, typename V, typename Hash, typename Eq>
inline auto ordered_map<K, V, Hash, Eq>::begin() -> iterator { return _order.begin(); }
template <typename K, typename V, typename Hash, typename Eq>
inline auto ordered_map<K, V, Hash, Eq>::begin() const -> const_iterator { return _order.begin(); }

template <typename K, typename V, typename Hash, typename Eq>
inline auto ordered_map<K, V, Hash, Eq>::end() -> iterator { return _order.end(); }
template <typename K, typename V, typename Hash, typename Eq>
inline auto ordered_map<K, V, Hash, Eq>::end() const -> const_iterator { return _order.end(); }

template <typename K, typename V, typename Hash, typename Eq>
inline auto ordered_map<K, V, Hash, Eq>::size() const -> usize { return _order.size(); }

template <typename K, typename V, typename Hash, typename Eq>
inline auto ordered_map<K, V, Hash, Eq>::empty() const -> bool { return _order.empty(); }

template <typename K, typename V, typename Hash, typename Eq>
inline void ordered_map<K, V, Hash, Eq>::reserve(usize cap)
{
    _order.reserve(cap);
    _index.reserve(cap);
}

template <typename K, typename V, typename Hash, typename Eq>
inline auto ordered_map<K, V, Hash, Eq>::capacity() const -> usize { return _order.capacity(); }

template <typename K, typename V, typename Hash, typename Eq>
inline auto ordered_map<K, V, Hash, Eq>::get(auto const& key) const -> V const*
{
    auto const it {_index.find(key)};
    if (it == _index.end()) { return nullptr; }
    return &_order[it->second].second;
}

template <typename K, typename V, typename Hash, typename Eq>
inline auto ordered_map<K, V, Hash, Eq>::get(auto const& key) -> V*
{
    auto const it {_index.find(key)};
    if (it == _index.end()) { return nullptr; }
    return &_order[it->second].second;
}

template <typename K, typename V, typename Hash, typename Eq>
inline void ordered_map<K, V, Hash, Eq>::set(K key, V value)
{
    auto const it {_index.find(key)};

    if (it != _index.end()) {
        _order[it->second].second = std::move(value);
        return;
    }

    usize const pos {_order.size()};
    _order.emplace_back(std::move(key), std::move(value));
    _index.emplace(_order.back().first, pos);
}

template <typename K, typename V, typename Hash, typename Eq>
inline void ordered_map<K, V, Hash, Eq>::erase(auto const& key)
{
    auto const it {_index.find(key)};
    if (it == _index.end()) { return; }

    usize const pos {it->second};

    _index.erase(it);
    _order.erase(_order.begin() + pos);

    for (usize i {pos}; i < _order.size(); ++i) {
        _index[_order[i].first] = i;
    }
}

template <typename K, typename V, typename Hash, typename Eq>
inline void ordered_map<K, V, Hash, Eq>::clear()
{
    _order.clear();
    _index.clear();
}

template <typename K, typename V, typename Hash, typename Eq>
inline auto ordered_map<K, V, Hash, Eq>::contains(auto const& key) const -> bool { return _index.find(key) != _index.end(); }

template <typename K, typename V, typename Hash, typename Eq>
inline auto ordered_map<K, V, Hash, Eq>::find(auto const& key) const -> const_iterator
{
    auto const it {_index.find(key)};
    if (it == _index.end()) { return _order.end(); }
    return _order.begin() + static_cast<isize>(it->second);
}

template <typename K, typename V, typename Hash, typename Eq>
inline auto ordered_map<K, V, Hash, Eq>::find(auto const& key) -> iterator
{
    auto const it {_index.find(key)};
    if (it == _index.end()) { return _order.end(); }
    return _order.begin() + static_cast<isize>(it->second);
}

}
