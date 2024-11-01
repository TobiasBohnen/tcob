// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Grid.hpp"

namespace tcob {

template <typename T>
inline grid<T>::grid(size_type size, T const& defaultValue)
    : _size {size}
    , _data(size.Width * size.Height, defaultValue)
{
}

template <typename T>
inline auto grid<T>::operator[](point_type pos) -> T&
{
    return _data[get_index(pos)];
}

template <typename T>
inline auto grid<T>::operator[](point_type pos) const -> T const&
{
    return _data[get_index(pos)];
}

template <typename T>
inline auto grid<T>::operator[](usize idx) -> T&
{
    return _data[idx];
}

template <typename T>
inline auto grid<T>::operator[](usize idx) const -> T const&
{
    return _data[idx];
}

template <typename T>
inline void grid<T>::fill(T const& value)
{
    std::fill(_data.begin(), _data.end(), value);
}

template <typename T>
inline auto grid<T>::begin() -> iterator
{
    return _data.begin();
}

template <typename T>
inline auto grid<T>::begin() const -> const_iterator
{
    return _data.begin();
}

template <typename T>
inline auto grid<T>::end() -> iterator
{
    return _data.end();
}

template <typename T>
inline auto grid<T>::end() const -> const_iterator
{
    return _data.end();
}

template <typename T>
inline auto grid<T>::get_row_count() const -> dimension_type
{
    return _size.Height;
}

template <typename T>
inline auto grid<T>::get_column_count() const -> dimension_type
{
    return _size.Width;
}

template <typename T>
inline auto grid<T>::get_extent() const -> size_type
{
    return _size;
}

template <typename T>
inline auto grid<T>::size() const -> usize
{
    return _data.size();
}

template <typename T>
inline auto grid<T>::get_index(point_type pos) const -> dimension_type
{
    return pos.Y * _size.Width + pos.X;
}

}
