// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Grid.hpp"

#include <cassert>

namespace tcob {

template <typename T>
inline grid<T>::grid(size_type size, T const& defaultValue)
    : _size {size}
    , _data(size.Width * size.Height, defaultValue)
{
}

template <typename T>
inline auto grid<T>::operator[](this auto&& self, isize idx) -> decltype(auto)
{
    assert(idx < std::ssize(self._data));
    return self._data[static_cast<usize>(idx)];
}

template <typename T>
inline auto grid<T>::operator[](this auto&& self, isize x, isize y) -> decltype(auto)
{
    return self.operator[](self.get_index(x, y));
}

template <typename T>
inline auto grid<T>::operator[](this auto&& self, point_type pos) -> decltype(auto)
{
    return self.operator[](pos.X, pos.Y);
}

template <typename T>
inline void grid<T>::fill(T const& value)
{
    std::fill(_data.begin(), _data.end(), value);
}

template <typename T>
inline auto grid<T>::begin(this auto&& self)
{
    return self._data.begin();
}

template <typename T>
inline auto grid<T>::end(this auto&& self)
{
    return self._data.end();
}

template <typename T>
inline auto grid<T>::data(this auto&& self)
{
    return self._data.data();
}

template <typename T>
inline auto grid<T>::height() const -> dimension_type
{
    return _size.Height;
}

template <typename T>
inline auto grid<T>::width() const -> dimension_type
{
    return _size.Width;
}

template <typename T>
inline auto grid<T>::size() const -> size_type
{
    return _size;
}

template <typename T>
inline auto grid<T>::contains(point_type pos) const -> bool
{
    return _size.contains(pos);
}

template <typename T>
inline auto grid<T>::count() const -> usize
{
    return _data.size();
}

template <typename T>
inline void grid<T>::resize(size_type newSize)
{
    _size = newSize;
    _data.resize(_size.Width * _size.Height);
}

template <typename T>
inline auto grid<T>::get_index(isize x, isize y) const -> isize
{
    assert(x >= 0 && y >= 0);
    return (y * _size.Width) + x;
}

////////////////////////////////////////////////////////////

template <typename T, usize Width, usize Height>
inline static_grid<T, Width, Height>::static_grid()
    : _data {}
{
}

template <typename T, usize Width, usize Height>
inline static_grid<T, Width, Height>::static_grid(T const& defaultValue)
    : _data {}
{
    fill(defaultValue);
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::operator[](this auto&& self, isize idx) -> decltype(auto)
{
    assert(idx < std::ssize(self._data));
    return self._data[static_cast<usize>(idx)];
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::operator[](this auto&& self, isize x, isize y) -> decltype(auto)
{
    return self.operator[](self.get_index(x, y));
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::operator[](this auto&& self, point_type pos) -> decltype(auto)
{
    return self.operator[](pos.X, pos.Y);
}

template <typename T, usize Width, usize Height>
inline void static_grid<T, Width, Height>::fill(T const& value)
{
    std::fill(_data.begin(), _data.end(), value);
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::begin(this auto&& self)
{
    return self._data.begin();
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::end(this auto&& self)
{
    return self._data.end();
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::data(this auto&& self)
{
    return self._data.data();
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::height() const -> dimension_type
{
    return Height;
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::width() const -> dimension_type
{
    return Width;
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::size() const -> size_type
{
    return {Width, Height};
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::contains(point_type pos) const -> bool
{
    return size().contains(pos);
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::count() const -> usize
{
    return _data.size();
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::get_index(isize x, isize y) const -> isize
{
    assert(x >= 0 && y >= 0);
    return (y * Width) + x;
}

}
