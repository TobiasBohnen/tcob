#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <typename T>
class grid {
    using iterator       = std::vector<T>::iterator;
    using const_iterator = std::vector<T>::const_iterator;

    using size_type      = size_i;
    using dimension_type = size_type::type;
    using point_type     = point<dimension_type>;

public:
    using type = T;

    grid() = default;
    explicit grid(size_type size, T const& defaultValue = T {});

    auto operator[](isize idx) -> T&;
    auto operator[](isize idx) const -> T const&;
    auto operator[](isize x, isize y) -> T&;
    auto operator[](isize x, isize y) const -> T const&;
    auto operator[](point_type pos) -> T&;
    auto operator[](point_type pos) const -> T const&;

    void fill(T const& value);

    auto begin() -> iterator;
    auto begin() const -> const_iterator;

    auto end() -> iterator;
    auto end() const -> const_iterator;

    auto height() const -> dimension_type;
    auto width() const -> dimension_type;
    auto size() const -> size_type;
    auto contains(point_type pos) const -> bool;

    auto count() const -> usize;
    void resize(size_type newSize);

    auto data() -> T*;
    auto data() const -> T const*;

private:
    auto get_index(isize x, isize y) const -> isize;

    size_type      _size;
    std::vector<T> _data;
};

////////////////////////////////////////////////////////////

template <typename T, usize Width, usize Height>
class static_grid {
    using array_type     = std::array<T, Width * Height>;
    using iterator       = array_type::iterator;
    using const_iterator = array_type::const_iterator;

    using size_type      = size_i;
    using dimension_type = size_type::type;
    using point_type     = point<dimension_type>;

public:
    using type = T;

    static_grid();
    explicit static_grid(T const& defaultValue);

    auto operator[](isize idx) -> T&;
    auto operator[](isize idx) const -> T const&;
    auto operator[](isize x, isize y) -> T&;
    auto operator[](isize x, isize y) const -> T const&;
    auto operator[](point_type pos) -> T&;
    auto operator[](point_type pos) const -> T const&;

    void fill(T const& value);

    auto begin() -> iterator;
    auto begin() const -> const_iterator;

    auto end() -> iterator;
    auto end() const -> const_iterator;

    auto height() const -> dimension_type;
    auto width() const -> dimension_type;
    auto size() const -> size_type;
    auto contains(point_type pos) const -> bool;

    auto count() const -> usize;
    auto data() -> T*;
    auto data() const -> T const*;

private:
    auto get_index(isize x, isize y) const -> isize;

    array_type _data;
};

}

#include "Grid.inl"
