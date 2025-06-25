// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Grid.hpp"

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <span>

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
inline auto grid<T>::row(this auto&& self, isize row) -> decltype(auto)
{
    assert(row >= 0 && row < self._size.Height);
    auto const w {self.width()};
    auto const start {row * w};
    return std::span {self._data.data() + start, static_cast<usize>(w)};
}

template <typename T>
inline void grid<T>::fill(T const& value)
{
    std::fill(_data.begin(), _data.end(), value);
}

template <typename T>
inline void grid<T>::assign(point_type pos, std::initializer_list<T> values)
{
    assign(pos, std::span<T const>(values));
}

template <typename T>
inline void grid<T>::assign(point_type pos, std::span<T const> values)
{
    assert(_size.contains(pos));
    auto const start {get_index(pos.X, pos.Y)};
    assert(start + std::ssize(values) <= std::ssize(_data));
    std::ranges::copy(values, _data.begin() + start);
}

template <typename T>
inline void grid<T>::append(std::initializer_list<T> values)
{
    append(std::span<T const>(values));
}

template <typename T>
inline void grid<T>::append(std::span<T const> values)
{
    assert(static_cast<dimension_type>(values.size()) == width());
    _data.append_range(values);
    ++_size.Height;
}

template <typename T>
inline void grid<T>::erase(isize row)
{
    assert(row >= 0 && row < _size.Height);

    auto const w {width()};
    auto const start {row * w};
    auto const end {start + w};
    _data.erase(_data.begin() + start, _data.begin() + end);
    --_size.Height;
}

template <typename T>
inline void grid<T>::clear()
{
    _data.clear();
    _size = size_type::Zero;
}

template <typename T>
inline auto grid<T>::begin(this auto&& self) -> decltype(auto)
{
    return self._data.begin();
}

template <typename T>
inline auto grid<T>::end(this auto&& self) -> decltype(auto)
{
    return self._data.end();
}

template <typename T>
inline auto grid<T>::data(this auto&& self) -> decltype(auto)
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
inline auto grid<T>::count() const -> isize
{
    return std::ssize(_data);
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
inline auto static_grid<T, Width, Height>::begin(this auto&& self) -> decltype(auto)
{
    return self._data.begin();
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::end(this auto&& self) -> decltype(auto)
{
    return self._data.end();
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::data(this auto&& self) -> decltype(auto)
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
