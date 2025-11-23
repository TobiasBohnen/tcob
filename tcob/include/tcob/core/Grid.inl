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
inline auto grid<T>::operator[](this auto&& self, i32 idx) -> decltype(auto)
{
    assert(idx < std::ssize(self._data));
    return self._data[static_cast<usize>(idx)];
}

template <typename T>
inline auto grid<T>::operator[](this auto&& self, i32 x, i32 y) -> decltype(auto)
{
    return self.operator[](self.get_index(x, y));
}

template <typename T>
inline auto grid<T>::operator[](this auto&& self, point_type pos) -> decltype(auto)
{
    return self.operator[](pos.X, pos.Y);
}

template <typename T>
inline auto grid<T>::row(this auto&& self, i32 row) -> decltype(auto)
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
    auto const start {get_index(pos.X, pos.Y)};
    assert(start + std::ssize(values) <= std::ssize(_data));
    std::ranges::copy(values, _data.begin() + start);
}

template <typename T>
inline void grid<T>::blit(rect_type const& bounds, std::span<T const> values)
{
    assert(bounds.right() <= width());
    assert(bounds.bottom() <= height());
    assert(bounds.left() >= 0);
    assert(bounds.top() >= 0);
    assert(values.size() == bounds.width() * bounds.height());

    i32 const dstX {bounds.left()};
    i32 const dstY {bounds.top()};
    i32 const w {bounds.width()};
    i32 const h {bounds.height()};

    assert(w > 0 && h > 0);

    for (i32 row {0}; row < h; ++row) {
        i32 const srcIndex {row * w};
        i32 const dstIndex {get_index(dstX, dstY + row)};

        std::copy_n(values.begin() + srcIndex, w, _data.begin() + dstIndex);
    }
}

template <typename T>
inline void grid<T>::append_row(std::initializer_list<T> values)
{
    append_row(std::span<T const>(values));
}

template <typename T>
inline void grid<T>::append_row(std::span<T const> values)
{
    assert(static_cast<i32>(values.size()) == width());
    _data.insert(_data.end(), values.begin(), values.end());
    // _data.append_range(values);
    ++_size.Height;
}

template <typename T>
inline void grid<T>::erase_row(i32 row)
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
inline auto grid<T>::height() const -> i32
{
    return _size.Height;
}

template <typename T>
inline auto grid<T>::width() const -> i32
{
    return _size.Width;
}

template <typename T>
inline auto grid<T>::size() const -> size_type
{
    return _size;
}

template <typename T>
inline auto grid<T>::count() const -> i32
{
    return static_cast<i32>(_data.size());
}

template <typename T>
inline void grid<T>::resize(size_type newSize)
{
    _size = newSize;
    _data.resize(_size.Width * _size.Height);
}

template <typename T>
inline auto grid<T>::get_index(i32 x, i32 y) const -> i32
{
    assert(x >= 0 && y >= 0);
    assert(x < _size.Width && y < _size.Height);
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
inline static_grid<T, Width, Height>::static_grid(std::initializer_list<std::initializer_list<T>> rows)
{
    usize idx {0};
    usize y {0};
    for (auto const& row : rows) {
        if (y >= Height) { break; }
        usize x {0};
        for (auto const& val : row) {
            if (x >= Width) { break; }
            _data[idx++] = val;
        }
        ++y;
    }
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::operator[](this auto&& self, i32 idx) -> decltype(auto)
{
    assert(idx < std::ssize(self._data));
    return self._data[static_cast<usize>(idx)];
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::operator[](this auto&& self, i32 x, i32 y) -> decltype(auto)
{
    return self.operator[](self.get_index(x, y));
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::operator[](this auto&& self, point_type pos) -> decltype(auto)
{
    return self.operator[](pos.X, pos.Y);
}

template <typename T, usize Width, usize Height>
inline static_grid<T, Width, Height>::operator grid<T>() const
{
    grid<T> retValue {{static_cast<i32>(Width), static_cast<i32>(Height)}};
    std::copy(_data.begin(), _data.end(), retValue.data().begin());
    return retValue;
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
inline auto static_grid<T, Width, Height>::height() const -> i32
{
    return Height;
}

template <typename T, usize Width, usize Height>
inline auto static_grid<T, Width, Height>::width() const -> i32
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
inline auto static_grid<T, Width, Height>::get_index(i32 x, i32 y) const -> i32
{
    assert(x >= 0 && y >= 0);
    assert(x < Width && y < Height);
    return (y * Width) + x;
}

}
